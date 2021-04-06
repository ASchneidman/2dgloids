//
// Created by Alex Schneidman on 2020-05-09.
//

#include "State.h"
#include "resourcemanager.h"
#include <random>
#include <map>
#include <iterator>
#include <iostream>
#include <omp.h>

#define NUM_BOIDS 100000
#define BOID_SPEED 10.0f

#define COLLISION_WEIGHT (1.3f)
#define ALIGN_WEIGHT (1.5f)
#define POSITION_WEIGHT (1.5f)

#define SIGN(x) ((x) < 0.0f ? -1.0f : 1.0f)

#define NEARBY_DIST 250.0f

#define LINE_OF_SIGHT (3.0 * glm::pi<float>() / 4.0)

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
        this->boids.push_back(new Boid(initPos, initialVel, this->Width, this->Height));
    }
}

void State::Update(GLfloat dt) {
    std::map<Boid *, glm::vec2> forces;
    for (Boid *b : this->boids) {
        glm::vec2 forceCollision(0.0f, 0.0f);
        glm::vec2 forceAlign;
        glm::vec2 forcePos;
        glm::vec2 force;
        glm::vec2 forceRand;

        glm::vec2 flockCenter(0.0, 0.0);
        glm::vec2 flockHeading(0.0, 0.0);

        int numClose = 0;

        for (int i = 0; i < this->boids.size(); i++) {
            Boid *other = this->boids[i];
            if (b == other)
                continue;

            // Collision avoidance
            glm::vec2 dir = glm::normalize(b->position - other->position);
            float dist = glm::distance(other->position, b->position);
            // dir is already normalized, so dont need to take norm
            if (dist < NEARBY_DIST) {
                flockCenter += other->position;
                flockHeading += glm::normalize(other->velocity);
                numClose += 1;
                float scaling = (1.0f / (dist * dist));
                forceCollision += dir * scaling;
            }
        }

        if (numClose > 0) {
            forceAlign = flockHeading;

            flockCenter /= numClose;
            forcePos = (flockCenter - b->position);

            forceCollision = b->SteerToward(forceCollision);
            forceCollision *= COLLISION_WEIGHT;

            forceAlign = b->SteerToward(forceAlign);
            forceAlign *= ALIGN_WEIGHT;

            forcePos = b->SteerToward(forcePos);
            forcePos *= POSITION_WEIGHT;

            force = forceCollision + forceAlign + forcePos;
        }

        forces[b] = force;
    }
    for (Boid *b : this->boids) {
        b->Update(forces[b], dt);
    }
}
void State::Render() {
    for (Boid *b : this->boids) {
        b->Draw(Renderer);
    }
}

State::~State() {
    delete Renderer;
}
