//
// Created by Alex Schneidman on 2020-05-09.
//

#include "Boid.h"

#define WRAP_VALUES(val, bound) (((val) >= (bound)) ? 0.0f : ((val) < 0.0f ? (bound - 1.0f) : val))

static glm::vec2 yaxis = glm::vec2(0.0f, 1.0f);

void Boid::Draw(EntityRenderer *renderer) {
    renderer->Draw(this->position, this->rotation);
}

void Boid::Update(glm::vec2 force, float dt) {
    this->velocity += force * dt;
    glm::vec2 change = velocity * dt;
    this->position += change;
    // Now calculate direction of movement
    glm::vec2 dir = glm::normalize(this->velocity);
    dir = glm::vec2(dir.x, -dir.y);
    if (dir.x < 0.0f) {
        this->rotation = -acos(glm::dot(yaxis, dir));
    } else {
        this->rotation = acos(glm::dot(yaxis, dir));
    }
    this->position = glm::vec2(WRAP_VALUES(this->position.x, this->width),
                                WRAP_VALUES(this->position.y, this->height));
}

float Boid::GetX() {
    return this->position.x;
}

float Boid::GetY() {
    return this->position.y;
}

