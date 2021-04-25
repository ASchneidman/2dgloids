#include "QuadTree.hpp"
#include <iostream>
#include <omp.h>
#include <string.h>

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

std::vector<QuadTree_t *> *all_nodes;

/**
 * Credit to 15-418/618 Staff for this function!!
 */ 
bool circleInRect(glm::vec2 &circle_pos, float x_min, float x_max, float y_min, float y_max) {
    // clamp circle center to box (finds the closest point on the box)
    float closestX = (circle_pos.x > x_min) ? ((circle_pos.x < x_max) ? circle_pos.x : x_max) : x_min;
    float closestY = (circle_pos.y > y_min) ? ((circle_pos.y < y_max) ? circle_pos.y : y_max) : y_min;

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


QuadTreeHead::QuadTreeHead(glm::vec2 bounds_x, glm::vec2 bounds_y) {
    //first = new QuadTree();
    first = (QuadTree_t*)malloc(sizeof(QuadTree_t));
    qt_init(first);
    this->bounds_x = bounds_x;
    this->bounds_y = bounds_y;

    all_nodes = &nodes;

    #ifdef VISUALIZE
    // vertex shader
    int vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &lineVertShader, NULL);
    glCompileShader(vertexShader);
    // check for shader compile errors
    checkCompileErrors(vertexShader, "VERTEX");
    // fragment shader
    int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &lineFragShader, NULL);
    glCompileShader(fragmentShader);
    // check for shader compile errors    
    checkCompileErrors(vertexShader, "VERTEX");


    // link shaders
    lineShaderProgram = glCreateProgram();
    glAttachShader(lineShaderProgram, vertexShader);
    glAttachShader(lineShaderProgram, fragmentShader);
    glLinkProgram(lineShaderProgram);
    // check for linking errors
    checkCompileErrors(lineShaderProgram, "PROGRAM");

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    glUseProgram(lineShaderProgram);
    #endif
}

void QuadTreeHead::clear() {
    //first->clear();
    //qt_clear(first);
    #pragma omp parallel for schedule(static) num_threads(THREADS)
    for (size_t i = 0; i < nodes.size(); i++) {
        nodes[i]->num_boids = 0;
        nodes[i]->is_subdivided = false;
    }
}

QuadTreeHead::~QuadTreeHead() {
    //delete first;
    qt_free(first);
    free(first);
    //if (VISUALIZE)
    #ifdef VISUALIZE
    glDeleteProgram(lineShaderProgram);
    #endif
}

void QuadTreeHead::visualize() {
    glUseProgram(lineShaderProgram);
    qt_visualize(first, bounds_x.x, bounds_x.y, bounds_y.x, bounds_y.y);
}

void qt_init(QuadTree_t *qt) {
    memset((void*)qt, 0, sizeof(QuadTree_t));

    qt->insert_lock = (omp_lock_t*)malloc(sizeof(omp_lock_t));
    qt->subdivide_lock = (omp_lock_t*)malloc(sizeof(omp_lock_t));
    omp_init_lock(qt->insert_lock);
    omp_init_lock(qt->subdivide_lock);
} 

void qt_free(QuadTree_t *qt) {
    if (qt->is_subdivided) {
        qt_free(qt->upperLeft);
        qt_free(qt->upperRight);
        qt_free(qt->lowerLeft);
        qt_free(qt->lowerRight);
    }
    omp_destroy_lock(qt->insert_lock);
    omp_destroy_lock(qt->subdivide_lock);
    free(qt->insert_lock);
    free(qt->subdivide_lock);
    if (VISUALIZE) {
        if (qt->buffers_initialized) {
            glDeleteVertexArrays(1, &qt->VAO);
            glDeleteBuffers(1, &qt->VBO);
        }
    }
}

bool qt_insert(QuadTree_t *qt, Boid *b, float x_min, float x_max, float y_min, float y_max) {
    if (b->position.x < x_min || b->position.x >= x_max || 
        b->position.y < y_min || b->position.y >= y_max) {
            return false;
    }

    int success = 0;
    omp_set_lock(qt->insert_lock);
    if (qt->num_boids < NODE_CAPACITY) {
        qt->boids[qt->num_boids] = b;
        qt->num_boids += 1;
        success = 1;
    }
    omp_unset_lock(qt->insert_lock);
    if (success) {
        return true;
    }


    int did_subdivde = 0;
    if (!qt->is_subdivided) {
        omp_set_lock(qt->subdivide_lock);
        if (!qt->is_subdivided) {
            did_subdivde = 1;

            if (qt->upperLeft == NULL) {
                // alloc all in close proximity
                QuadTree_t *children = (QuadTree_t*)malloc(sizeof(QuadTree_t) * 4);
                qt->upperLeft = children;
                qt->upperRight = &children[1];
                qt->lowerLeft = &children[2];
                qt->lowerRight = &children[3];
                qt_init(qt->upperLeft);
                qt_init(qt->upperRight);
                qt_init(qt->lowerLeft);
                qt_init(qt->lowerRight);
            }
            qt->is_subdivided = true;
        }
        omp_unset_lock(qt->subdivide_lock);
    }

    if (did_subdivde) {
        // This can be done w/o the subdivide lock
        #pragma omp critical
        {
            all_nodes->push_back(qt->upperLeft);
            all_nodes->push_back(qt->upperRight);
            all_nodes->push_back(qt->lowerLeft);
            all_nodes->push_back(qt->lowerRight);
        }
    }

    float middle_x = (x_min + x_max) / 2;
    float middle_y = (y_min + y_max) / 2;
    if (did_subdivde) {
        for (int i = 0; i < qt->num_boids; i++) {
            Boid *o = qt->boids[i];
            if (qt_insert(qt->upperLeft, o, x_min, middle_x, middle_y, y_max))
                continue;
            if (qt_insert(qt->upperRight, o, middle_x, x_max, middle_y, y_max))
                continue;
            if (qt_insert(qt->lowerLeft, o, x_min, middle_x, y_min, middle_y))
                continue;
            if (qt_insert(qt->lowerRight, o, middle_x, x_max, y_min, middle_y))
                continue;
        }
    }
    

    if (qt_insert(qt->upperLeft, b, x_min, middle_x, middle_y, y_max))
        return true;
    if (qt_insert(qt->upperRight, b, middle_x, x_max, middle_y, y_max))
        return true;
    if (qt_insert(qt->lowerLeft, b, x_min, middle_x, y_min, middle_y))
        return true;
    if (qt_insert(qt->lowerRight, b, middle_x, x_max, y_min, middle_y))
        return true;

    return false;
} 

