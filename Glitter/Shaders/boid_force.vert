
out vec2 total_force;

uniform samplerBuffer position_velocity;

// should be only around 400 bytes
layout (packed) uniform sizes {
    int grid_cell_sizes[n_rows * n_cols];
};

uniform float nearby_dist;
uniform float max_velocity;
uniform float max_force;

uniform float collision_weight;
uniform float align_weight;
uniform float position_weight;

uniform int position_velocity_offset;

/**
 * Credit to 15-418/618 Staff for this function!!
 */ 
int circleInRect(in vec2 circle_pos, in vec2 x_range, in vec2 y_range) {
    // clamp circle center to box (finds the closest point on the box)
    float closestX = (circle_pos.x > x_range.x) ? ((circle_pos.x < x_range.y) ? circle_pos.x : x_range.y) : x_range.x;
    float closestY = (circle_pos.y > y_range.x) ? ((circle_pos.y < y_range.y) ? circle_pos.y : y_range.y) : y_range.x;

    // is circle radius less than the distance to the closest point on
    // the box?
    float distX = closestX - circle_pos.x;
    float distY = closestY - circle_pos.y;

    if ( ((distX*distX) + (distY*distY)) <= (nearby_dist*nearby_dist) ) {
        return 1;
    } else {
        return 0;
    }
}

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

    // Offset into texture where positions and velocities are stored
    // for each grid cell, have to store the number of boids in that cell and the sum 
    // of total boid indices will be NUM_BOIDS, however each texel stores 4 indices

    vec4 p_v = texelFetch(position_velocity, gl_InstanceID + position_velocity_offset);

    vec2 my_position = p_v.xy;
    vec2 my_velocity = p_v.zw;
    

    int tex_index = 0;
    float grid_width = screen_width / float(n_rows);
    float grid_height = screen_height / float(n_cols);

    for (int r = 0; r < n_rows; r++) {
    for (int c = 0; c < n_cols; c++) {
        // number of boids in this grid cell
        int n_boids = grid_cell_sizes[r * n_cols + c];

        vec2 x_range = vec2(grid_width * r, grid_width * (r+1));
        vec2 y_range = vec2(grid_height * c, grid_height * (c+1));
        // check if my circle intersects this bbox
        if (circleInRect(my_position, x_range, y_range) == 0) {
            // don't intersect, so skip to the next grid cell tex index
            tex_index += n_boids / 4 + int(n_boids % 4 != 0);
            continue;
        }
        
        // Circle does intersect, so iterate through boids
        for (int b = 0; b < n_boids; b+=4) {
            vec4 next_four_boids = texelFetch(position_velocity, tex_index);
            for (int j = 0; j < 4; j++) {
                if (b + j >= n_boids) {
                    break;
                }
                int i;
                if (j == 0) {
                    i = int(next_four_boids.x);
                } else if (j == 1) {
                    i = int(next_four_boids.y);
                } else if (j == 2) {
                    i = int(next_four_boids.z);
                } else {
                    i = int(next_four_boids.w);
                }

                if (i == gl_InstanceID) {
                    continue;
                }

                vec4 other_p_v = texelFetch(position_velocity, position_velocity_offset + i);
                vec2 other_position = other_p_v.xy;
                vec2 other_velocity = other_p_v.zw;

                float dist = distance(other_position, my_position);
                float is_near = float(dist < nearby_dist);
                if (dist < nearby_dist) {
                    flockCenter += other_position * is_near;
                    flockHeading += other_velocity * is_near;
                    numClose += int(dist < nearby_dist);
                    // extra dist is so dir is normalized
                    float scaling = (1.0f / (dist * dist));
                    vec2 dir = normalize(my_position - other_position);
                    forceCollision += dir * scaling * is_near;
                }
            }
            tex_index += 1;
        }


    }
    }

    if (numClose > 0) {
        forceAlign = flockHeading;
        forceAlign /= numClose;

        flockCenter /= numClose;
        forcePos = (flockCenter - my_position);

        forceCollision = SteerToward(forceCollision, my_velocity);
        forceCollision *= collision_weight;

        forceAlign = SteerToward(forceAlign, my_velocity);
        forceAlign *= align_weight;

        forcePos = SteerToward(forcePos, my_velocity);
        forcePos *= position_weight;

        force = forceCollision + forceAlign + forcePos;
    }

    total_force = force;
}

