//
// Created by Alex Schneidman on 2020-05-09.
//

#include "EntityRenderer.h"
#include <iostream>
#include <omp.h>

#define SIDE_POS ((float)(sqrt(3.0f) / 4.0f))

void EntityRenderer::initRenderData() {
    model_matrices = new glm::mat4[NUM_BOIDS];
    color_matrices = new glm::vec3[NUM_BOIDS];



    // configure VAO/VBO
    unsigned int VBO;
    float vertices[] = {
            -2.0f*SIDE_POS, 0.5f, 0.0f,
            0.0f, -2.0f, 0.0f,
            2.0f*SIDE_POS, 0.5f, 0.0f
    };

    glGenVertexArrays(1, &this->VAO);
    glGenBuffers(1, &VBO);

    // Load in the triangle model
    glBindVertexArray(this->VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);


    // Now load in all the model matrices
    glGenBuffers(1, &model_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, model_buffer);
    glBufferData(GL_ARRAY_BUFFER, NUM_BOIDS * sizeof(glm::mat4), &model_matrices[0], GL_STATIC_DRAW);

    std::size_t vec4Size = sizeof(glm::vec4);
    glEnableVertexAttribArray(2); 
    glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, 4 * vec4Size, (void*)0);
    glEnableVertexAttribArray(3); 
    glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, 4 * vec4Size, (void*)(1 * vec4Size));
    glEnableVertexAttribArray(4); 
    glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, 4 * vec4Size, (void*)(2 * vec4Size));
    glEnableVertexAttribArray(5); 
    glVertexAttribPointer(5, 4, GL_FLOAT, GL_FALSE, 4 * vec4Size, (void*)(3 * vec4Size));

    glVertexAttribDivisor(2, 1);
    glVertexAttribDivisor(3, 1);
    glVertexAttribDivisor(4, 1);
    glVertexAttribDivisor(5, 1);

    glBindBuffer(GL_ARRAY_BUFFER, 0);

    // Noow load in the color matrices
    glGenBuffers(1, &color_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, color_buffer);
    glBufferData(GL_ARRAY_BUFFER, NUM_BOIDS * sizeof(glm::vec3), &color_matrices[0], GL_STATIC_DRAW);

    glEnableVertexAttribArray(6); 
    glVertexAttribPointer(6, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);

    glVertexAttribDivisor(6, 1);

    glBindBuffer(GL_ARRAY_BUFFER, 0);


    glBindVertexArray(0);
}
void EntityRenderer::DrawBoids(std::vector<Boid *> &boids) {
    this->shader.Use();

    
    glBindVertexArray(this->VAO);
    //#pragma omp parallel for
    for (size_t i = 0; i < boids.size(); i++) {
        Boid *b = boids[i];
        glm::mat4 model = glm::mat4(1.0f);

        model = glm::translate(model, glm::vec3(b->position, 0.0f));

        model = glm::translate(model, glm::vec3(0.0f, 0.75f, 0.0f)) * ER_DEFAULT_SIZE;
        model = glm::rotate(model, b->rotation, glm::vec3(0.0f, 0.0f, 1.0f));
        model = glm::translate(model, glm::vec3(0.0f, -0.75f, 0.0f)) * ER_DEFAULT_SIZE;

        model = glm::scale(model, glm::vec3(ER_DEFAULT_SIZE, ER_DEFAULT_SIZE, 1.0f));

        model_matrices[i] = model;
        color_matrices[i] = b->color;
    }
    
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ARRAY_BUFFER, model_buffer);
    glBufferData(GL_ARRAY_BUFFER, boids.size() * sizeof(glm::mat4), &model_matrices[0], GL_STATIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ARRAY_BUFFER, color_buffer);
    glBufferData(GL_ARRAY_BUFFER, boids.size() * sizeof(glm::vec3), &color_matrices[0], GL_STATIC_DRAW);

    glDrawArraysInstanced(GL_TRIANGLES, 0, 3, boids.size());

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

void EntityRenderer::Draw(glm::vec2 &pos, glm::vec2 size, float rotate, glm::vec3 color) {
    this->shader.Use();
    glm::mat4 model = glm::mat4(1.0f);

    model = glm::translate(model, glm::vec3(pos, 0.0f));

    model = glm::translate(model, glm::vec3(0.0f, 0.75f, 0.0f)) * size.y;
    model = glm::rotate(model, rotate, glm::vec3(0.0f, 0.0f, 1.0f));
    model = glm::translate(model, glm::vec3(0.0f, -0.75f, 0.0f)) * size.y;

    model = glm::scale(model, glm::vec3(size, 1.0f));

    this->shader.SetMatrix4("model", model);
    this->shader.SetVector3f("inColor", color);

    glBindVertexArray(this->VAO);
    glDrawArrays(GL_TRIANGLES, 0, 3);
    glBindVertexArray(0);
}

EntityRenderer::EntityRenderer(Shader &shader) {
    this->shader = shader;
    this->initRenderData();
}

EntityRenderer::~EntityRenderer() {
    glDeleteVertexArrays(1, &this->VAO);
}

void EntityRenderer::Draw(glm::vec2 &pos, float rotate) {
    this->Draw(pos,
            glm::vec2(ER_DEFAULT_SIZE, ER_DEFAULT_SIZE),
            rotate, glm::vec3(1.0f, 0.0f, 0.0f));
}

void EntityRenderer::Draw(glm::vec2 &pos, float rotate, glm::vec3 color) {
    this->Draw(pos, glm::vec2(ER_DEFAULT_SIZE, ER_DEFAULT_SIZE), rotate, color);
}
