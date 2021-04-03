//
// Created by Alex Schneidman on 2020-05-09.
//

#ifndef GLITTER_BOID_H
#define GLITTER_BOID_H


#include <glm/vec2.hpp>
#include "EntityRenderer.h"

#define MAX_VELOCITY 700.0f
#define MAX_FORCE 600.0f

class Boid {
private:
public:
    Boid(glm::vec2 &initialPos, glm::vec2 &initialVel,
            float width, float height): width(width), height(height) {
        this->position = initialPos;
        velocity = initialVel;
        this->rotation = M_PI;
        this->color = glm::vec3((rand() % 10) / 9.0f, (rand() % 10) / 9.0f, (rand() % 10) / 9.0f);
    }
    ~Boid() = default;
    void Draw(EntityRenderer *renderer);
    void Update(glm::vec2 force, float dt);
    float GetX(); float GetY();
    glm::vec2 SteerToward(glm::vec2 force);
    glm::vec2 GetVelocity();
    // Each iteration, a random force is chosen. This is
    // added to the velocity, which is then added to position
    // (scaled by dt)
    glm::vec2 position, velocity;
    // Inferred from movement, about z-axis
    float rotation;
    float width, height;
    glm::vec3 color;
};


#endif //GLITTER_BOID_H
