#ifndef _GLITTER_QUADTREE_H
#define _GLITTER_QUADTREE_H

#include "glitter.hpp"
#include "Boid.h"
#include <functional>


class QuadTree {

public:
    QuadTree(glm::vec2 bounds_x, glm::vec2 bounds_y): bounds_x(bounds_x), bounds_y(bounds_y), num_boids(0) {
        if (VISUALIZE) {
            glGenBuffers(1, &VBO);
            glGenVertexArrays(1, &VAO);
        }
    };
    ~QuadTree();
    // Bounding Box
    glm::vec2 bounds_x;
    glm::vec2 bounds_y;

    // Boids in this QuadTree
    int num_boids;
    Boid *boids[NODE_CAPACITY];

    bool is_subdivided = false;

    // Children
    QuadTree *upperLeft = NULL;
    QuadTree *upperRight = NULL;
    QuadTree *lowerLeft = NULL;
    QuadTree *lowerRight = NULL;

    bool insert(Boid *b);
    void query(Boid *b, std::function<void(Boid *)> &iterate_function);
    void clear();
    void visualize();

private:
    GLuint VBO;
    GLuint VAO;
};


class QuadTreeHead {
    public:
        QuadTreeHead(glm::vec2 bounds_x, glm::vec2 bounds_y);
        ~QuadTreeHead();

        bool insert(Boid *b) {
            return first->insert(b);
        };
        void query(Boid *b, std::function<void(Boid *)> &iterate_function) {
            first->query(b, iterate_function);
        };
        void visualize();
        void clear();
    private:
        QuadTree *first;
        GLuint lineShaderProgram;
};



#endif