#version 330 core
in int index;
out vec2 out_force;

uniform vec2 positions[NUM_BOIDS];
uniform vec2 velocities[NUM_BOIDS];

uniform float nearby_dist;

vec2 clamp_magnitude(vec2 vec, float max_value) {
    if (length(vec) > max_value) {
        vec2 v = normalize(vec);
        v *= max_value;
        return v;
    }
    return vec;
}

vec2 SteerToward(vec2 force, vec2 velocity) {
    return clamp_magnitude(normalize(force) * max_velocity - velocity, MAX_FORCE); 
}

void main() {
    vec2 forceCollision = vec2(0.0, 0.0);
    vec2 forceAlign = vec2(0.0, 0.0);
    vec2 forcePos = vec2(0.0, 0.0);
    vec2 force = vec2(0.0, 0.0);
    vec2 forceRand = vec2(0.0, 0.0);

    vec2 flockCenter = vec2(0.0, 0.0);
    vec2 flockHeading = vec2(0.0, 0.0);

    int numClose = 0;

    vec2 my_position = positions[index];
    vec2 my_velocity = velocities[index];

    for (int i = 0; i < NUM_BOIDS; i++) {
        if (i == index) {
            continue;
        }
        vec2 other_position = positions[i];
        vec2 other_velocity = velocities[i];
        float dist = distance(other_position, my_position);
        if (dist < nearby_dist) {
            flockCenter += other_position;
            flockHeading += other_velocity;
            numClose += 1;
            // extra dist is so dir is normalized
            float scaling = (1.0f / (dist * dist * dist));
            glm::vec2 dir = my_position - other_position;
            forceCollision += dir * scaling;
        }
    }

    if (numClose > 0) {
        forceAlign = flockHeading;
        forceAlign /= numClose;

        flockCenter /= numClose;
        forcePos = (flockCenter - b->position);

        forceCollision = SteerToward(forceCollision, my_velocity);
        forceCollision *= collision_weight;

        forceAlign = b->SteerToward(forceAlign, my_velocity);
        forceAlign *= align_weight;

        forcePos = b->SteerToward(forcePos, my_velocity);
        forcePos *= position_weight;

        force = forceCollision + forceAlign + forcePos;
    }

    out_force = force;
}

