#include "common.h"
#include <vector>

using std::vector;

// Returns the distance to the scene from position p
distance map(const vec3& p);

// Returns the color of the object with material id at position p
vec3 get_color(const int id, const vec3& p);

// Returns the position of the camera in the scene
vec3 get_camera_position();

// Returns the target of the camera
vec3 get_camera_target();

// Returns a list of the lights in the the scene
vector<vec3> get_lights();

// Returns the minimum distance that is considered as hitting an object
float get_dist_epsilon();

// Returns the maximum distance to march to
float get_max_dist();

// Returns the up vector of the camera
vec3 get_camera_up();

// Returns the maximum number of iterations in the raymarching loop
int get_max_iter();
