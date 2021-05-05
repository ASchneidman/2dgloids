
struct Node
{
    // Children
    int is_subdivided;
    int num_boids;
    int first_child;
    int boids_start_index;
};

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

void query(uniform float radius, 
            float<2> &forceCollision, 
            float<2> &forceAlign, 
            float<2> &forcePos, 
            float<2> &flockCenter, 
            float<2> &flockHeading,
            float<4> &boid_p_v,
            int &numClose,
            int node,
            float<4> bbox,
            uniform Node nodes[],
            uniform float positions_velocities[]) {
    
    if (circleInRect(boid_p_v.xy, bbox, radius) == 0) {
        return;
    }

    if (nodes[node].is_subdivided == 0) {
        for (int other = nodes[node].boids_start_index; other < nodes[node].boids_start_index + nodes[node].num_boids; other++) {
            float<4> other_p_v = get_p_v(positions_velocities, 4 * other);

            bool not_me = !(other_p_v.x == boid_p_v.x && other_p_v.y == boid_p_v.y && other_p_v.z == boid_p_v.z && other_p_v.w == boid_p_v.w);

            //if (other_p_v.x == boid_p_v.x && other_p_v.y == boid_p_v.y && other_p_v.z == boid_p_v.z && other_p_v.w == boid_p_v.w) {
            //    continue;
            //}
            // Collision avoidance
            float dist = distance(other_p_v.xy, boid_p_v.xy);
            int within_dist = (dist < radius) & (not_me);
            //if (dist < radius) {
                flockCenter += other_p_v.xy * within_dist;
                flockHeading += other_p_v.zw * within_dist;
                numClose += within_dist;
                float scaling = (1.0f / (dist * dist + (1 - within_dist)));
                float<2> dir = normalize(boid_p_v.xy - other_p_v.xy);
                forceCollision += dir * scaling * within_dist;
            //}
        }
        return;
    }

    float middle_x = (bbox.x + bbox.y) / 2;
    float middle_y = (bbox.z + bbox.w) / 2;

    float<4> upperLeft = {bbox.x, middle_x, middle_y, bbox.w};
    float<4> upperRight = {middle_x, bbox.y, middle_y, bbox.w};
    float<4> lowerLeft = {bbox.x, middle_x, bbox.z, middle_y};
    float <4> lowerRight = {middle_x, bbox.y, bbox.z, middle_y};

    int first_child = nodes[node].first_child;
    query(radius, forceCollision, forceAlign, forcePos, flockCenter, flockHeading, boid_p_v, numClose, first_child, upperLeft, nodes, positions_velocities);
    query(radius, forceCollision, forceAlign, forcePos, flockCenter, flockHeading, boid_p_v, numClose, first_child + 1, upperRight, nodes, positions_velocities);
    query(radius, forceCollision, forceAlign, forcePos, flockCenter, flockHeading, boid_p_v, numClose, first_child + 2, lowerLeft, nodes, positions_velocities);
    query(radius, forceCollision, forceAlign, forcePos, flockCenter, flockHeading, boid_p_v, numClose, first_child + 3, lowerRight, nodes, positions_velocities);
}

export void compute_forces(uniform int boid_indices[],
                      uniform int num_boids,
                      uniform float radius,
                      uniform float out_forces[], 
                      uniform float positions_velocities[],
                      uniform int positions_velocity_indices[],
                      uniform Node nodes[],
                      uniform float bounds[],
                      uniform float max_velocity,
                      uniform float max_force,
                      uniform float collision_weight,
                      uniform float align_weight,
                      uniform float position_weight) {
        
        foreach(i = 0 ... num_boids-1)
        {
            int boid = boid_indices[i];

            float<4> bbox = {bounds[0], bounds[1], bounds[2], bounds[3]};
            int p_v_index = 4 * positions_velocity_indices[boid];
            float<4> my_p_v = get_p_v(positions_velocities, p_v_index);

            float<2> forceCollision = {0.0, 0.0};
            float<2> forceAlign = {0.0, 0.0};
            float<2> forcePos = {0.0, 0.0};
            float<2> force = {0.0, 0.0};

            float<2> flockCenter = {0.0, 0.0};
            float<2> flockHeading = {0.0, 0.0};

            int numClose = 0;

            query(radius, forceCollision, forceAlign, forcePos, flockCenter, flockHeading, my_p_v, numClose, 0, bbox, nodes, positions_velocities);
    

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
