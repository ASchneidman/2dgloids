//
// Created by Alex Schneidman on 2020-05-09.
//

#include "Boid.h"
#include <math.h>
#include <glm/gtx/color_space.hpp>

static glm::vec2 yaxis = glm::vec2(0.0f, 1.0f);

float wrap_value(float value, float bound) {
    float m = std::fmod(value, bound);
    if (m < 0) {
        m += bound;
    }
    return m;
}

glm::vec2 clamp_magnitude(glm::vec2 vec, float max_value) {
    if (glm::length(vec) > max_value) {
        glm::vec2 v = glm::normalize(vec);
        v *= max_value;
        return v;
    }
    return vec;
}

glm::vec2 Boid::SteerToward(glm::vec2 force) {
    return clamp_magnitude(glm::normalize(force) * max_velocity - velocity, MAX_FORCE); 
}

void Boid::Update(glm::vec2 force, float dt) {
    this->velocity += force * dt;
    this->velocity = clamp_magnitude(this->velocity, max_velocity);
    
    glm::vec2 change = this->velocity * dt;

    if (change.x + position.x < 0 || change.x + position.x >= this->width) {
        velocity *= glm::vec2(-1.0f, 1.0f);
        change = velocity * dt;
    }
    if (change.y + position.y < 0 || change.y + position.y >= this->height) {
        velocity *= glm::vec2(1.0f, -1.0f);
        change = velocity * dt;
    }

    this->position += change;

    // Now calculate direction of movement
    glm::vec2 dir = glm::normalize(this->velocity);
    dir = glm::vec2(dir.x, -dir.y);
    
    if (dir.x < 0.0f) {
        this->rotation = -acos(glm::dot(yaxis, dir));
    } else {
        this->rotation = acos(glm::dot(yaxis, dir));
    }
    this->position = glm::vec2(wrap_value(this->position.x, this->width),
                                wrap_value(this->position.y, this->height));

    color = glm::rgbColor(glm::vec3((rotation + glm::pi<float>()) * 180.0f/glm::pi<float>(), .7, 1.0));
}

float Boid::GetX() {
    return this->position.x;
}

float Boid::GetY() {
    return this->position.y;
}

glm::vec2 Boid::GetVelocity() {
    return this->velocity;
}

