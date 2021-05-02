#ifndef _GLITTER_QUADTREE_H
#define _GLITTER_QUADTREE_H

#include "glitter.hpp"
#include "Boid.h"
#include <functional>
#include <omp.h>
#include <vector>
#include <list>

typedef struct QuadTree QuadTree_t;

class QuadTreeHead;

typedef struct QuadTreeElem {
    // Index into vector of boids of this element's boid
    int boid;
    // Index into vector of QuadTreeElem_t of this element's next
    int next;
} QuadTreeElem_t;

void qt_init(QuadTree_t *qt);
bool qt_insert(QuadTreeHead *head, QuadTree_t *qt, Boid *b, float x_min, float x_max, float y_min, float y_max, int depth);
void qt_query(QuadTreeHead *head, QuadTree_t *qt, Boid *b, std::vector<Boid *> &boids, float x_min, float x_max, float y_min, float y_max);

/*
void qt_clear(QuadTreeHead *head, QuadTree_t *qt);
void qt_clear_left(QuadTreeHead *head, QuadTree_t *qt);
void qt_clear_right(QuadTreeHead *head, QuadTree_t *qt);
void qt_visualize(QuadTreeHead *head, QuadTree_t *qt, float x_min, float x_max, float y_min, float y_max);
void qt_free(QuadTreeHead *head, QuadTree_t *qt);
*/

class QuadTreeHead {
    public:
        QuadTreeHead(glm::vec2 bounds_x, glm::vec2 bounds_y, std::vector<Boid *> *boids);
        //~QuadTreeHead();

        bool insert(Boid *b) {
            return qt_insert(this, nodes[0], b, bounds_x.x, bounds_x.y, bounds_y.x, bounds_y.y, 0);
        };

        void query(Boid *b, std::vector<Boid *> &boids) {
            qt_query(this, nodes[0], b, boids, bounds_x.x, bounds_x.y, bounds_y.x, bounds_y.y);
        };
        //void visualize();
        void clear();

        int alloc_elem() {
            assert(!free_elements.empty());
            int ret = free_elements.front();
            free_elements.pop_front();
            return ret;
        }

        void dealloc_elem(int elem_index) {
            free_elements.push_back(elem_index);
        }


        std::vector<QuadTree_t *>nodes;
        std::vector<QuadTreeElem_t *> elements;

        std::list<int> free_elements;

        std::vector<Boid *> *boids;

        glm::vec2 bounds_x;
        glm::vec2 bounds_y;

        #ifdef VISUALIZE
        GLuint lineShaderProgram;
        #endif
};

typedef struct QuadTree {
    int num_boids = 0;
    // Children
    int first_element;
    int first_child;

    bool is_subdivided = false;

    /*
    #ifdef VISUALIZE
    bool buffers_initialized = 0;
    GLuint VBO;
    GLuint VAO;
    #endif
    */
} QuadTree_t;


#endif