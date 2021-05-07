//
// Created by Alex Schneidman on 2020-05-09.
//

#include "State.h"
#include "resourcemanager.h"
#include <random>
#include <map>
#include <iterator>

#include <iostream>
#include <sstream>
#include <fstream>
#include <numeric>
#include <functional>
#include <algorithm>

#include <omp.h>
#include <glm/gtx/color_space.hpp>
#include <GLFW/glfw3.h>

#define SIGN(x) ((x) < 0.0f ? -1.0f : 1.0f)

int do_update = 0;
int update_frames = 1;

std::uniform_real_distribution<float> randDir(0.0f, 2 * glm::pi<float>());


EntityRenderer *Renderer;
std::default_random_engine generator(10);
std::uniform_real_distribution<float> distribution(-100.0,100.0);

State::State(GLuint width, GLuint height) : Width(width), Height(height) {}

void State::Init() {
    // load shaders
    ResourceManager::LoadShader("Glitter/entity.vert", "Glitter/entity.frag", nullptr, "entity");
    // configure shaders
    glm::mat4 projection = glm::ortho(0.0f, static_cast<float>(this->Width),
                                      static_cast<float>(this->Height), 0.0f, -1.0f, 1.0f);
    ResourceManager::GetShader("entity").Use().SetMatrix4("projection", projection);
    // set render-specific controls
    Shader sh = ResourceManager::GetShader("entity");
    Renderer = new EntityRenderer(sh);

    std::uniform_real_distribution<float> posDistX(0.0f, this->Width-1);
    std::uniform_real_distribution<float> posDistY(0.0f, this->Height-1);


    std::uniform_real_distribution<float> randSpeed(0.0f, BOID_SPEED);


    for (int i = 0; i < NUM_BOIDS; i++) {
        glm::vec2 initPos = glm::vec2(posDistX(generator), posDistY(generator));
        float theta = randDir(generator);
        glm::vec2 initVel = glm::vec2(glm::sin(theta), glm::cos(theta));
    
        glm::vec2 initialVel = initVel * randSpeed(generator);
        this->boids.push_back(new Boid(initPos, initialVel, this->Width, this->Height, i));
    }


    forces = new GLfloat[NUM_BOIDS * 2];
    //grid_cell_sizes = new GLint[N_ROWS * N_COLS];
    grid_cell_sizes = new std::vector<int>(N_ROWS * N_COLS, 0);
    //inputs = new GLfloat[NUM_BOIDS * 4];
    inputs = new GLint[NUM_BOIDS];


    for (int r = 0; r < N_ROWS; r++) {
        for (int c = 0; c < N_COLS; c++) {
            grid[r][c] = new std::vector<int>();
            omp_init_lock(&locks[r][c]);
        }
    }
}



void State::Update(GLfloat dt) {
    position_velocity.clear();
    // clear grid
    #pragma omp parallel for collapse(2) num_threads(THREADS)
    for (int r = 0; r < N_ROWS; r++) {
        for (int c = 0; c < N_COLS; c++) {
            grid[r][c]->clear();
        }
    }


    // Build grid
    #pragma omp parallel for schedule(dynamic, 100) num_threads(THREADS)
    for (int i = 0; i < boids.size(); i++) {
        Boid *b = boids[i];
        float percent_x = b->position.x / SCREEN_WIDTH;
        float percent_y = b->position.y / SCREEN_HEIGHT;
        
        int r = percent_x * N_ROWS;
        int c = percent_y * N_COLS;

        omp_set_lock(&locks[r][c]);
        grid[r][c]->push_back(i);
        omp_unset_lock(&locks[r][c]);
    }


    // Pack all the data into a texture
    // Populate grid part
    for (int r = 0; r < N_ROWS; r++) {
        for (int c = 0; c < N_COLS; c++) {
            std::vector<int> *cell = grid[r][c];
            int n_boids = cell->size();
            (*grid_cell_sizes)[r * N_COLS + c] = n_boids;

            // insert all the boids
            for (int i = 0; i < n_boids; i++) {
                //position_velocity.push_back((*cell)[i]);
                Boid *b = boids[(*cell)[i]];
                //inputs[b->index] = position_velocity.size() / 4;
                inputs[position_velocity.size() / 4] = b->index;
                position_velocity.push_back(b->position.x);
                position_velocity.push_back(b->position.y);
                position_velocity.push_back(b->velocity.x);
                position_velocity.push_back(b->velocity.y);
            }
        }
    }


    // Perform exclusive scan to get the start indices of each grid cell
    indices[0] = 0;
    for (int i = 1; i < grid_cell_sizes->size(); i++) {
        indices[i] = indices[i-1] + (*grid_cell_sizes)[i-1];
    }

    assert(position_velocity.size() % 4 == 0);
    


    for (int i = 0; i < boids.size(); i++) {
        int boid_index = inputs[i];
        Boid *b = boids[boid_index];
        b->Update(glm::vec2(forces[2*i], forces[2*i+1]), dt);
    }


}
void State::Render() {
    Renderer->DrawBoids(boids);
}

State::~State() {
    delete Renderer;
}
