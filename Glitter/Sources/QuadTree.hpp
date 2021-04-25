#ifndef _GLITTER_QUADTREE_H
#define _GLITTER_QUADTREE_H

#include "glitter.hpp"
#include "Boid.h"
#include <functional>
#include <omp.h>
#include <vector>

typedef struct QuadTree {
    int num_boids = 0;

    bool is_subdivided = false;

    // Children
    struct QuadTree *upperLeft = NULL;
    struct QuadTree *upperRight = NULL;
    struct QuadTree *lowerLeft = NULL;
    struct QuadTree *lowerRight = NULL;

    omp_lock_t *insert_lock;
    omp_lock_t *subdivide_lock;

    #ifdef VISUALIZE
    bool buffers_initialized = 0;
    GLuint VBO;
    GLuint VAO;
    #endif

    Boid *boids[NODE_CAPACITY];
} QuadTree_t;

void qt_init(QuadTree_t *qt);
bool qt_insert(QuadTree_t *qt, Boid *b, float x_min, float x_max, float y_min, float y_max);

//void qt_query(QuadTree_t *qt, Boid *b, std::function<void(Boid *)> &iterate_function, float x_min, float x_max, float y_min, float y_max);
void qt_query(QuadTree_t *qt, Boid *b, std::vector<Boid *> &boids, float x_min, float x_max, float y_min, float y_max);

void qt_clear(QuadTree_t *qt);
void qt_clear_left(QuadTree_t *qt);
void qt_clear_right(QuadTree_t *qt);
void qt_visualize(QuadTree_t *qt, float x_min, float x_max, float y_min, float y_max);
void qt_free(QuadTree_t *qt);

class QuadTreeHead {
    public:
        QuadTreeHead(glm::vec2 bounds_x, glm::vec2 bounds_y);
        ~QuadTreeHead();

        bool insert(Boid *b) {
            return qt_insert(first, b, bounds_x.x, bounds_x.y, bounds_y.x, bounds_y.y);
        };

        //void query(Boid *b, std::function<void(Boid *)> &iterate_function) {
        void query(Boid *b, std::vector<Boid *> &boids) {
            //qt_query(first, b, iterate_function, bounds_x.x, bounds_x.y, bounds_y.x, bounds_y.y);
            qt_query(first, b, boids, bounds_x.x, bounds_x.y, bounds_y.x, bounds_y.y);
        };
        void visualize();
        void clear();
    private:
        QuadTree_t *first;
        std::vector<QuadTree_t *>nodes;

        glm::vec2 bounds_x;
        glm::vec2 bounds_y;

        #ifdef VISUALIZE
        GLuint lineShaderProgram;
        #endif
};



#endif