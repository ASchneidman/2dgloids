//
// Created by Alex Schneidman on 2020-05-09.
//

#include "Boid.h"

#define WRAP_VALUES(val, bound) (((val) >= (bound)) ? 0.0f : ((val) < 0.0f ? (bound - 1.0f) : val))

static glm::vec2 yaxis = glm::vec2(0.0f, 1.0f);

glm::vec2 clamp_magnitude(glm::vec2 vec, double max_value) {
    if (glm::length(vec) > max_value) {
        glm::vec2 v = glm::normalize(vec);
        v *= max_value;
        return v;
    }
    return vec;
}

glm::vec2 Boid::SteerToward(glm::vec2 force) {
    return clamp_magnitude(glm::normalize(force) * MAX_VELOCITY - velocity, MAX_FORCE); 
}

void Boid::Draw(EntityRenderer *renderer) {
    renderer->Draw(this->position, this->rotation, this->color);
}

void Boid::Update(glm::vec2 force, float dt) {
    this->velocity += force * dt;
    this->velocity = clamp_magnitude(this->velocity, MAX_VELOCITY);
    
    glm::vec2 change = this->velocity * dt;
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


    //glm::vec2 norm_pos = glm::normalize(position);
    //this->color.x = norm_pos.x;
    //this->color.y = norm_pos.y;

    this->color.x = 1;
    this->color.y = 1;
    this->color.z = 1;

    this->color *= glm::length(velocity) / MAX_VELOCITY;
    this->color *= glm::length(velocity) / MAX_VELOCITY;
                                
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

