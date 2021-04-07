// Preprocessor Directives
#ifndef GLITTER
#define GLITTER
#pragma once

// System Headers
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

// Reference: https://github.com/nothings/stb/blob/master/stb_image.h#L4
// To use stb_image, add this in *one* C++ source file.
//     #define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>


// Boid related constants
#define MAX_VELOCITY 700.0f
#define MAX_FORCE 600.0f

#define NUM_BOIDS 100000
#define BOID_SPEED 10.0f

#define COLLISION_WEIGHT (1.3f)
#define ALIGN_WEIGHT (1.5f)
#define POSITION_WEIGHT (1.5f)

#define NEARBY_DIST 250.0f

#define ER_DEFAULT_SIZE 5.0f
#define ER_DEFAULT_ROT 0.0f

// The Width of the screen
const unsigned int SCREEN_WIDTH = 6000;
// The height of the screen
const unsigned int SCREEN_HEIGHT = 2000;

#endif //~ Glitter Header