//void qt_query(QuadTree_t *qt, Boid *b, std::function<void(Boid *)> &iterate_function, float x_min, float x_max, float y_min, float y_max) {
void qt_query(QuadTree_t *qt, Boid *b, std::vector<Boid *> &boids, float x_min, float x_max, float y_min, float y_max) {
    // Check if sphere of influence intersects this bbox
    if (!circleInRect(b->position, x_min, x_max, y_min, y_max)) {
        return;
    }

    if (!qt->is_subdivided) {
        // Not subdivided, so iterate
        for (int i = 0; i < qt->num_boids; i++) {
            //iterate_function(qt->boids[i]);
            boids.push_back(qt->boids[i]);
        }
        return;
    }
    float middle_x = (x_min + x_max) / 2;
    float middle_y = (y_min + y_max) / 2;
    qt_query(qt->upperLeft, b, boids, x_min, middle_x, middle_y, y_max);
    qt_query(qt->upperRight, b, boids, middle_x, x_max, middle_y, y_max);
    qt_query(qt->lowerLeft, b, boids, x_min, middle_x, y_min, middle_y);
    qt_query(qt->lowerRight, b, boids, middle_x, x_max, y_min, middle_y);
    /*
    qt_query(qt->upperLeft, b, iterate_function, x_min, middle_x, middle_y, y_max);
    qt_query(qt->upperRight, b, iterate_function, middle_x, x_max, middle_y, y_max);
    qt_query(qt->lowerLeft, b, iterate_function, x_min, middle_x, y_min, middle_y);
    qt_query(qt->lowerRight, b, iterate_function, middle_x, x_max, y_min, middle_y);
    */

}

void qt_clear(QuadTree_t *qt) {
    qt->num_boids = 0;
    if (qt->is_subdivided) {
        qt_clear(qt->upperLeft);
        qt_clear(qt->upperRight);
        qt_clear(qt->lowerLeft);
        qt_clear(qt->lowerRight);
    }
    qt->is_subdivided = false;
}

void qt_clear_left(QuadTree_t *qt) {
    qt->num_boids = 0;
    if (qt->is_subdivided) {
        qt_clear_left(qt->upperLeft);
        qt_clear_left(qt->lowerLeft);
    }
    qt->is_subdivided = false;
}

void qt_clear_right(QuadTree_t *qt) {
    qt->num_boids = 0;
    if (qt->is_subdivided) {
        qt_clear_right(qt->upperRight);
        qt_clear_right(qt->lowerRight);
    }
    qt->is_subdivided = false;
}

void qt_visualize(QuadTree_t *qt, float x_min, float x_max, float y_min, float y_max) {
    if (!qt->buffers_initialized) {
        qt->buffers_initialized = 1;
        glGenBuffers(1, &qt->VBO);
        glGenVertexArrays(1, &qt->VAO);
    }
    
    float vertices[] = {
        x_min, y_min, 0.0f, // bottom left corner
        x_max, y_min, 0.0f, // bottom right corner
        x_max, y_max, 0.0f, // top right corner
        x_min, y_max, 0.0f, // top left corner
    };

    for (int i = 0; i < 12; i+=3) {
        vertices[i] = ((vertices[i] / SCREEN_WIDTH) - 0.5f) * 2;
        vertices[i+1] = (((SCREEN_HEIGHT - vertices[i+1]) / SCREEN_HEIGHT) - 0.5f) * 2;

    }

    glBindVertexArray(qt->VAO);

    glBindBuffer(GL_ARRAY_BUFFER, qt->VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glDrawArrays(GL_LINE_LOOP, 0, 4);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    if (qt->is_subdivided) {
        float middle_x = (x_min + x_max) / 2;
        float middle_y = (y_min + y_max) / 2;
        qt_visualize(qt->upperLeft, x_min, middle_x, middle_y, y_max);
        qt_visualize(qt->upperRight, middle_x, x_max, middle_y, y_max);
        qt_visualize(qt->lowerLeft, x_min, middle_x, y_min, middle_y);
        qt_visualize(qt->lowerRight, middle_x, x_max, y_min, middle_y);
    }
}