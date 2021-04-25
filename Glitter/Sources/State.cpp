//
// Created by Alex Schneidman on 2020-05-09.
//

#include "State.h"
#include "resourcemanager.h"
#include <random>
#include <map>
#include <iterator>

#include <iostream>
#include <sstream>
#include <fstream>

#include <omp.h>
#include <glm/gtx/color_space.hpp>
#include <GLFW/glfw3.h>

#define SIGN(x) ((x) < 0.0f ? -1.0f : 1.0f)

int do_update = 0;
int update_frames = 1;

std::uniform_real_distribution<float> randDir(0.0f, 2 * glm::pi<float>());


EntityRenderer *Renderer;
std::default_random_engine generator(10);
std::uniform_real_distribution<float> distribution(-100.0,100.0);

State::State(GLuint width, GLuint height) : Width(width), Height(height) {}

void State::Init() {
    // load shaders
    ResourceManager::LoadShader("Glitter/entity.vert", "Glitter/entity.frag", nullptr, "entity");
    // configure shaders
    glm::mat4 projection = glm::ortho(0.0f, static_cast<float>(this->Width),
                                      static_cast<float>(this->Height), 0.0f, -1.0f, 1.0f);
    ResourceManager::GetShader("entity").Use().SetMatrix4("projection", projection);
    // set render-specific controls
    Shader sh = ResourceManager::GetShader("entity");
    Renderer = new EntityRenderer(sh);

    std::uniform_real_distribution<float> posDistX(0.0f, this->Width-1);
    std::uniform_real_distribution<float> posDistY(0.0f, this->Height-1);


    std::uniform_real_distribution<float> randSpeed(0.0f, BOID_SPEED);


    for (int i = 0; i < NUM_BOIDS; i++) {
        glm::vec2 initPos = glm::vec2(posDistX(generator), posDistY(generator));
        float theta = randDir(generator);
        glm::vec2 initVel = glm::vec2(glm::sin(theta), glm::cos(theta));
    
        glm::vec2 initialVel = initVel * randSpeed(generator);
        this->boids.push_back(new Boid(initPos, initialVel, this->Width, this->Height, i));
    }


    // CODE TO LOAD AND COMPILE THE FORCE SHADER
    std::string forceShaderCode;
    std::ifstream forceShaderFile("Glitter/boid_force.vert");

    std::stringstream forceShaderStream;
    forceShaderStream << "#version 330 core\n#define NUM_BOIDS " << NUM_BOIDS << "\n" << forceShaderFile.rdbuf();

    forceShaderFile.close();

    // Create shader program
    const char *forceShaderStr = forceShaderStream.str().c_str();
    forceShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(forceShader, 1, &forceShaderStr, nullptr);
    glCompileShader(forceShader);

    sh.checkCompileErrors(forceShader, "VERTEX");

    forceProgram = glCreateProgram();
    glAttachShader(forceProgram, forceShader);

    const GLchar* feedbackNames[2] = {"force_x", "force_y"};
    glTransformFeedbackVaryings(forceProgram, 2, feedbackNames, GL_SEPARATE_ATTRIBS);
    glLinkProgram(forceProgram);

    glUseProgram(forceProgram);

    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);


    // Generate the buffers
    glGenBuffers(1, &force_x_buffer);
    glGenBuffers(1, &force_y_buffer);

    // Bind them
    glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0, force_x_buffer);
    glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 1, force_y_buffer);

    glBindVertexArray(0);

    positions = new GLfloat[2 * NUM_BOIDS];
    velocities = new GLfloat[2 * NUM_BOIDS];
    forces_x = new GLfloat[NUM_BOIDS];
    forces_y = new GLfloat[NUM_BOIDS];
}



void State::Update(GLfloat dt) {
    glUseProgram(forceProgram);

    #pragma omp parallel for schedule(static) 
    for (int i = 0; i < boids.size(); i++) {
        Boid *b = boids[i];
        positions[2*i] = b->position.x;
        positions[2*i+1] = b->position.y;
        velocities[2*i] = b->velocity.x;
        velocities[2*i+1] = b->velocity.y;
    }




    glBindVertexArray(vao);

    glUniform2fv(glGetUniformLocation(forceProgram, "positions"), NUM_BOIDS, positions);
    glUniform2fv(glGetUniformLocation(forceProgram, "velocities"), NUM_BOIDS, velocities);
    glUniform1f(glGetUniformLocation(forceProgram, "nearby_dist"), nearby_dist);
    glUniform1f(glGetUniformLocation(forceProgram, "max_velocity"), max_velocity);
    glUniform1f(glGetUniformLocation(forceProgram, "max_force"), (float)MAX_FORCE);


    // Allocate space for the results x
    glBindBuffer(GL_ARRAY_BUFFER, force_x_buffer);
    glBufferData(GL_ARRAY_BUFFER, NUM_BOIDS * sizeof(GLfloat), nullptr, GL_STATIC_READ);
    // Allocate space for the results y
    glBindBuffer(GL_ARRAY_BUFFER, force_y_buffer);
    glBufferData(GL_ARRAY_BUFFER, NUM_BOIDS * sizeof(GLfloat), nullptr, GL_STATIC_READ);

    // Tell opengl where to read/write the data
    glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0, force_x_buffer);
    glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 1, force_y_buffer);


    // Actually perform the computation
    glEnable(GL_RASTERIZER_DISCARD);

    glBeginTransformFeedback(GL_POINTS);
        glDrawArrays(GL_POINTS, 0, NUM_BOIDS);
    glEndTransformFeedback();

    glDisable(GL_RASTERIZER_DISCARD);

    glFlush();
    // Bind buffer to read from
    glBindBuffer(GL_ARRAY_BUFFER, force_x_buffer);
    glGetBufferSubData(GL_ARRAY_BUFFER, 0, NUM_BOIDS * sizeof(GLfloat), forces_x);

    glBindBuffer(GL_ARRAY_BUFFER, force_y_buffer);
    glGetBufferSubData(GL_ARRAY_BUFFER, 0, NUM_BOIDS * sizeof(GLfloat), forces_y);

    glBindVertexArray(0);

    for (int i = 0; i < boids.size(); i++) {
        Boid *b = boids[i];
        b->Update(glm::vec2(forces_x[i], forces_y[i]), dt);
    }
}
void State::Render() {
    Renderer->DrawBoids(boids);
}

State::~State() {
    delete Renderer;
}
