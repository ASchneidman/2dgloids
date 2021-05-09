#ifndef _GLITTER_GRIDBIN_H
#define _GLITTER_GRIDBIN_H

#include "glitter.hpp"
#include "Boid.h"
#include <functional>
#include <vector>

class Grid{
    public:
        Grid(glm::vec2 bounds_x, glm::vec2 bounds_y);
        ~Grid();

        glm::vec2 bounds_x; // left and right bounds of grid
        glm::vec2 bounds_y; // top and bottom bounds of grid

        std::vector<Boid *> *boids;

        bool insert(Boid *b);
        void query(Boid *b, std::function<void(Boid *)> &iterate_function);
        void clear();

    private:

};


class GridBin{
    public:
        GridBin();
        ~GridBin();
        
        // screen is divided equally into MxN matrix grid; change these to change grid
        static const int gridDim_M = 100;
        static const int gridDim_N = 10;

        Grid *get_grid(int i, int j);
        Grid *which_grid(Boid *b);
        bool insert(Boid *b);
        void query(Boid *b, std::function<void(Boid *)> &iterate_function);
        void clear();
        
    private:
        Grid *grids[gridDim_M][gridDim_N];
        
};


#endif
