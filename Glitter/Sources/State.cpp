//
// Created by Alex Schneidman on 2020-05-09.
//

#include "State.h"
#include "resourcemanager.h"
#include <random>

EntityRenderer *Renderer;
std::default_random_engine generator;
std::uniform_real_distribution<float> distribution(-100.0,100.0);

#define NUM_BOIDS 10

State::State(GLuint width, GLuint height) : Width(width), Height(height) {}

void State::Init() {
    // load shaders
    ResourceManager::LoadShader("entity.vert", "entity.frag", nullptr, "entity");
    // configure shaders
    glm::mat4 projection = glm::ortho(0.0f, static_cast<float>(this->Width),
                                      static_cast<float>(this->Height), 0.0f, -1.0f, 1.0f);
    ResourceManager::GetShader("entity").Use().SetMatrix4("projection", projection);
    // set render-specific controls
    Shader sh = ResourceManager::GetShader("entity");
    Renderer = new EntityRenderer(sh);

    std::uniform_real_distribution<float> posDistX(0.0f, this->Width-1);
    std::uniform_real_distribution<float> posDistY(0.0f, this->Height-1);

    std::uniform_real_distribution<float> randDir(-1.0f, 1.0f);

    for (int i = 0; i < NUM_BOIDS; i++) {
        glm::vec2 initPos = glm::vec2(posDistX(generator), posDistY(generator));
        //glm::vec2 initPos = glm::vec2(100.0f, 100.0f);
        glm::vec2 initialVel = glm::vec2(randDir(generator), randDir(generator)) * 100.0f;
        this->boids.push_back(new Boid(initPos, initialVel, this->Width, this->Height));
    }
}

void State::Update(GLfloat dt) {
    for (Boid *b : this->boids) {
        //glm::vec2 force = glm::vec2(distribution(generator), distribution(generator));
        b->Update(glm::vec2(0.0f, 0.0f), dt);
    }

}
float last = M_PI;
void State::Render() {
    for (Boid *b : this->boids) {
        b->Draw(Renderer);
    }
}

State::~State() {
    delete Renderer;
}
