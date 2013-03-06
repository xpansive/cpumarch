#include "common.h"
#include "scene.h"

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
	return dot(abs(vec2(d, dot(p, n))), vec2(0.5)) - r;
}

float d_outline_cylinder(const float d, const vec3& p, const vec3& n, const float r)
{
	return length(vec2(d, dot(p, n))) - r;
}

float d_sphere(const vec3& p, const float r)
{
	return length(p) - r;
}

float d_box(const vec3& p, const vec3& s, const float r = 0)
{
	return length(max(abs(p) - s + r, 0)) - r;
}

float d_box_signed(const vec3& p, const vec3& s, const float r = 0)
{
	const vec3 d = abs(p) - s;
 	return min(max(d.x, max(d.y, d.z)), 0.0f) + length(max(d, 0.0f)) - r;
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

void powN1(vec3& z, const float r, float& dr) {
	const float power = 8;
	// extract polar coordinates
	float theta = acos(z.z/r);
	float phi = atan(z.y/z.x);
	dr =  pow( r, power-1.0)*power*dr + 1.0;
	
	// scale and rotate the point
	float zr = pow( r,power);
	theta = theta*power;
	phi = phi*power;
	
	// convert back to cartesian coordinates
	z = zr*vec3(sin(theta)*cos(phi), sin(phi)*sin(theta), cos(theta));
}

void powN1fast(vec3& w)
{
	const float x2 = w.x*w.x; const float x4 = x2*x2;
	const float y2 = w.y*w.y; const float y4 = y2*y2;
	const float z2 = w.z*w.z; const float z4 = z2*z2;

	const float k3 = x2 + z2;
	const float k2 = glm::inversesqrt(k3*k3*k3*k3*k3*k3*k3);
	const float k5 = x2*z2*2;
	const float k1 = x4 + y4 + z4 - 6*y2*z2 - 6*x2*y2 + k5;
	const float k4 = k3 - y2;

	w.x =  64*w.x*w.y*w.z*(x2 - z2)*k4*(x4 - 3*k5+z4)*k1*k2;
	w.z = -8*w.y*k4*(x4*x4 - 14*x4*k5 + 70*x4*z4 - 14*k5*z4 + z4*z4)*k1*k2;
	w.y = -16*y2*k3*k4*k4 + k1*k1;
}

float d_mandelbulb(const vec3& p) {
	const float bailout = 2;
	vec3 w = p;
	float r = 0;
	float dr = 1;
	int i;
	for(i = 0; i < 4; i++) {
		const float x2 = w.x*w.x;
		const float y2 = w.y*w.y; 
		const float z2 = w.z*w.z;
		const float k3 = x2 + z2;
		r = sqrt(k3 + y2);
		if (r > bailout)
			return 0.5 * log(r) * r / dr;
		dr = dr*r*r*r*r*r*r*r*8 + 1;
		const float x4 = x2*x2;
		const float y4 = y2*y2;
		const float z4 = z2*z2;
		const float k2 = inversesqrt(k3*k3*k3*k3*k3*k3*k3);
		const float k5 = x2*z2*2;
		const float k1 = x4 + y4 + z4 - 6*y2*z2 - 6*x2*y2 + k5;
		const float k4 = k3 - y2;
		w.x =  64*w.x*w.y*w.z*(x2 - z2)*k4*(x4 - 3*k5+z4)*k1*k2 + p.x;
		w.z = -8*w.y*k4*(x4*x4 - 14*x4*k5 + 70*x4*z4 - 14*k5*z4 + z4*z4)*k1*k2 + p.z;
		w.y = -16*y2*k3*k4*k4 + k1*k1 + p.y;
	}
	r = length(w);
	return 0.5 * log(r) * r / dr;
}

float d_menger_sponge(const vec3& p)
{
	float d = d_box(p, vec3(1.0));

	float s = 1;
	for(int m = 0; m < 5; m++)
	{
		const vec3 a = mod(p * s, 2.0f) - 1.0f;
		s *= 3;
		const vec3 r = abs(1.0f - abs(a) * 3.0f);

		const float da = max(r.x, r.y);
		const float db = max(r.y, r.z);
		const float dc = max(r.z, r.x);
		const float c = (min(da, min(db, dc)) - 1) / s;

		d = max(d, c);
	}
	return d;
}

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

float ambient_occlusion(const vec3& p, const vec3& n, const float d)
{
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

vec3 raymarch(const vec3& pos, const vec3& dir, const int userdata)
{
	static const float min_dist = get_dist_epsilon();
	static const float max_dist = get_max_dist();
	static const int max_iter = get_max_iter();
	static const vector<vec3> lightPositions = get_lights();
	static const vec3 background_color = get_background_color();

	distance dist { 0, 0 };
	float total_dist = 0;
	vec3 p = pos;
	int iter;
	for (iter = 0; iter < max_iter; iter++) {
		dist = map(p);
		if (dist.field < min_dist || total_dist > max_dist)
			break;
		p += dir * dist.field;
		total_dist += dist.field;
	}

	if (dist.field < min_dist) {
		const vec3 color = calculate_color(dist, p, dir, userdata);
		return color;
	} else {
		return background_color;
	}
}

vec3 get_normal(const float dist, const vec3& p)
{
	static const float normdist = 0.0001;
	const vec3 n = normalize(vec3(
				dist - map(p + vec3(normdist, 0, 0)).field,
				dist - map(p + vec3(0, normdist, 0)).field,
				dist - map(p + vec3(0, 0, normdist)).field));
	return n;
}
