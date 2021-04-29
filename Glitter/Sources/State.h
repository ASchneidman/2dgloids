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

    GLuint force_buffer;
    //GLuint force_x_buffer;
    //GLuint force_y_buffer;


    GLuint vao;

    GLuint tbo;
    GLuint texture_buffer;

    //GLfloat *position_velocity;
    std::vector<GLfloat> position_velocity;
    GLfloat *forces;
    //GLfloat *forces_x;
    //GLfloat *forces_y;

    //std::vector<int> grid[N_ROWS * N_COLS];
    std::vector<int> *grid[N_ROWS][N_COLS];
};


#endif //GLITTER_STATE_H
