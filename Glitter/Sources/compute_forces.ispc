static inline float length(float<2> vec) {
    return sqrt(vec.x * vec.x + vec.y * vec.y);
}

static inline float<2> normalize(float<2> vec) {
    return vec / (length(vec) + 1e-10);
}

static inline float distance(float<2> a, float<2> b) {
    return length(a-b);
}

static inline float<4> get_p_v(uniform float positions_velocities[], int p_v_index) {
    float<4> ret = {positions_velocities[p_v_index], positions_velocities[p_v_index + 1], positions_velocities[p_v_index + 2], positions_velocities[p_v_index + 3]};
    return ret;
}

float<2> clamp_magnitude(float<2> vec, float max_value) {
    if (length(vec) > max_value) {
        float<2> v = normalize(vec);
        v *= max_value;
        return v;
    }
    return vec;
}

float<2> SteerToward(float<2> force, float<2> velocity, uniform float max_velocity, uniform float max_force) {
    return clamp_magnitude(normalize(force) * max_velocity - velocity, max_force); 
}
int circleInRect(float<2> circle_pos, float<4> bbox, uniform float radius) {
    // clamp circle center to box (finds the closest point on the box)
    float closestX = (circle_pos.x > bbox.x) ? ((circle_pos.x < bbox.y) ? circle_pos.x : bbox.y) : bbox.x;
    float closestY = (circle_pos.y > bbox.z) ? ((circle_pos.y < bbox.w) ? circle_pos.y : bbox.w) : bbox.z;

    // is circle radius less than the distance to the closest point on
    // the box?
    float distX = closestX - circle_pos.x;
    float distY = closestY - circle_pos.y;

    if ( ((distX*distX) + (distY*distY)) <= (radius*radius) ) {
        return 1;
    } else {
        return 0;
    }
}

export void compute_forces(uniform int boid_indices[],
                      uniform int num_boids,
                      uniform float radius,
                      uniform float out_forces[], 
                      uniform float positions_velocities[],
                      uniform float positions_velocities_len,
                      uniform int positions_velocity_indices[],
                      uniform int grid_cell_sizes[],
                      uniform int n_rows,
                      uniform int n_cols,
                      uniform float bounds[],
                      uniform float max_velocity,
                      uniform float max_force,
                      uniform float collision_weight,
                      uniform float align_weight,
                      uniform float position_weight) {

    foreach (i = 0...num_boids-1) {
        int boid = boid_indices[i];

        float<4> bbox = {bounds[0], bounds[1], bounds[2], bounds[3]};
        int my_p_v_index = 4 * positions_velocity_indices[boid];
        float<4> my_p_v = get_p_v(positions_velocities, my_p_v_index);
        float<2> my_position = my_p_v.xy;
        float<2> my_velocity = my_p_v.zw;

        float<2> forceCollision = {0.0, 0.0};
        float<2> forceAlign = {0.0, 0.0};
        float<2> forcePos = {0.0, 0.0};
        float<2> force = {0.0, 0.0};

        float<2> flockCenter = {0.0, 0.0};
        float<2> flockHeading = {0.0, 0.0};

        int numClose = 0;

        float screen_width = bounds[1] - bounds[0];
        float screen_height = bounds[3] - bounds[2];
    
        float grid_width = screen_width / (float)(n_rows);
        float grid_height = screen_height / (float)(n_cols);

        int minX = (int)((float)(n_rows) * (max(0.0, my_position.x - radius) / screen_width));
        int maxX = (int)((float)(n_rows) * (min(screen_width, my_position.x + radius) / screen_width)) + 1;
        int minY = (int)((float)(n_cols) * (max(0.0, my_position.y - radius) / screen_height));
        int maxY = (int)((float)(n_cols) * (min(screen_height, my_position.y + radius) / screen_height)) + 1;


        for (int r = minX; r < min(maxX, n_rows); r++) {
        for (int c = minY; c < min(maxY, n_cols); c++) {
            int p_v_index = grid_cell_sizes[r * n_cols + c];
            int n_boids;
            if (r * n_cols + c == n_rows * n_cols - 1) {
                n_boids = positions_velocities_len - p_v_index;
            } else {
                n_boids = grid_cell_sizes[r * n_cols + c + 1] - p_v_index;
            }

            for (int b = 0; b < n_boids; b++) {

                float<4> other_p_v = get_p_v(positions_velocities, 4 * p_v_index);
                float<2> other_position = other_p_v.xy;
                float<2> other_velocity = other_p_v.zw;

                if (other_p_v.x == my_p_v.x && other_p_v.y == my_p_v.y && other_p_v.z == my_p_v.z && other_p_v.w == my_p_v.w) {
                    p_v_index += 1;
                    continue;
                }

                float dist = distance(other_position, my_position);
                if (dist < radius) {
                    flockCenter += other_position;
                    flockHeading += other_velocity;
                    numClose += 1;
                    // extra dist is so dir is normalized
                    float scaling = (1.0f / (dist * dist));
                    float<2> dir = normalize(my_position - other_position);
                    forceCollision += dir * scaling;
                }
                p_v_index += 1;
            }
        }
        }

        if (numClose > 0) {
            forceAlign = flockHeading;
            forceAlign /= numClose;

            flockCenter /= numClose;
            forcePos = (flockCenter - my_p_v.xy);

            forceCollision = SteerToward(forceCollision, my_p_v.zw, max_velocity, max_force);
            forceCollision *= collision_weight;

            forceAlign = SteerToward(forceAlign, my_p_v.zw, max_velocity, max_force);
            forceAlign *= align_weight;

            forcePos = SteerToward(forcePos, my_p_v.zw, max_velocity, max_force);
            forcePos *= position_weight;

            force = forceCollision + forceAlign + forcePos;

        }


        out_forces[2 * boid] = force.x;
        out_forces[2 * boid + 1] = force.y;
    }

}