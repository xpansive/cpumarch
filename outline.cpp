#include "scene.h"

distance map(const vec3& p)
{
	float f = length(p) - 2.0f;
	float d = max(f, length(vec2(p.x, p.y)) - 0.5f);
	d = min(d, max(f, length(vec2(p.z, p.y)) - 0.5f));
	d = min(d, max(f, length(vec2(p.z, p.x)) - 0.5f));
	d = min(max(-f - 0.5f, d), max(-d, f + 1.0f));

	distance shape {
		d,
		0
	};

	d = d_outline_diamond(shape.field - 0.25, p, vec3(1, 0, 0), 0.05);
	d = min(d, d_outline_diamond(shape.field - 0.25, p, vec3(0, 1, 0), 0.05));
	d = min(d, d_outline_diamond(shape.field - 0.25, p, vec3(0, 0, 1), 0.05));

	distance outline {
		d,
		1
	};

	d = d_plane(p + vec3(0, 3, 0), vec3(0, 1, 0));

	distance floor {
		d,
		2
	};

	return o_union(floor, o_union(shape, outline));
}

vec3 get_color(const int id, const vec3& pos)
{
	switch (id)
	{
		case 0:
			return vec3(1, 0, 0);

		case 1:
			return normalize(abs(pos));

		case 2:
			return vec3(int(dot(round(vec2(pos.x, pos.z)), vec2(1.0f))) & 1);
	}
	return vec3(0);
}

vec3 get_camera_position()
{
	return vec3(3, 2, 1);
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
	return { vec3(3, 3, 3) };
}

float get_dist_epsilon()
{
	return 0.00001;
}

float get_max_dist()
{
	return 300;
}

int get_max_iter()
{
	return 512;
}
