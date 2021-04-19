#include "GridBin.hpp"
#include <iostream>


// Grid class
Grid::Grid(glm::vec2 bounds_x, glm::vec2 bounds_y): bounds_x(bounds_x),
    bounds_y(bounds_y),
    num_boids(0) {
}

Grid::~Grid() {

}

int num_boids;
Boid *boids[NODE_CAPACITY];

bool Grid::insert(Boid *b) {
    if (b->position.x < bounds_x.x || b->position.x >= bounds_x.y || 
        b->position.y < bounds_y.x || b->position.y >= bounds_y.y) {
            return false;
    }

    if (num_boids < NODE_CAPACITY) {
        boids[num_boids] = b;
        num_boids += 1;
        return true;
    }

    return false;
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
    for (int i = 0; i < num_boids; i++) {
        iterate_function(boids[i]);
    }
    return;
}

void Grid::clear() {
    num_boids = 0;
}

// void Grid::visualize();



// GridBin class
GridBin::GridBin() {
    // make grid 
    // Grid *grids[gridDim_M][gridDim_N];
    float grid_x = SCREEN_WIDTH / gridDim_M;
    float grid_y = SCREEN_HEIGHT / gridDim_N;
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

Grid GridBin::which_grid(Boid *b) {
    // search function can be more efficient 
    float grid_x = SCREEN_WIDTH / gridDim_M;
    float grid_y = SCREEN_HEIGHT / gridDim_N;
    for (int i = 0; i < gridDim_M; i++) {
        for (int j = 0; j < gridDim_N; j++) {
            if (b->position.x >= grids[i][j]->bounds_x.x && b->position.x < grids[i][j]->bounds_x.y
             && b->position.y >= grids[i][j]->bounds_y.x && b->position.y < grids[i][j]->bounds_y.y) {
                 return *grids[i][j];
             }
        }
    }
}

bool GridBin::insert(Boid *b) {
    // find grid to insert boid in
    Grid grid_idx = which_grid(b);
    return grid_idx.insert(b);
}


void GridBin::query(Boid *b, std::function<void(Boid *)> &iterate_function) {
    // find grid query
    Grid grid_idx = which_grid(b);
    grid_idx.query(b, iterate_function);
}

// void GridBin::visualize();

void GridBin::clear() {
    for (int i = 0; i < gridDim_M; i++) {
        for (int j = 0; j < gridDim_N; j++) {
            grids[i][j]->clear();
        }
    }
}
