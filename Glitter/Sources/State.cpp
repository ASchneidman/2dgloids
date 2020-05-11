//
// Created by Alex Schneidman on 2020-05-09.
//

#include "State.h"
#include "resourcemanager.h"
#include <random>
#include <map>
#include <iterator>

#define NUM_BOIDS 100
#define BOID_SPEED 100.0f
//#define MAP_SQUARES_X 250
//#define MAP_SQUARES_Y 90

#define COLLISION_WEIGHT (1000000.0f)
#define ALIGN_WEIGHT (500.0f)
#define POSITION_WEIGHT (200000.0f)

#define FORCE_CAP 1000.0f
#define SIGN(x) ((x) < 0.0f ? -1.0f : 1.0f)

#define NEARBY_DIST 250.0f

EntityRenderer *Renderer;
std::default_random_engine generator;
std::uniform_real_distribution<float> distribution(-100.0,100.0);

static std::map<int, std::map<int, std::map<long, Boid *>>> boidMap;

State::State(GLuint width, GLuint height) : Width(width), Height(height) {}

void State::Init() {
    //assert((this->Width % MAP_SQUARES_X) == 0);
    //assert((this->Height % MAP_SQUARES_Y) == 0);

    // load shaders
    ResourceManager::LoadShader("entity.vert", "entity.frag", nullptr, "entity");
    // configure shaders
    glm::mat4 projection = glm::ortho(0.0f, static_cast<float>(this->Width),
                                      static_cast<float>(this->Height), 0.0f, -1.0f, 1.0f);
    ResourceManager::GetShader("entity").Use().SetMatrix4("projection", projection);
    // set render-specific controls
    Shader sh = ResourceManager::GetShader("entity");
    Renderer = new EntityRenderer(sh);

    std::uniform_real_distribution<float> posDistX(0.0f, this->Width-1);
    std::uniform_real_distribution<float> posDistY(0.0f, this->Height-1);

    std::uniform_real_distribution<float> randDir(-1.0f, 1.0f);

    for (int i = 0; i < NUM_BOIDS; i++) {
        glm::vec2 initPos = glm::vec2(posDistX(generator), posDistY(generator));
        glm::vec2 initialVel = glm::normalize(glm::vec2(randDir(generator), randDir(generator))) * BOID_SPEED;
        this->boids.push_back(new Boid(initPos, initialVel, this->Width, this->Height));

        //boidMap[(int)(initPos.x / MAP_SQUARES_X)][(int)(initPos.y / MAP_SQUARES_Y)][(long)this->boids.at(i)] = (this->boids.at(i));
    }
}

void State::Update(GLfloat dt) {
    std::map<Boid *, glm::vec2> forces;

    for (Boid *b : this->boids) {
        glm::vec2 myPos(b->GetX(), b->GetY());
        glm::vec2 forceCollision(0.0f, 0.0f);
        glm::vec2 forceAlign;
        glm::vec2 forcePos;

        float avgVX, avgVY;
        float avgX, avgY;

        int numClose = 0;

        for (Boid *other : this->boids) {
            if (b == other)
                continue;

            // Collision avoidance
            glm::vec2 otherPos(other->GetX(), other->GetY());
            glm::vec2 dir = glm::normalize(myPos - otherPos);
            float dist = glm::distance(otherPos, myPos);
            if (dist < NEARBY_DIST) {
                avgVX += other->GetVelocity().x; avgVY += other->GetVelocity().y;
                avgX += other->GetX(); avgY += other->GetY();
                numClose += 1;
                float scaling = (1.0f / (dist * dist));
                forceCollision += dir * scaling;
            }
        }

        if (numClose > 0) {
            avgVX /= numClose;
            avgVY /= numClose;
            avgX /= numClose;
            avgY /= numClose;

            glm::vec2 avgVel(avgVX, avgVY);
            glm::vec2 avgPos(avgX, avgY);
            // Incorporate avg velocity
            glm::vec2 dir = glm::normalize(avgVel - b->GetVelocity());
            forceAlign = dir;

            // Incorporate average flock position

            dir = glm::normalize(avgPos - myPos);
            float dist = glm::distance(myPos, avgPos);
            float scaling = (1.0f / (dist));
            forcePos = dir * scaling;
        }

        // Average forces
        glm::vec2 force = COLLISION_WEIGHT * forceCollision + ALIGN_WEIGHT * forceAlign + POSITION_WEIGHT * forcePos;
        force = glm::vec2(SIGN(force.x) * std::min(std::abs(force.x), FORCE_CAP), SIGN(force.y) * std::min(std::abs(force.y), FORCE_CAP));

        /*
        // Find nearest neighbors
        int xBox = (int) (b->GetX() / MAP_SQUARES_X);
        int yBox = (int) (b->GetY() / MAP_SQUARES_Y);
        for (int i = -1; i <= 1; i++) {
            for (int j = -1; j <= 1; j++) {
                if (xBox + i < 0 || xBox + i >= this->Width / MAP_SQUARES_X
                    || yBox + j < 0 || yBox + j >= this->Height / MAP_SQUARES_Y) {
                    continue;
                }
                for (std::pair<long,Boid*> other : boidMap[xBox + i][yBox + j]) {
                    glm::vec2 otherPos(other.second->GetX(), other.second->GetY());
                    glm::vec2 dir = otherPos - myPos;
                    force += -dir;
                }
            }
        }*/
        forces[b] = force;
    }
    for (Boid *b : this->boids) {
        //int x = b->GetX();
        //int y = b->GetY();
        // Remove from old square
        //assert(boidMap[(int)(x / MAP_SQUARES_X)][(int)(y / MAP_SQUARES_Y)].erase((long)b) != 0);

        b->Update(forces[b], dt);

        // Add to new square
        //x = b->GetX(); y = b->GetY();
        //boidMap[(int)(x / MAP_SQUARES_X)][(int)(y / MAP_SQUARES_Y)][(long)b] = b;
    }
}
void State::Render() {
    for (Boid *b : this->boids) {
        b->Draw(Renderer);
    }
}

State::~State() {
    delete Renderer;
}
