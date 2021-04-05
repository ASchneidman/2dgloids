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

// The Width of the screen
const unsigned int SCREEN_WIDTH = 6000;
// The height of the screen
const unsigned int SCREEN_HEIGHT = 3000;

State state(SCREEN_WIDTH, SCREEN_HEIGHT);

int main(int argc, char * argv[]) {

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

    state.Init();

    // deltaTime variables
    // -------------------
    float deltaTime = 0.0f;
    float lastFrame = 0.0f;

    float averageFrameTime = 0.0f;
    float startTime = glfwGetTime();
    // Rendering Loop
    int i = 0;
    while (glfwWindowShouldClose(mWindow) == false && i < 30) {
        // calculate delta time
        // --------------------
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;
        averageFrameTime += deltaTime;

        if (glfwGetKey(mWindow, GLFW_KEY_ESCAPE) == GLFW_PRESS)
            glfwSetWindowShouldClose(mWindow, true);

        // Background Fill Color
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        float c = glfwGetTime();
        state.Update(deltaTime);
        printf("Time to run frame %d: %.3f\n", i, glfwGetTime() - c);
        state.Render();

        // Flip Buffers and Draw
        glfwSwapBuffers(mWindow);
        glfwPollEvents();
        i += 1;
    }   
    printf("Total time: %.3f\n", i, glfwGetTime() - startTime);
    printf("Average frame time: %.3f\n", averageFrameTime / 30);

    glfwTerminate();

    return EXIT_SUCCESS;
}
