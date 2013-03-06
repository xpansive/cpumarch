#pragma once

#include <glm/glm.hpp>
#include <glm/ext.hpp>

using glm::vec2;
using glm::vec3;
using glm::vec4;
using glm::pow;
using glm::length;
using glm::max;
using glm::abs;
using glm::dot;
using glm::cross;
using glm::normalize;
using glm::clamp;
using glm::min;
using glm::floor;
using glm::round;
using glm::reflect;
using glm::inversesqrt;
using glm::mix;
using glm::mod;

struct distance {
	float field;
	int object_id;
};

//
// Math utilities
//

float lengthn(const vec3& x, const float n);
float lengthn(const vec2& x, const float n);

//
// Distance operators
//

// Returns the union of two distance fields
distance o_union(const distance& a, const distance& b);

// Returns the intersection of two distance fields
distance o_intersect(const distance& a, const distance& b);

// Subracts b from a
distance o_subtract(const distance& a, const distance& b);


//
// Primitive distance functions
// All objects are centered at the origin, displace p to translate them
//

// Returns the distance to a sphere with radius r
float d_sphere(const vec3& p, const float r);

// Returns the distance to a box with size s and rounded corners with radius r
float d_box(const vec3& p, const vec3& s, const float r);
float d_box_signed(const vec3& p, const vec3& s, const float r);

// Returns the distance to a plane with normal n
float d_plane(const vec3& p, const vec3& n);

// Returns the distance to a torus with radius r
float d_torus(const vec3& p, const vec2& r);
float d_torus_lengthn(const vec3& p, const vec2& r, const float n);


//
// Distance relative distance functions
//

// Outlines a distance with a diamond shape of normal n and radius r
float d_outline_diamond(const float d, const vec3& p, const vec3& n, const float r);


//
// Fractal distance functions
//

// Returns the distance to a mandelbulb of power 8
float d_mandelbulb(const vec3& p);

// Returns the distance to a mandelbulb of power n
float d_mandelbulb(const vec3& p, const float n);

float d_menger_sponge(const vec3& p);

//
// Raymarching functions
//

// Marches a ray from pos in direction dir until it hits an object. Returns object color.
vec3 raymarch(const vec3& pos, const vec3& dir, const int userdata);


//
// Lighting functions
//

float subsurface_scattering(const vec3& p, const vec3& n, const float d);
float ambient_occlusion(const vec3& p, const vec3& n, const float d);
float shadow(vec3 p, const vec3& l);
float softshadow(const vec3& ro, const vec3& rd, const float mint, const float maxt, const float k);
vec3 get_normal(const float dist, const vec3& p);
