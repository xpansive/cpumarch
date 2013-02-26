#include "common.h"
#include "scene.h"
#include <chrono>
#include <iostream>
#include <vector>
#include <unistd.h>
#include <thread>
#include <glm/gtc/noise.hpp>
#include <glm/gtc/random.hpp>

using std::vector;
using std::thread;

float softshadow(const vec3& ro, const vec3& rd, const float mint, const float maxt, const float k)
{
	float res = 1;
	float t = mint;
	for(int iter = 0; iter < 128; iter++) {
		float h = map(ro + rd * t).field;
		if(h < 0.0001)
			return 0;
		res = min(res, k*h/t);
		t += h;
		if (t > maxt)
			break;
	}
	return res;
}

float shadow(vec3 p, const vec3& l)
{
	vec3 d = normalize(l - p);
	float maxd = glm::distance(p, l);
	float dist = 0.1;
	for (int i = 0; i < 256; i++) {
		p += d * dist;
		dist = map(p).field;
		if (dist < 0.001 || dist > maxd) break;
	}
	if (dist < 0.001) {
		return 0.3;
	}
	return 1;
}

float ambient_occlusion(const vec3& p, const vec3& n, const float d) {
	float res = 1;
	for (int i = 0; i < 5; i++) {
		res -= (i * d - map(p + n * (i * d)).field) / exp2(i);
	}
	return res;
}

float subsurface_scattering(const vec3& p, const vec3& n, const float d)
{
	float res = 0;
	for (int i = 0; i < 5; i++) {
		res += (i * d - map(p + n * (i * d)).field) / exp2(i);
	}
	return res;
}

vec3 process(const float x, const float y, const float width, const float height)
{
	static const float min_dist = get_dist_epsilon();
	static const float max_dist = get_max_dist();
	static const int max_iter = get_max_iter();
	static const vec3 camUp = get_camera_up();
	static const vec3 camTarget = get_camera_target();
	static const vec3 camPos = get_camera_position();
	static const vector<vec3> lightPositions = get_lights();
	static const vec3 camDir = normalize(camTarget - camPos);
	static const vec3 u = cross(camUp, camDir);
	static const vec3 v = cross(camDir, u);

	const vec2 screenPos = -1.0f + 2.0f * vec2(x, height - y) / vec2(width, height);
	const vec3 rayDir = normalize(u * screenPos.x * width / height + v * screenPos.y + camDir);

	distance dist { 0, 0 };
	float total_dist = 0;
	vec3 p = camPos;
	for (int iter = 0; iter < max_iter; iter++) {
		dist = map(p);
		if (abs(dist.field) < min_dist || total_dist > max_dist)
			break;
		p += rayDir * dist.field;
		total_dist += dist.field;
	}
	
	if (dist.field < min_dist) {
		float lighting = 0;
		const float normdist = 0.00001;
		const vec3 n = normalize(vec3(
			dist.field - map(p + vec3(normdist, 0, 0)).field,
			dist.field - map(p + vec3(0, normdist, 0)).field,
			dist.field - map(p + vec3(0, 0, normdist)).field));
		for (const vec3 light : lightPositions) {
			const vec3 lightDir = normalize(p - light);
			float s = softshadow(p, normalize(light - p), 0.3, max_dist, 256) * 0.8 + 0.2;
			float b = max(dot(n, lightDir), 0.0f);
			float ao = ambient_occlusion(p, n, 0.1);
			float sss = subsurface_scattering(p, rayDir, 0.9);

			lighting += ao * sss * s * b;
		}
		lighting = clamp(lighting, 0.05f, 1.0f);
		const vec3 color = get_color(dist.object_id, p) * lighting;
		return color;
	} else {
		return vec3(0);
	}
}

void output_image(char* image, int width, int height)
{
	std::cout << "P6\n" << width << " " << height << "\n255\n";
	std::cout.write(image, width * 3 * height);
}

int image_index(const int x, const int y, const int width)
{
	return (x + y * width) * 3;
}

void render(char* image, const int offset_y, const int max_y, const int width, const int height, const int id)
{
	image += image_index(0, offset_y, width);
	for (int y = offset_y; y < max_y; y++) {
		for (int x = 0; x < width; x++) {
#ifdef SUPERSAMPLE
#ifdef RANDOM_SUPERSAMPLING
			const float offset = 0.5f;
			vec3 color;
			for (int i = 0; i < SUPERSAMPLE_NUM_SAMPLES; i++) {
				const vec2 shift = glm::gaussRand(vec2(-offset), vec2(offset));
				color += process(x + shift.x, y + shift.y, width, height);
			}
			color /= SUPERSAMPLE_NUM_SAMPLES;
#endif
#else
			vec3 color = process(x, y, width, height);
#endif

			*image++ = char(color.x * 255);
			*image++ = char(color.y * 255);
			*image++ = char(color.z * 255);
		}
	}
	std::cerr << id << std::endl;
}

int main()
{
	int width = 1920, height = 1080;
	char* image = new char[width * 3 * height];

	int num_processors = sysconf(_SC_NPROCESSORS_ONLN);
	std::cerr << "Using " << num_processors << " threads." << std::endl;

	auto start_time = std::chrono::system_clock::now();

	vector<thread> threads;
	for (int i = 0; i < num_processors; i++) {
		threads.emplace_back(thread(render, image, height / num_processors * i, height / num_processors * (i + 1), width, height, i));
	}

	for (int i = 0; i < num_processors; i++) {
		threads[i].join();
	}

	auto end_time = std::chrono::system_clock::now();
	int msecs = std::chrono::duration_cast<std::chrono::milliseconds> (end_time - start_time).count();

	output_image(image, width, height);
	delete [] image;

	std::cerr << "Time: " << msecs << "ms." << std::endl;
}
