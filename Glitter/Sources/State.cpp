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
    forceShaderStream << "#version 330 core\n#define NUM_BOIDS " << NUM_BOIDS << "\n" \
                      << "#define screen_width " << SCREEN_WIDTH << "\n" \
                      << "#define screen_height " << SCREEN_HEIGHT << "\n" \
                      << "#define n_rows " << N_ROWS << "\n" \
                      << "#define n_cols " << N_COLS << "\n" \
                      <<  forceShaderFile.rdbuf();

    forceShaderFile.close();

    // Create shader program
    auto cppstr = forceShaderStream.str();
    const char *forceShaderStr = cppstr.c_str();
    forceShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(forceShader, 1, &forceShaderStr, nullptr);
    glCompileShader(forceShader);

    sh.checkCompileErrors(forceShader, "VERTEX");

    forceProgram = glCreateProgram();
    glAttachShader(forceProgram, forceShader);

    //const GLchar* feedbackNames[2] = {"force_x", "force_y"};
    const GLchar *feedbackNames[1] = {"total_force"};
    glTransformFeedbackVaryings(forceProgram , 1, feedbackNames, GL_INTERLEAVED_ATTRIBS);
    glLinkProgram(forceProgram);

    glUseProgram(forceProgram);

    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    // Generate the buffers
    glGenBuffers(1, &force_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, force_buffer);
    glBufferData(GL_ARRAY_BUFFER, NUM_BOIDS * sizeof(GLfloat) * 2, nullptr, GL_STATIC_READ);


    // Create the texture buffer
    glGenBuffers(1, &tbo);
    glBindBuffer(GL_TEXTURE_BUFFER, tbo);


    // Generate texture and associate it with the tbo
    glGenTextures(1, &texture_buffer);
    glBindTexture(GL_TEXTURE_BUFFER, texture_buffer);
    glTexBuffer(GL_TEXTURE_BUFFER, GL_RGBA32F, tbo);
    glActiveTexture(GL_TEXTURE0);
    glBindBuffer(GL_TEXTURE_BUFFER, 0);


    // Bind them
    glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0, force_buffer);




    // Set the binding stuff for the UBO
    GLuint buffer_index = glGetUniformBlockIndex(forceProgram, "sizes");   
    glUniformBlockBinding(forceProgram, buffer_index, 1);

    glGenBuffers(1, &UBO);

    // Allocate storage for ubo
    glBindBuffer(GL_UNIFORM_BUFFER, UBO);
    // Size four since we have both position and velocity
    glBufferData(GL_UNIFORM_BUFFER, N_ROWS * N_COLS * sizeof(GLint), NULL, GL_STATIC_DRAW);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
    glBindBufferRange(GL_UNIFORM_BUFFER, 1, UBO, 0, N_ROWS * N_COLS * sizeof(GLint));





    glBindVertexArray(0);


    forces = new GLfloat[NUM_BOIDS * 2];

    grid_cell_sizes = new GLint[N_ROWS * N_COLS];

    for (int r = 0; r < N_ROWS; r++) {
        for (int c = 0; c < N_COLS; c++) {
            grid[r][c] = new std::vector<int>();
        }
    }
}



void State::Update(GLfloat dt) {
    position_velocity.clear();
    // clear grid
    for (int r = 0; r < N_ROWS; r++) {
        for (int c = 0; c < N_COLS; c++) {
            grid[r][c]->clear();
        }
    }


    // Build grid
    for (int i = 0; i < boids.size(); i++) {
        Boid *b = boids[i];
        float percent_x = b->position.x / SCREEN_WIDTH;
        float percent_y = b->position.y / SCREEN_HEIGHT;
        
        int r = percent_x * N_ROWS;
        int c = percent_y * N_COLS;

        grid[r][c]->push_back(i);
    }


    // Pack all the data into a texture
    // Populate grid part
    for (int r = 0; r < N_ROWS; r++) {
        for (int c = 0; c < N_COLS; c++) {
            std::vector<int> *cell = grid[r][c];
            int n_boids = cell->size();
            grid_cell_sizes[(r * N_COLS + c)] = n_boids;


            // insert all the boids
            for (int i = 0; i < n_boids; i++) {
                position_velocity.push_back((*cell)[i]);
            }



            // round up to next multiple of 4
            if (position_velocity.size() % 4 != 0) {
                position_velocity.insert(position_velocity.end(), 4 - (position_velocity.size() % 4), 0);
            }
        }
    }

    int offset = position_velocity.size();
    assert(position_velocity.size() % 4 == 0);

    for (int i = 0; i < boids.size(); i++) {
        Boid *b = boids[i];
        position_velocity.push_back(b->position.x);
        position_velocity.push_back(b->position.y);
        position_velocity.push_back(b->velocity.x);
        position_velocity.push_back(b->velocity.y);
    }

    glUseProgram(forceProgram);
    glBindVertexArray(vao);

    glUniform1i(glGetUniformLocation(forceProgram, "position_velocity_offset"), offset / 4);

    glUniform1f(glGetUniformLocation(forceProgram, "nearby_dist"), nearby_dist);
    glUniform1f(glGetUniformLocation(forceProgram, "max_velocity"), max_velocity);
    glUniform1f(glGetUniformLocation(forceProgram, "max_force"), (float)MAX_FORCE);

    glUniform1f(glGetUniformLocation(forceProgram, "collision_weight"), collision_weight);
    glUniform1f(glGetUniformLocation(forceProgram, "align_weight"), align_weight);
    glUniform1f(glGetUniformLocation(forceProgram, "position_weight"), position_weight);


    glBindBuffer(GL_UNIFORM_BUFFER, UBO);
    glBufferSubData(GL_UNIFORM_BUFFER, 0, N_ROWS * N_COLS * sizeof(GLint), grid_cell_sizes);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);


    // bind texture
    glBindBuffer(GL_TEXTURE_BUFFER, tbo);
    // Send data
    glBufferData(GL_TEXTURE_BUFFER, position_velocity.size() * sizeof(GLfloat), position_velocity.data(), GL_STATIC_DRAW);
    glBindBuffer(GL_TEXTURE_BUFFER, 0);


    // Actually perform the computation
    glEnable(GL_RASTERIZER_DISCARD);

    glBeginTransformFeedback(GL_POINTS);
        glDrawArraysInstanced(GL_POINTS, 0, 1, NUM_BOIDS);
    glEndTransformFeedback();

    glDisable(GL_RASTERIZER_DISCARD);

    glFlush();
    // Bind buffer to read from
    glBindBuffer(GL_ARRAY_BUFFER, force_buffer);
    glGetBufferSubData(GL_ARRAY_BUFFER, 0, NUM_BOIDS * sizeof(GLfloat) * 2, forces);

    glBindVertexArray(0);

    for (int i = 0; i < boids.size(); i++) {
        Boid *b = boids[i];
        b->Update(glm::vec2(forces[2*i], forces[2*i+1]), dt);
    }



}
void State::Render() {
    Renderer->DrawBoids(boids);
}

State::~State() {
    delete Renderer;
}
