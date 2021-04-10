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

#define SIGN(x) ((x) < 0.0f ? -1.0f : 1.0f)


std::uniform_real_distribution<float> randDir(0.0f, 2 * glm::pi<float>());


EntityRenderer *Renderer;
std::default_random_engine generator(10);
std::uniform_real_distribution<float> distribution(-100.0,100.0);

static std::map<int, std::map<int, std::map<long, Boid *>>> boidMap;

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

    qt = new QuadTreeHead(glm::vec2(0.0f, Width), glm::vec2(0.0f, Height));
}


void State::Update(GLfloat dt) {
    glm::vec2 forces[NUM_BOIDS];

    /*
    if (qt != NULL) {
        delete qt;
    }
    qt = new QuadTreeHead(glm::vec2(0.0f, Width), glm::vec2(0.0f, Height));
    */


    for (Boid *b : boids) {
        qt->insert(b);
    }

    #pragma omp parallel for
    for (size_t i = 0; i < boids.size(); i++) {
        Boid *b = boids[i];
        glm::vec2 forceCollision(0.0f, 0.0f);
        glm::vec2 forceAlign(0.0f);
        glm::vec2 forcePos(0.0f);
        glm::vec2 force(0.0f);
        glm::vec2 forceRand(0.0f);

        glm::vec2 flockCenter(0.0, 0.0);
        glm::vec2 flockHeading(0.0, 0.0);
        int numClose = 0;
        glm::vec3 mincolor = b->natural_color;

        std::function<void(Boid*)> lambda = [&](Boid *other) {
            if (other->index == b->index) {
                return;
            }
            // Collision avoidance
            float dist = glm::distance(other->position, b->position);
            // dir is already normalized, so dont need to take norm
            if (dist < nearby_dist) {
                if (other->index < b->index) {
                    mincolor = other->color;
                }

                flockCenter += other->position;
                flockHeading += other->velocity;
                numClose += 1;
                float scaling = (1.0f / (dist * dist));
                glm::vec2 dir = glm::normalize(b->position - other->position);
                forceCollision += dir * scaling;
            }
        };

        qt->query(b, lambda);
    
        if (numClose > 0) {
            forceAlign = flockHeading;
            forceAlign /= numClose;

            flockCenter /= numClose;
            forcePos = (flockCenter - b->position);

            forceCollision = b->SteerToward(forceCollision);
            forceCollision *= collision_weight;

            forceAlign = b->SteerToward(forceAlign);
            forceAlign *= align_weight;

            forcePos = b->SteerToward(forcePos);
            forcePos *= position_weight;

            force = forceCollision + forceAlign + forcePos;

        }
        b->color = .4f * b->color + (.6f) * mincolor;


        float theta = randDir(generator);
        forceRand = glm::vec2(glm::sin(theta), glm::cos(theta));
        force += forceRand;

        forces[b->index] = force;
    }
    

    for (Boid *b : this->boids) {
        b->Update(forces[b->index], dt);
    }

}
void State::Render() {
    Renderer->DrawBoids(boids);
    if (qt && VISUALIZE) {
        qt->visualize();
    }
    qt->clear();
}

State::~State() {
    delete Renderer;
}
