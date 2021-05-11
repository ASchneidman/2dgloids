#ifndef _GLITTER_QUADTREE_H
#define _GLITTER_QUADTREE_H

#include "glitter.hpp"
#include "Boid.h"
#include <functional>
#include <omp.h>
#include <vector>
#include <list>

typedef struct QuadTree {
    int num_boids = 0;
    // Children
    int first_element;
    int first_child;

    int is_subdivided = false;

    omp_lock_t insert_lock;
    omp_lock_t subdivide_lock;
} QuadTree_t;


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


class QuadTreeHead {
    public:
        QuadTreeHead(glm::vec2 bounds_x, glm::vec2 bounds_y, std::vector<Boid *> *boids);

        bool insert(Boid *b) {
            return qt_insert(this, nodes[0], b, bounds_x.x, bounds_x.y, bounds_y.x, bounds_y.y, 0);
        };

        void query(Boid *b, std::vector<Boid *> &boids) {
            qt_query(this, nodes[0], b, boids, bounds_x.x, bounds_x.y, bounds_y.x, bounds_y.y);
        };
        void clear();

        int alloc_children() {
            int ret = 0;

            QuadTree_t *children = new QuadTree_t[4];

            omp_set_lock(&nodes_lock);

            nodes.push_back(&children[0]);
            ret = nodes.size() - 1;
            nodes.push_back(&children[1]);
            nodes.push_back(&children[2]);
            nodes.push_back(&children[3]);

            omp_unset_lock(&nodes_lock);

            qt_init(&children[0]);
            qt_init(&children[1]);
            qt_init(&children[2]);
            qt_init(&children[3]);
            return ret;
        }


        std::vector<QuadTree_t *>nodes;
        QuadTreeElem_t elements[NUM_BOIDS];

        std::vector<Boid *> *boids;

        glm::vec2 bounds_x;
        glm::vec2 bounds_y;

        omp_lock_t nodes_lock;
};




#endif