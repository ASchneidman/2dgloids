//layout (location = 0) in vec4 p_v;
//layout (location = 0) in int b_index;

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

    //total_force = vec2(b_index, 0);
    //return;

    //vec4 p_v = texelFetch(position_velocity, b_index);
    vec4 p_v = texelFetch(position_velocity, gl_InstanceID);

    vec2 my_position = p_v.xy;
    vec2 my_velocity = p_v.zw;


    
    float grid_width = screen_width / float(n_rows);
    float grid_height = screen_height / float(n_cols);

    int minX = int(float(n_rows) * (max(0.0, my_position.x - nearby_dist) / screen_width));
    int maxX = int(float(n_rows) * (min(screen_width, my_position.x + nearby_dist) / screen_width)) + 1;
    int minY = int(float(n_cols) * (max(0.0, my_position.y - nearby_dist) / screen_height));
    int maxY = int(float(n_cols) * (min(screen_height, my_position.y + nearby_dist) / screen_height)) + 1;

    int tex_size = textureSize(position_velocity);

    for (int r = minX; r < min(maxX, n_rows); r++) {
    for (int c = minY; c < min(maxY, n_cols); c++) {
        // number of boids in this grid cell
        //int n_boids = grid_cell_sizes[r * n_cols + c];
        int tex_index = grid_cell_sizes[r * n_cols + c];
        int n_boids;
        if (r * n_cols + c == n_rows * n_cols - 1) {
            n_boids = tex_size - tex_index;
        } else {
            n_boids = grid_cell_sizes[r * n_cols + c + 1] - tex_index;
        }

        vec2 x_range = vec2(grid_width * r, grid_width * (r+1));
        vec2 y_range = vec2(grid_height * c, grid_height * (c+1));
        
        // Circle does intersect, so iterate through boids
        for (int b = 0; b < n_boids; b++) {
            vec4 other_p_v = texelFetch(position_velocity, tex_index);
            vec2 other_position = other_p_v.xy;
            vec2 other_velocity = other_p_v.zw;
            // don't have indices of boids, so if position and velocity is 
            // same, skip
            float not_me = float(my_position != other_position || my_velocity != other_velocity);
            
            if (my_position == other_position && my_velocity == other_velocity) {
                tex_index += 1;
                continue;
            }

            float dist = distance(other_position, my_position);
            float is_near = float(dist < nearby_dist) * not_me;

            flockCenter += other_position * is_near;
            flockHeading += other_velocity * is_near;
            numClose += int(is_near);
            // extra dist is so dir is normalized
            float scaling = (1.0f / (dist * dist));
            vec2 dir = normalize(my_position - other_position);
            forceCollision += dir * scaling * is_near;

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
