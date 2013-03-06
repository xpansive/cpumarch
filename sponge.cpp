#include "scene.h"

distance map(const vec3& p)
{
	return {
		d_menger_sponge(mod(p + 0.5f, 1.0f) - 0.5f), 0
	};
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
	if (userdata == 0) {
		vec3 reflect_ray = glm::reflect(ray, normal);
		diffuse = mix(diffuse, raymarch(p + reflect_ray * 0.1f, reflect_ray, userdata + 1), ratio);
	}
	vec3 color(0);
	for (vec3 light : get_lights()) {
		const vec3 light_dir = normalize(p - light);
		const float light_dist = glm::distance(p, light);
		color += max(dot(normal, light_dir), 0.0f) * diffuse;
		//const vec3 half = normalize(-light_dir + ray);
		//color += pow(max(dot(normal, half), 0.0f), 128.0f);
		color *= softshadow(p, -light_dir, 0.1, light_dist, 32) * 0.7 + 0.3;
	}
	//color *= ambient_occlusion(p, normal, 0.1);
	//color *= mix(vec3(1), get_background_color(), 0.5 + 0.5 * dot(normal, vec3(0, -1, 0)));
	//color = pow(color, vec3(0.45f));
	color = clamp(color, 0.0f, 1.0f);
	return color;
}

vec3 get_background_color()
{
	return vec3(0);	
}

vec3 get_camera_position()
{
	return vec3(0.1, -0.3, 0.6);
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
	return { vec3(0) };
}

float get_dist_epsilon()
{
	return 0.00001;
}

float get_max_dist()
{
	return 200;
}

int get_max_iter()
{
	return 512;
}
