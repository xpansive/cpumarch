#include "scene.h"

distance map(const vec3& p)
{
	return o_union({
		d_mandelbulb(p), 0
	}, {
		d_plane(p+vec3(0,1,0), vec3(0,1,0)), 1
	});
}

vec3 get_color(const int id, const vec3& pos)
{
	switch (id)
	{
		case 0:
			return normalize(abs(pos));

		case 1:
			return vec3(int(dot(round(pos), vec3(1.0f))) & 1);
	}
	return vec3(0);
}

vec3 calculate_color(const distance& dist, const vec3& p, const vec3& ray, const int userdata)
{
	vec3 normal = get_normal(dist.field, p);

	const float eta = 0.3;
	const float sqf = (1 - eta) / (1 + eta);
	const float f = sqf * sqf;
	const float fresnel_power = 5;
	const float ratio = f + (1 - f) * pow(1 - dot(ray, normal), fresnel_power);

	vec3 diffuse = get_color(dist.object_id, p);
	if (dist.object_id == 2 && userdata < 3) {
		vec3 reflect_ray = glm::reflect(ray, normal);
		diffuse = mix(diffuse, raymarch(p + reflect_ray * 0.01f, reflect_ray, userdata + 1), ratio);
	}
	vec3 color(0.1);
	for (vec3 light : get_lights()) {
		const vec3 light_dir = normalize(p - light);
		const float light_dist = glm::distance(p, light);
		color += max(dot(normal, light_dir), 0.0f) * diffuse;
		const vec3 half = normalize(light_dir + ray);
		color += pow(max(dot(normal, half), 0.0f), 64.0f);
		color *= softshadow(p, -light_dir, 0.1, light_dist, 64) * 0.7 + 0.3;
	}
	//color *= mix(vec3(1), get_background_color(), 0.5 + 0.5 * dot(normal, vec3(0, -1, 0)));
	//color = pow(color, vec3(0.45f));
	color = clamp(color, 0.0f, 1.0f);
	return color;
}

vec3 get_background_color()
{
	return vec3(0.5, 0.7, 0.9);	
}

vec3 get_camera_position()
{
	//return vec3(0.6, -0.3, 2);
	return vec3(0.7, 1, 1.5);
}

vec3 get_camera_target()
{
	return vec3(0, 0, 0);
}

vec3 get_camera_up()
{
	return vec3(0, 1, 0);
}

vector<vec3> get_lights()
{
	return { vec3(2, 3, 2) };
}

float get_dist_epsilon()
{
	return 0.0025;
}

float get_max_dist()
{
	return 200;
}

int get_max_iter()
{
	return 512;
}
