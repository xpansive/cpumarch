#include <glm/glm.hpp>
#include <chrono>
#include <iostream>
#include <vector>
#include <unistd.h>
#include <thread>
#include <glm/gtc/noise.hpp>
#include <glm/gtc/random.hpp>

using glm::vec3;
using glm::vec4;
using glm::pow;
using glm::length;
using glm::max;
using glm::abs;
using glm::vec2;
using glm::dot;
using glm::cross;
using glm::normalize;
using glm::clamp;
using glm::min;
using glm::floor;
using glm::round;
using std::vector;
using std::thread;

//#define SUPERSAMPLE
#define RANDOM_SUPERSAMPLING
//#define GRID_SUPERSAMPLING
//#define ROTATED_GRID_SUPERSAMPLING


struct distance {
	float field;
	int object_id;
};

void output_image(char* image, int width, int height)
{
	using std::cout;
	cout << "P6\n" << width << " " << height << "\n255\n";
	cout.write(image, width * 3 * height);
}

int image_index(const int x, const int y, const int width)
{
	return (x + y * width) * 3;
}

float lengthn(const vec3& x, const float n)
{
	return pow(pow(x.x, n) + pow(x.y, n) + pow(x.z, n), 1.0 / n);
}

float lengthn(const vec2& x, const float n)
{
	return pow(pow(x.x, n) + pow(x.y, n), 1.0 / n);
}

distance o_union(const distance& a, const distance& b)
{
	if (a.field < b.field)
		return a;
	return b;
}

float d_outline_box(const float d, const vec3& p, const vec3& n, const vec2& s)
{
	return length(max(vec2(d, dot(p, n)) - s, 0.0f));
}

float d_outline_diamond(const float d, const vec3& p, const vec3& n, const float r)
{
	return dot(abs(vec2(d, dot(p, n))), vec2(0.5f)) - r;
}

float d_sphere(const vec3& p, const float r)
{
	return length(p) - r;
}

float d_box(const vec3& p, const vec3& s, const float r = 0.0)
{
	return length(max(abs(p) - s, 0.0)) - r;
}

float d_plane(const vec3& p, const vec3& n)
{
	return dot(p, n);
}

float d_torus(const vec3& p, const vec2& s)
{
	return length(vec2(length(vec2(p.x, p.z)) - s.x, p.y)) - s.y;
}

float d_torus_lengthn(const vec3& p, const vec2& s, const float n)
{
	return lengthn(vec2(lengthn(vec2(p.x, p.z), n) - s.x, p.y), n) - s.y;
}

distance map(const vec3& p, bool show_plane = true)
{
//	return d_box(p, vec3(1.0), 0.0);
//	return d_sphere(p, 1.0);
//	return d_torus(p, vec2(1.0, 0.5));
//	vec4 square_torus = vec4(d_torus_lengthn(p, vec2(1.5, 0.3), 32), 1.0, 0.0, 0.0);
//	vec4 sphere = vec4(d_sphere(p, 1), 0.0, 1.0, 0.0);
//	vec4 plane = vec4(d_plane(p + vec3(0, 3, 0), vec3(0, 1, 0)), 0.0, 0.0, 1.0);
	
//	vec4 res = o_union(square_torus, o_union(plane, sphere));
//	vec4 res = square_torus;
	
	distance plane { 
		d_plane(p + vec3(0, 3, 0), vec3(1, 0, 0)),
		5
	};

	float f = length(p) - 2.0f;
	float d = max(f, length(vec2(p.x, p.y)) - 0.5f);
	d = min(d, max(f, length(vec2(p.z, p.y)) - 0.5f));
	d = min(d, max(f, length(vec2(p.z, p.x)) - 0.5f));
	d = min(max(-f - 0.5f, d), max(-d, f + 1.0f));

	distance shape {
		d,
		1
	};

	//d = d_outline_diamond(shape.field - 0.25, p, normalize(vec3(1, 0, 0)), 0.05);
	//d = min(d, d_outline_diamond(shape.field - 0.25, p, normalize(vec3(0, 1, 0)), 0.05));
	//d = min(d, d_outline_diamond(shape.field - 0.25, p, normalize(vec3(0, 0, 1)), 0.05));

	//d = d_outline_diamond(shape.field, p, normalize(vec3(1, 0, 0)), 0.05);
	d = d_outline_box(shape.field, p, normalize(vec3(1, 0, 0)), vec2(0.1));
	//d = min(d, d_outline_diamond(shape.field, p, normalize(vec3(1, 0, 1)), 0.05));
	//d = min(d, d_outline_diamond(shape.field, p, normalize(vec3(0, 1, 1)), 0.05));

	distance outline {
		d_box(p + vec3(0.99, 0, 0), vec3(1)),
		3
	};
	distance res;
	if (show_plane)
		res = o_union(plane, outline);
	else
		res = outline;
	return res;
}

