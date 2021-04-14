#include "GridBin.hpp"
#include <iostream>

const static char *lineVertShader = R"(#version 330
layout (location = 0) in vec3 aPos;

void main() {
    gl_Position = vec4(aPos, 1.0f);
})";

const static char *lineFragShader = R"(#version 330
out vec4 out_color;

void main() {
    out_color = vec4(1.0, 1.0, 1.0, 1.0);
}

)";

void checkCompileErrors(unsigned int object, std::string type)
{
    int success;
    char infoLog[1024];
    if (type != "PROGRAM")
    {
        glGetShaderiv(object, GL_COMPILE_STATUS, &success);
        if (!success)
        {
            glGetShaderInfoLog(object, 1024, NULL, infoLog);
            std::cout << "| ERROR::SHADER: Compile-time error: Type: " << type << "\n"
                      << infoLog << "\n -- --------------------------------------------------- -- "
                      << std::endl;
        }
    }
    else
    {
        glGetProgramiv(object, GL_LINK_STATUS, &success);
        if (!success)
        {
            glGetProgramInfoLog(object, 1024, NULL, infoLog);
            std::cout << "| ERROR::Shader: Link-time error: Type: " << type << "\n"
                      << infoLog << "\n -- --------------------------------------------------- -- "
                      << std::endl;
        }
    }
}

GridBin::~GridBin() {
    if (VISUALIZE) {
        glDeleteVertexArrays(1, &VAO);
        glDeleteBuffers(1, &VBO);
    }
}


bool GridBin::insert(Boid *b) {
    if (b->position.x < bounds_x.x || b->position.x >= bounds_x.y || 
        b->position.y < bounds_y.x || b->position.y >= bounds_y.y) {
            return false;
    }

    if (num_boids < NODE_CAPACITY) {
        boids[num_boids] = b;
        num_boids += 1;
        return true;
    }

    return false;
}

/**
 * Credit to 15-418/618 Staff for this function!!
 */ 
bool circleInRect(glm::vec2 &circle_pos, glm::vec2 &bounds_x, glm::vec2 &bounds_y) {
    // clamp circle center to box (finds the closest point on the box)
    float closestX = (circle_pos.x > bounds_x.x) ? ((circle_pos.x < bounds_x.y) ? circle_pos.x : bounds_x.y) : bounds_x.x;
    float closestY = (circle_pos.y > bounds_y.x) ? ((circle_pos.y < bounds_y.y) ? circle_pos.y : bounds_y.y) : bounds_y.x;

    // is circle radius less than the distance to the closest point on
    // the box?
    float distX = closestX - circle_pos.x;
    float distY = closestY - circle_pos.y;

    if ( ((distX*distX) + (distY*distY)) <= (nearby_dist*nearby_dist) ) {
        return true;
    } else {
        return false;
    }
}

void GridBin::query(Boid *b, std::function<void(Boid *)> &iterate_function) {
    // Check if sphere of influence intersects this bbox
    if (!circleInRect(b->position, bounds_x, bounds_y)) {
        return;
    }

    for (int i = 0; i < num_boids; i++) {
        iterate_function(boids[i]);
    }
    return;
}

void GridBin::clear() {
    num_boids = 0;
}

void GridBin::visualize() {
    assert(VISUALIZE);
    // Assumes a vao is bound, shader in use, MVP matrix bound

    
    float vertices[] = {
        bounds_x.x, bounds_y.x, 0.0f, // bottom left corner
        bounds_x.y, bounds_y.x, 0.0f, // bottom right corner
        bounds_x.y, bounds_y.y, 0.0f, // top right corner
        bounds_x.x, bounds_y.y, 0.0f, // top left corner
    };

    for (int i = 0; i < 12; i+=3) {
        vertices[i] = ((vertices[i] / SCREEN_WIDTH) - 0.5f) * 2;
        vertices[i+1] = (((SCREEN_HEIGHT - vertices[i+1]) / SCREEN_HEIGHT) - 0.5f) * 2;

    }

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glDrawArrays(GL_LINE_LOOP, 0, 4);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

}
