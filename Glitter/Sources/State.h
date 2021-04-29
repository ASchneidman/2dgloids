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


    GLuint UBO;

    GLuint vao;

    GLuint tbo;
    GLuint texture_buffer;

    //GLfloat *position_velocity;
    std::vector<GLfloat> position_velocity;
    GLfloat *forces;
    GLint *grid_cell_sizes;

    //std::vector<int> grid[N_ROWS * N_COLS];
    std::vector<int> *grid[N_ROWS][N_COLS];
};

#endif //GLITTER_STATE_H
