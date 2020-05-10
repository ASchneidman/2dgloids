//
// Created by Alex Schneidman on 2020-05-09.
//

#include "EntityRenderer.h"

#define SIDE_POS ((float)(sqrt(3.0f) / 4.0f))

void EntityRenderer::initRenderData() {
    // configure VAO/VBO
    unsigned int VBO;
    float vertices[] = {
            -2.0f*SIDE_POS, 0.5f, 0.0f,
            0.0f, -2.0f, 0.0f,
            2.0f*SIDE_POS, 0.5f, 0.0f
    };

    glGenVertexArrays(1, &this->VAO);
    glGenBuffers(1, &VBO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glBindVertexArray(this->VAO);
    glEnableVertexAttribArray(0);
    //glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
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
    //glDrawArrays(GL_TRIANGLES, 0, 6);
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
