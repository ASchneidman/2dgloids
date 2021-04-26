//
// Created by Alex Schneidman on 2020-05-09.
//

#ifndef GLITTER_STATE_H
#define GLITTER_STATE_H


#include <glad/glad.h>
#include <vector>
#include "EntityRenderer.h"
#include "Boid.h"

class State {
public:
    State(GLuint width, GLuint height);
    ~State();
    void Init();
    void Update(GLfloat dt);
    void Render();
private:
    GLuint Width, Height;
    std::vector<Boid *> boids;

    GLuint forceShader;
    GLuint forceProgram;

    GLuint force_x_buffer;
    GLuint force_y_buffer;

    GLuint vao;

    GLuint UBO;

    GLfloat *positions;
    GLfloat *velocities;
    GLfloat *forces_x;
    GLfloat *forces_y;
};


#endif //GLITTER_STATE_H
