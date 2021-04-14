#ifndef _GLITTER_GRIDBIN_H
#define _GLITTER_GRIDBIN_H

#include "glitter.hpp"
#include "Boid.h"
#include <functional>

class GridBin {

public:
    GridBin(glm::vec2 bounds_x, glm::vec2 bounds_y): bounds_x(bounds_x), bounds_y(bounds_y), num_boids(0) {
        if (VISUALIZE) {
            glGenBuffers(1, &VBO);
            glGenVertexArrays(1, &VAO);
        }
    };
    ~GridBin();
    // Bounding Box
    glm::vec2 bounds_x;
    glm::vec2 bounds_y;

    // Boids in this GridBin
    int num_boids;
    Boid *boids[NODE_CAPACITY];

    bool insert(Boid *b);
    void query(Boid *b, std::function<void(Boid *)> &iterate_function);
    void clear();
    void visualize();

private:
    GLuint VBO;
    GLuint VAO;
};

#endif