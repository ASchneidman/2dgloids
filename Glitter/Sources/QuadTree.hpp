#ifndef _GLITTER_QUADTREE_H
#define _GLITTER_QUADTREE_H

#include "glitter.hpp"
#include "Boid.h"
#include <functional>

#define NODE_CAPACITY 100


class QuadTree {

public:
    QuadTree(glm::vec2 bounds_x, glm::vec2 bounds_y): bounds_x(bounds_x), bounds_y(bounds_y), num_boids(0) {};
    ~QuadTree();
    // Bounding Box
    glm::vec2 bounds_x;
    glm::vec2 bounds_y;

    // Boids in this QuadTree
    int num_boids;
    Boid *boids[NODE_CAPACITY];

    // Children
    QuadTree *upperLeft = NULL;
    QuadTree *upperRight = NULL;
    QuadTree *lowerLeft = NULL;
    QuadTree *lowerRight = NULL;

    bool insert(Boid *b);
    void query(Boid *b, std::function<void(Boid *)> &iterate_function);
};


#endif