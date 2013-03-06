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

vec3 process(const float x, const float y, const float width, const float height)
{
	static const vec3 camUp = get_camera_up();
	static const vec3 camTarget = get_camera_target();
	static const vec3 camPos = get_camera_position();
	static const vec3 camDir = normalize(camTarget - camPos);
	static const vec3 u = cross(camUp, camDir);
	static const vec3 v = cross(camDir, u);

	const vec2 screenPos = -1.0f + 2.0f * vec2(x, height - y) / vec2(width, height);
	const vec3 rayDir = normalize(u * screenPos.x * width / height + v * screenPos.y + camDir);
	
	return raymarch(camPos, rayDir, 0);
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
