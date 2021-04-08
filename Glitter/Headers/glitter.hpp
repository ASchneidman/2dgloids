// Preprocessor Directives
#ifndef GLITTER
#define GLITTER
//#pragma once

// System Headers
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

// Reference: https://github.com/nothings/stb/blob/master/stb_image.h#L4
// To use stb_image, add this in *one* C++ source file.
//     #define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>


// Boid related constants

#define NUM_BOIDS 1500
#define BOID_SPEED 100.0f

#define MAX_VEL 1800.0f
#define MAX_FORCE 1000000.0f

#define COLLISION_WEIGHT (1.f)
#define ALIGN_WEIGHT (1.f)
#define POSITION_WEIGHT (1.f)

#define NEARBY_DIST 70.0f

#define ER_DEFAULT_SIZE 5.0f
#define ER_DEFAULT_ROT 0.0f

// The Width of the screen
const unsigned int SCREEN_WIDTH = 1700;
// The height of the screen
const unsigned int SCREEN_HEIGHT = 900;

extern float max_velocity;
extern float nearby_dist;

extern float collision_weight;
extern float align_weight;
extern float position_weight;

#endif //~ Glitter Header
