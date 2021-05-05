//
// Created by Alex Schneidman on 2020-05-09.
//

#include "State.h"
#include "resourcemanager.h"
#include "QuadTree.hpp"
#include <random>
#include <map>
#include <iterator>
#include <iostream>
#include <omp.h>
#include <glm/gtx/color_space.hpp>
#include <GLFW/glfw3.h>
#include "ispc_query.h"

#define SIGN(x) ((x) < 0.0f ? -1.0f : 1.0f)

using namespace ispc;

/*
extern void compute_forces(int num_boids, 
                      float radius,
                        float out_forces[], 
                        float positions_velocities[],
                        int positions_velocity_indices[],
                        Node nodes[],
                        float bounds[],
                        float max_velocity,
                        float max_force,
                        float collision_weight,
                        float align_weight,
                        float position_weight);
*/

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

    qt = new QuadTreeHead(glm::vec2(0.0f, Width), glm::vec2(0.0f, Height), &boids);


    forces = new float[NUM_BOIDS * 2];
    positions_velocities = new float[NUM_BOIDS * 4];
    position_velocity_indices = new int[NUM_BOIDS];

    for (int i = 0; i < NUM_BOIDS; i++) {
        reordered_boids[i] = i;
    }
}


void State::Update(GLfloat dt) {
    //glm::vec2 forces[NUM_BOIDS];

    if (do_update % update_frames == 0) {
        float s = glfwGetTime();
        qt->clear();
        float e = glfwGetTime();
        //printf("clear time: %.3f\n", e-s);
        s = glfwGetTime();
        for (size_t i = 0; i < NUM_BOIDS; i++) {
            int boid_index = reordered_boids[i];
            Boid *b = boids[boid_index];
            qt->insert(b);
        }

        //e = glfwGetTime();
        //printf("update time: %.3f\n", e-s);
        do_update = 0;
    }
    do_update += 1;

    //float s = glfwGetTime();
    // Build datastructures for ispc
    Node *nodes = new Node[qt->nodes.size()];
    
    int p_v_index = 0;
    for (int i = 0; i < qt->nodes.size(); i++) {
        QuadTree_t *node = qt->nodes[i];

        nodes[i].is_subdivided = node->is_subdivided;
        nodes[i].num_boids = node->num_boids;
        nodes[i].first_child = node->first_child;

        if (node->is_subdivided == false) {
            nodes[i].boids_start_index = p_v_index;

            int current_child = node->first_element;
            for (int j = 0; j < node->num_boids; j++) {
                QuadTreeElem_t *elem = qt->elements[current_child];
                Boid *b = boids[elem->boid];
                positions_velocities[4 * p_v_index] = b->position.x;
                positions_velocities[4 * p_v_index + 1] = b->position.y;
                positions_velocities[4 * p_v_index + 2] = b->velocity.x;
                positions_velocities[4 * p_v_index + 3] = b->velocity.y;

                position_velocity_indices[b->index] = p_v_index;

                reordered_boids[elem->boid] = p_v_index;

                p_v_index += 1;

                current_child = elem->next;
            }
        }
    }
    float bbox[4] = {qt->bounds_x.x, qt->bounds_x.y, qt->bounds_y.x, qt->bounds_y.y};
    
    //float e = glfwGetTime();
    //printf("ISPC data struct building time: %.3f\n", e-s);

    //s = glfwGetTime();
    #pragma omp parallel for schedule(dynamic, 250) num_threads(THREADS)
    for (int i = 0; i < qt->nodes.size(); i++) {
        QuadTree_t *node = qt->nodes[i];
        if (node->is_subdivided == false) {
            int indices[node->num_boids];

            int current_child = node->first_element;
            for (int j = 0; j < node->num_boids; j++) {
                QuadTreeElem_t *elem = qt->elements[current_child];
                indices[j] = elem->boid;
    
                current_child = elem->next;
            }
            compute_forces(indices, node->num_boids, nearby_dist, forces, positions_velocities, position_velocity_indices, nodes, bbox, max_velocity, MAX_FORCE, collision_weight, align_weight, position_weight);

        }
    }

    delete[] nodes;

    //e = glfwGetTime();

    //printf("ISPC query time: %.3f\n", e-s);


    #pragma omp parallel for schedule(static) num_threads(THREADS)
    for (size_t i = 0; i < boids.size(); i++) {
        Boid *b = boids[i];
        b->Update(glm::vec2(forces[2 * b->index], forces[2 * b->index + 1]), dt);
    }

}
void State::Render() {
    Renderer->DrawBoids(boids);
    if (qt && VISUALIZE) {
        //qt->visualize();
    }
}

State::~State() {
    delete Renderer;
}