float softshadow(const vec3& ro, const vec3& rd, const float mint, const float maxt, const float k)
{
    float res = 1.0;
    for( float t=mint; t < maxt; )
    {
        float h = map(ro + rd*t).field;
        if( h<0.0001 )
            return 0.0;
        res = min( res, k*h/t );
        t += h;
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

vec3 get_color(const int id, const vec3 pos)
{
	switch (id)
	{
		case 0:
			return vec3(0);

		case 1:
			return vec3(1, 0, 0);

		case 2:
			return vec3(int(dot(round(pos), vec3(1.0f))) & 1);
		
		case 3:
			return normalize(abs(pos));

		case 4:
			return vec3(0, (glm::simplex(pos * 4.0f) * 0.5 + 0.5), 0);

		case 5:
			return vec3(clamp(map(pos, false).field, 0.0f, 1.0f));
	}
	return vec3(0);
}

bool should_supersample_texture(const int id)
{
	switch(id)
	{
		case 2:
			return true;
	}
	return false;
}

glm::vec3 process(const float x, const float y, const float width, const float height)
{
	const vec2 screenPos = -1.0f + 2.0f * vec2(x, height - y) / vec2(width, height);

	const vec3 camUp = vec3(0, 1, 0);
	const vec3 camTarget = vec3(0);
	const vec3 camPos = vec3(3, 0, 0);
	const vector<vec3> lightPositions { 
		vec3(3, 2, 4)
	};

	const vec3 camDir = normalize(camTarget - camPos);
	const vec3 u = cross(camUp, camDir);
	const vec3 v = cross(camDir, u);
	const vec3 rayDir = normalize(u * screenPos.x * width / height + v * screenPos.y + camDir);

	//vec3 v=cross(camDir, camSize);
	//vec3 camRay = camPos + camDir;
	//vec3 scrCoord=vcv+screenPos.x*u*width/height+screenPos.y*v;
	//vec3 scp=normalize(scrCoord-camPos);

	const float min_dist = 0.00001;
	const float max_dist = 300.0;
	distance dist;
	float total_dist = 0.0;
	int iter = 0;
	vec3 p = camPos;
	for (dist.field = 0.001; dist.field > min_dist && total_dist < max_dist && iter < 512; iter++) {
		p += rayDir * dist.field;
		dist = map(p);
		total_dist += dist.field;
	}
	
	if (dist.field < min_dist) {
		float lighting = 0;
		const float normdist = 0.0001;
		const vec3 n = normalize(vec3(
			dist.field - map(p + vec3(normdist, 0, 0)).field,
			dist.field - map(p + vec3(0, normdist, 0)).field,
			dist.field - map(p + vec3(0, 0, normdist)).field));
		for (const vec3 light : lightPositions) {
			const vec3 lightDir = normalize(p - light);
			//float s = shadow(p, light);
			float s = softshadow(p, normalize(light - p), 0.3, max_dist, 256) * 0.8 + 0.2;
			float b = max(dot(n, lightDir), 0.0f);

			lighting += b;
		}
		lighting = clamp(lighting, 0.05f, 1.0f);
		if (dist.object_id == 5) lighting = 1.0;
		const vec3 object_color = get_color(dist.object_id, p) * lighting;
		vec3 color = object_color;
		return color;
	} else {
		return vec3(0);
	}

	return glm::vec3(1);
}

void render(char* image, const int offset_y, const int max_y, const int width, const int height, const int id)
{
	image += image_index(0, offset_y, width);
	for (int y = offset_y; y < max_y; y++) {
		for (int x = 0; x < width; x++) {
#ifdef SUPERSAMPLE
#ifdef RANDOM_SUPERSAMPLING
			const int num_samples = 32;
			const float offset = 0.5f;
			glm::vec3 color;
			for (int i = 0; i < num_samples; i++) {
				const float xoff = glm::linearRand(-offset, offset);
				const float yoff = glm::linearRand(-offset, offset);
				color += process(x + xoff, y + yoff, width, height);
			}
			color /= num_samples;
#endif
#else
			glm::vec3 color = process(x, y, width, height);
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
	int width = 1024, height = 1024;
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
