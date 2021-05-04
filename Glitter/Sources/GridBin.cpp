#include "GridBin.hpp"
#include <iostream>
#include <omp.h>


// Grid class
Grid::Grid(glm::vec2 bounds_x, glm::vec2 bounds_y): bounds_x(bounds_x), bounds_y(bounds_y) {
    boids = new std::vector<Boid *>; // malloc ?
}

Grid::~Grid() {
    delete boids;
}

bool Grid::insert(Boid *b) {
    if (b->position.x < bounds_x.x || b->position.x > bounds_x.y || 
        b->position.y < bounds_y.x || b->position.y > bounds_y.y) {
            return false;
    }
    boids->push_back(b);
    return true;
}

/**
 * Credit to 15-418/618 Staff for this function!!
 */ 
bool circleInRect(glm::vec2 &circle_pos, glm::vec2 &bounds_x, glm::vec2 &bounds_y) {
    // clamp circle center to box (finds the closest point on the box)
    float closestX = (circle_pos.x > bounds_x.x) ? ((circle_pos.x < bounds_x.y) ? circle_pos.x : bounds_x.y) : bounds_x.x;
    float closestY = (circle_pos.y > bounds_y.x) ? ((circle_pos.y < bounds_y.y) ? circle_pos.y : bounds_y.y) : bounds_y.x;

    // is circle radius less than the distance to the closest point on
    // the box?
    float distX = closestX - circle_pos.x;
    float distY = closestY - circle_pos.y;

    if ( ((distX * distX) + (distY * distY)) <= (nearby_dist * nearby_dist) ) {
        return true;
    } else {
        return false;
    }
}

void Grid::query(Boid *b, std::function<void(Boid *)> &iterate_function) {
    // Check if sphere of influence intersects this bbox
    if (!circleInRect(b->position, bounds_x, bounds_y)) {
        return;
    }
    for (int i = 0; i < boids->size(); i++) {
        iterate_function((*boids)[i]);
    }
    return;
}

void Grid::clear() {
    boids->clear();
}

// GridBin class
GridBin::GridBin() {
    // make grid 
    float grid_x = ((float)SCREEN_WIDTH) / gridDim_M;
    float grid_y = ((float)SCREEN_HEIGHT) / gridDim_N;

    // TODO: maybe parallelize 
    for (int i = 0; i < gridDim_M; i++) {
        for (int j = 0; j < gridDim_N; j++) {
            // set bounds of each grid
            glm::vec2 bounds_x(grid_x * i, grid_x * (i + 1));
            glm::vec2 bounds_y(grid_y * j, grid_y * (j + 1));
            grids[i][j] = new Grid(bounds_x, bounds_y);
        }
    }
}

GridBin::~GridBin() {
    for (int i = 0; i < gridDim_M; i++) {
        for (int j = 0; j < gridDim_N; j++) {
            // delete each grid
            delete grids[i][j];
        }
    }
}

Grid *GridBin::get_grid(int i, int j) {
    return grids[i][j];
}

Grid *GridBin::which_grid(Boid *b) {
    float percent_x = b->position.x / SCREEN_WIDTH;
    float percent_y = b->position.y / SCREEN_HEIGHT;
    int x_grid = gridDim_M * percent_x;
    int y_grid = gridDim_N * percent_y;
    return grids[x_grid][y_grid];
}

bool GridBin::insert(Boid *b) {
    // find grid to insert boid in
    Grid *grid_idx = which_grid(b);
    grid_idx->insert(b);
    return true;
}


void GridBin::query(Boid *b, std::function<void(Boid *)> &iterate_function) {
    // find grid query
    for (int i = 0; i < gridDim_M; i++) {
        for (int j = 0; j < gridDim_N; j++) {
            grids[i][j]->query(b, iterate_function);
        }
    }
}

void GridBin::clear() {
    #pragma omp parallel for collapse(2) num_threads(NUM_THREADS)
    for (int i = 0; i < gridDim_M; i++) {
        for (int j = 0; j < gridDim_N; j++) {
            grids[i][j]->clear();
        }
    }
}
