#include "QuadTree.hpp"
#include <iostream>

QuadTree::~QuadTree() {
    if (upperLeft != NULL) {
        delete upperLeft;
        delete upperRight;
        delete lowerLeft;
        delete lowerRight;
    }
}

bool QuadTree::insert(Boid *b) {
    if (b->position.x < bounds_x.x || b->position.x >= bounds_x.y || 
        b->position.y < bounds_y.x || b->position.y >= bounds_y.y) {
            return false;
    }

    if (num_boids < NODE_CAPACITY) {
        boids[num_boids] = b;
        num_boids += 1;
        return true;
    }

    if (upperLeft == NULL) {
        float middle_x = (bounds_x.y + bounds_x.x) / 2;
        float middle_y = (bounds_y.y + bounds_y.x) / 2;
        upperLeft = new QuadTree(glm::vec2(bounds_x.x, middle_x), glm::vec2(middle_y, bounds_y.y));
        upperRight = new QuadTree(glm::vec2(middle_x, bounds_x.y), glm::vec2(middle_y, bounds_y.y));

        lowerLeft = new QuadTree(glm::vec2(bounds_x.x, middle_x), glm::vec2(bounds_y.x, middle_y));
        lowerRight = new QuadTree(glm::vec2(middle_x, bounds_x.y), glm::vec2(bounds_y.x, middle_y));

        for (int i = 0; i < num_boids; i++) {
            Boid *o = boids[i];
            if (upperLeft->insert(o))
                continue;
            if (upperRight->insert(o))
                continue;
            if (lowerLeft->insert(o))
                continue;
            if (lowerRight->insert(o))
                continue;
        }
    }

    if (upperLeft->insert(b))
        return true;
    if (upperRight->insert(b))
        return true;
    if (lowerLeft->insert(b))
        return true;
    if (lowerRight->insert(b))
        return true;

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

void QuadTree::query(Boid *b, std::function<void(Boid *)> &iterate_function) {
    // Check if sphere of influence intersects this bbox
    if (!circleInRect(b->position, bounds_x, bounds_y)) {
        return;
    }

    if (upperLeft == NULL) {
        // Not subdivided, so iterate
        for (int i = 0; i < num_boids; i++) {
            iterate_function(boids[i]);
        }
        return;
    }

    upperLeft->query(b, iterate_function);
    upperRight->query(b, iterate_function);
    lowerLeft->query(b, iterate_function);
    lowerRight->query(b, iterate_function);
}
