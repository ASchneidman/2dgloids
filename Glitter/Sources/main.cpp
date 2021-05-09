// Local Headers
#include "glitter.hpp"
#include "State.h"

// System Headers
#include <glad/glad.h>
#include <GLFW/glfw3.h>

// Standard Headers
#include <cstdio>
#include <cstdlib>
#include <iostream>

float max_velocity = MAX_VEL;
float nearby_dist = NEARBY_DIST;

float collision_weight = COLLISION_WEIGHT;
float align_weight = ALIGN_WEIGHT;
float position_weight = POSITION_WEIGHT;

State state(SCREEN_WIDTH, SCREEN_HEIGHT);

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);

int main(int argc, char * argv[]) {
    (void)argc; (void)argv;

    // Load GLFW and Create a Window
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);
    auto mWindow = glfwCreateWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "OpenGL", nullptr, nullptr);

    // Check for Valid Context
    if (mWindow == nullptr) {
        fprintf(stderr, "Failed to Create OpenGL Context");
        return EXIT_FAILURE;
    }

    // Create Context and Load OpenGL Functions
    glfwMakeContextCurrent(mWindow);
    gladLoadGL();
    fprintf(stderr, "OpenGL %s\n", glGetString(GL_VERSION));

    glfwSetKeyCallback(mWindow, key_callback);

    state.Init();

    // deltaTime variables
    // -------------------
    float deltaTime = 0.0f;
    float lastFrame = 0.0f;

    float averageFrameTime = 0.0f;
    float startTime = glfwGetTime();
    // Rendering Loop
    int i = 0;
    while (glfwWindowShouldClose(mWindow) == false && i < 120) {
        // calculate delta time
        // --------------------
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        if (glfwGetKey(mWindow, GLFW_KEY_ESCAPE) == GLFW_PRESS)
            glfwSetWindowShouldClose(mWindow, true);

        // Background Fill Color
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        float update_start = glfwGetTime();
        state.Update(.01);
        float update_end = glfwGetTime();
        float render_start = glfwGetTime();
        state.Render();
        float render_end = glfwGetTime();

        printf("\rUpdate time: %.3f, Render time: %.3f, Max Velocity: %.3f, View Distance: %.3f, Collision Weight: %.3f, Align Weight: %.3f, Position Weight: %.3f", 
        update_end - update_start, render_end - render_start, max_velocity, nearby_dist,
        collision_weight, align_weight, position_weight);

        averageFrameTime += (update_end - update_start);

        // Flip Buffers and Draw
        glfwSwapBuffers(mWindow);
        glfwPollEvents();
        i += 1;
    }   
    std::cout << std::endl;

    printf("Average update time: %.3f\n", averageFrameTime / i);

    glfwTerminate();

    return EXIT_SUCCESS;
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    (void)scancode; (void)action; (void)mods; (void)window;
    if (action == GLFW_PRESS || action == GLFW_REPEAT) {
        switch (key)
        {
        case GLFW_KEY_W:
            max_velocity += 100.0f;
            break;
        case GLFW_KEY_S:
            max_velocity -= 100.0f;
            break;
        case GLFW_KEY_UP:
            nearby_dist += 10.0f;
            break;
        case GLFW_KEY_DOWN:
            nearby_dist -= 10.0f;
            break;
        case GLFW_KEY_1:
            collision_weight += .1f;
            break;
        case GLFW_KEY_2:
            collision_weight -= .1f;
            break;
        case GLFW_KEY_3:
            align_weight += .1f;
            break;
        case GLFW_KEY_4:
            align_weight -= .1f;
            break;
        case GLFW_KEY_5:
            position_weight += .1f;
            break;
        case GLFW_KEY_6:
            position_weight -= .1f;
            break;
        default:
            break;
        }
    }
    max_velocity = std::max(max_velocity, 1.0f);
    nearby_dist = std::max(nearby_dist, 1.0f);
    collision_weight = std::max(collision_weight, 0.0f);
    align_weight = std::max(align_weight, 0.0f);
    position_weight = std::max(position_weight, 0.0f);
}
