#ifndef _GLITTER_GRIDBIN_H
#define _GLITTER_GRIDBIN_H

#include "glitter.hpp"
#include "Boid.h"
#include <functional>

class Grid{
    public:
        Grid(glm::vec2 bounds_x, glm::vec2 bounds_y);
        ~Grid();

        glm::vec2 bounds_x; // left and right bounds of grid
        glm::vec2 bounds_y; // top and bottom bounds of grid

        int num_boids;
        Boid *boids[NODE_CAPACITY];

        bool insert(Boid *b);
        void query(Boid *b, std::function<void(Boid *)> &iterate_function);
        void clear();
        // void visualize();

    private:

};


class GridBin{
    public:
        GridBin();
        ~GridBin();
        
        // screen is divided equally into MxN matrix grid; change these to change grid
        static const int gridDim_M = 20;
        static const int gridDim_N = 10;

        Grid which_grid(Boid *b);
        bool insert(Boid *b);
        void query(Boid *b, std::function<void(Boid *)> &iterate_function);
        // void visualize();
        void clear();

    private:
        
        Grid *grids[gridDim_M][gridDim_N];
};


#endif
