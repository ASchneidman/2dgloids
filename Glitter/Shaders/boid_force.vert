
out float force_x;
out float force_y;

uniform vec2 positions[NUM_BOIDS];
uniform vec2 velocities[NUM_BOIDS];

uniform float nearby_dist;
uniform float max_velocity;
uniform float max_force;


vec2 clamp_magnitude(vec2 vec, float max_value) {
    if (length(vec) > max_value) {
        vec2 v = normalize(vec);
        v *= max_value;
        return v;
    }
    return vec;
}

vec2 SteerToward(vec2 force, vec2 velocity) {
    return clamp_magnitude(normalize(force) * max_velocity - velocity, max_force); 
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

    vec2 my_position = positions[gl_VertexID];
    vec2 my_velocity = velocities[gl_VertexID];
    

    for (int i = 0; i < NUM_BOIDS; i++) {
        if (i == gl_VertexID) {
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
            float scaling = (1.0f / (dist * dist));
            vec2 dir = normalize(my_position - other_position);
            forceCollision += dir * scaling;
        }
    }

    if (numClose > 0) {
        forceAlign = flockHeading;
        forceAlign /= numClose;

        flockCenter /= numClose;
        forcePos = (flockCenter - my_position);

        forceCollision = SteerToward(forceCollision, my_velocity);

        forceAlign = SteerToward(forceAlign, my_velocity);

        forcePos = SteerToward(forcePos, my_velocity);

        force = forceCollision + forceAlign + forcePos;
    }

    force_x = force.x;
    force_y = force.y;
}

