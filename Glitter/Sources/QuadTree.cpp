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

//std::vector<QuadTree_t *> *all_nodes;

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
/*
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
}*/


QuadTreeHead::QuadTreeHead(glm::vec2 bounds_x, glm::vec2 bounds_y, std::vector<Boid *> *boids) {
    // Preallocate all elements
    elements.reserve(NUM_BOIDS);
    while (elements.size() < NUM_BOIDS) {
        elements.push_back(new QuadTreeElem_t);
        free_elements.push_back(elements.size()-1);
    }


    this->bounds_x = bounds_x;
    this->bounds_y = bounds_y;
    nodes.push_back(new QuadTree_t);
    qt_init(nodes[0]);

    this->boids = boids;

    /*
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
    */
}

void QuadTreeHead::clear() {
    free_elements.clear();
    for (int i = 0; i < elements.size(); i++) {
        free_elements.push_back(i);
    }

    #pragma omp parallel for schedule(static) num_threads(THREADS)
    for (size_t i = 0; i < nodes.size(); i++) {
        nodes[i]->num_boids = 0;
        nodes[i]->is_subdivided = false;
        nodes[i]->first_element = -1;
        // Do not set first child to -1 since it can reuse the preallocated children
    }
}

/*
void QuadTreeHead::visualize() {
    glUseProgram(lineShaderProgram);
    qt_visualize(nodes[0], bounds_x.x, bounds_x.y, bounds_y.x, bounds_y.y);
}
*/

void qt_init(QuadTree_t *qt) {
    qt->first_child = -1;
    qt->first_element = -1;
    qt->num_boids = 0;
    qt->is_subdivided = false;
}

bool qt_insert(QuadTreeHead *head, QuadTree_t *qt, Boid *b, float x_min, float x_max, float y_min, float y_max, int depth) {
    if (b->position.x < x_min || b->position.x >= x_max || 
        b->position.y < y_min || b->position.y >= y_max) {
            return false;
    }

    if (qt->num_boids < NODE_CAPACITY || depth == MAX_DEPTH) {
        int elem_index = head->alloc_elem();
        QuadTreeElem_t *elem = head->elements[elem_index];
        elem->boid = b->index;
        elem->next = qt->first_element;
        // first boid is now the last element in the elements list
        qt->first_element = elem_index;
        qt->num_boids += 1;
        return true;
    }

    float middle_x = (x_min + x_max) / 2;
    float middle_y = (y_min + y_max) / 2;

    if (!qt->is_subdivided) {

        if (qt->first_child == -1) {
            QuadTree_t *children = new QuadTree_t[4];
            head->nodes.push_back(&children[0]);
            qt->first_child = head->nodes.size() - 1;
            head->nodes.push_back(&children[1]);
            head->nodes.push_back(&children[2]);
            head->nodes.push_back(&children[3]);
            qt_init(&children[0]);
            qt_init(&children[1]);
            qt_init(&children[2]);
            qt_init(&children[3]);
        }


        // Insert all my boids
        int current_child = qt->first_element;
        for (int i = 0; i < qt->num_boids; i++) {
            //assert(current_child != -1);
            QuadTreeElem_t *elem = head->elements[current_child];
            int next_child = elem->next;
            Boid *o = (*head->boids)[elem->boid];
            // Remove elem from alloced list
            head->dealloc_elem(current_child);

            current_child = next_child;
            
            if (qt_insert(head, head->nodes[qt->first_child], o, x_min, middle_x, middle_y, y_max, depth+1))
                continue;
            if (qt_insert(head, head->nodes[qt->first_child + 1], o, middle_x, x_max, middle_y, y_max, depth+1))
                continue;
            if (qt_insert(head, head->nodes[qt->first_child + 2], o, x_min, middle_x, y_min, middle_y, depth+1))
                continue;
            if (qt_insert(head, head->nodes[qt->first_child + 3], o, middle_x, x_max, y_min, middle_y, depth+1))
                continue;
            
        }

        qt->is_subdivided = true;
        qt->first_element = -1;
    }


    if (qt_insert(head, head->nodes[qt->first_child], b, x_min, middle_x, middle_y, y_max, depth+1))
        return true;
    if (qt_insert(head, head->nodes[qt->first_child + 1], b, middle_x, x_max, middle_y, y_max, depth+1))
        return true;
    if (qt_insert(head, head->nodes[qt->first_child + 2], b, x_min, middle_x, y_min, middle_y, depth+1))
        return true;
    if (qt_insert(head, head->nodes[qt->first_child + 3], b, middle_x, x_max, y_min, middle_y, depth+1))
        return true;
    return false;
} 

void qt_query(QuadTreeHead *head, QuadTree_t *qt, Boid *b, std::vector<Boid *> &boids, float x_min, float x_max, float y_min, float y_max) {
    // Check if sphere of influence intersects this bbox
    if (!circleInRect(b->position, x_min, x_max, y_min, y_max)) {
        return;
    }

    if (!qt->is_subdivided) {
        // Not subdivided, so iterate
        int current_child = qt->first_element;
        for (int i = 0; i < qt->num_boids; i++) {
            //assert(current_child != -1);
            QuadTreeElem_t *elem = head->elements[current_child];
            boids.push_back((*head->boids)[elem->boid]);
            current_child = elem->next;
        }
        return;
    }
    float middle_x = (x_min + x_max) / 2;
    float middle_y = (y_min + y_max) / 2;

    qt_query(head, head->nodes[qt->first_child], b, boids, x_min, middle_x, middle_y, y_max);
    qt_query(head, head->nodes[qt->first_child + 1], b, boids, middle_x, x_max, middle_y, y_max);
    qt_query(head, head->nodes[qt->first_child + 2], b, boids, x_min, middle_x, y_min, middle_y);
    qt_query(head, head->nodes[qt->first_child + 3], b, boids, middle_x, x_max, y_min, middle_y);
}

/*
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
*/