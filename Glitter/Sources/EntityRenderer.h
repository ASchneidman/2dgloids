//
// Created by Alex Schneidman on 2020-05-09.
//

#ifndef GLITTER_ENTITYRENDERER_H
#define GLITTER_ENTITYRENDERER_H


#include "shader.h"
#include "Boid.h"
#include "glitter.hpp"
#include <vector>

#define ER_DEFAULT_SIZE 10.0f
#define ER_DEFAULT_ROT 0.0f

class EntityRenderer {
public:
    EntityRenderer(Shader &shader);
    ~EntityRenderer();
    // Note: rotation in radians
    void Draw(glm::vec2 &pos,
              glm::vec2 size = glm::vec2(ER_DEFAULT_SIZE, ER_DEFAULT_SIZE),
              float rotate = ER_DEFAULT_ROT,
              glm::vec3 color = glm::vec3(1.0f, 0.0f, 0.0f));
    void Draw(glm::vec2 &pos,
              float rotate);
    void Draw(glm::vec2 &pos,
                float rotate,
                glm::vec3 color);
    void DrawBoids(std::vector<Boid *> &boids);
private:
    Shader shader;
    GLuint VAO;
    glm::mat4 *model_matrices;
    glm::vec3 *color_matrices;
    GLuint model_buffer;
    GLuint color_buffer;
    void initRenderData();
};


#endif //GLITTER_ENTITYRENDERER_H
