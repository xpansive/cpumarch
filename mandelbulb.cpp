#include "scene.h"

distance map(const vec3& p)
{
	return o_union({
		d_mandelbulb(p), 0
	}, { 
		d_plane(p+vec3(0,3,0), vec3(0,1,0)), 1
	});
}

vec3 get_color(const int id, const vec3& pos)
{
	switch (id)
	{
		case 0:
			return normalize(abs(pos));

		case 1:
			return vec3(int(dot(round(vec2(pos.x, pos.z)), vec2(1.0f))) & 1);
	}
	return vec3(0);
}

vec3 get_camera_position()
{
	return vec3(0.3, 0.3, 1.5);
}

vec3 get_camera_target()
{
	return vec3(0);
}

vec3 get_camera_up()
{
	return vec3(0, 1, 0);
}

vector<vec3> get_lights()
{
	return { vec3(-3, 3, 3) };
}

float get_dist_epsilon()
{
	return 0.001;
}

float get_max_dist()
{
	return 300;
}

int get_max_iter()
{
	return 256;
}
