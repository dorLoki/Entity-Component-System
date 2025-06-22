#include <array>
#include <chrono>
#include <iostream>
#include <random>

#include "v1/ecs.hpp"

struct Player {};
struct Enemy {};

struct Coordinates {
    double x, y;
};

struct Velocity {
    double xVel, yVel;
};

double getRandom() { return (static_cast<double>(rand() % 1000 + 1)) / 100.0; }

void mainEcs() {
    // entity list
    const size_t entity_count = 1000;
    std::array<ecs::Entity, entity_count> entity_list{};

    // random for value generation
    std::random_device rd;
    std::mt19937 rng(rd());

    // init entity list
    for (size_t i = 0; i < entity_count; i++) {
        entity_list[i] = ecs::createEntity<Player>();
        ecs::addComponent(entity_list[i], Coordinates{getRandom(), getRandom()});
        ecs::addComponent(entity_list[i], Velocity{getRandom(), getRandom()});
    }
    // add position system
    ecs::addSystem([]() {
        auto& coords = ecs::getComponentList<Coordinates>();
        const auto vel = ecs::getComponentList<Velocity>();
        for (int i = 0; i < ECS_MAX_ENTITIES; i++) {
            coords[i].x += vel[i].xVel;
            coords[i].y += vel[i].yVel;
        }
    });
    // tick n times and mess time
    auto startTime = std::chrono::high_resolution_clock::now();
    for (size_t i = 0; i < 1000; i++) {
        ecs::tick();
    }
    auto endTime = std::chrono::high_resolution_clock::now();
    auto time = endTime - startTime;

    // output data
    auto& coordinates = ecs::getComponentList<Coordinates>();
    for (size_t i = 0; i < entity_count; i++) {
        // std::cout << i << '\t' << coordinates[i].x << ' ' << coordinates[i].y << '\n';
    }
    std::cout << "tick time: "
              << std::chrono::duration_cast<std::chrono::microseconds>(time).count() << std::endl;
}

class PlayerOOP {
   public:
    PlayerOOP() {
        this->x = getRandom();
        this->y = getRandom();
        this->xVel = getRandom();
        this->yVel = getRandom();
    };
    ~PlayerOOP() = default;
    void move() {
        x += xVel;
        y += yVel;
    }
    void toString(int i) { std::cout << i << '\t' << x << ' ' << y << '\n'; }

   private:
    double x, y;
    double xVel, yVel;
};

void mainOop() {
    // entity list
    const size_t entity_count = 1000;
    std::array<PlayerOOP, entity_count> entity_list{};

    auto startTime = std::chrono::high_resolution_clock::now();
    // tick 1000 times
    for (size_t i = 0; i < 1000; i++) {
        // system
        for (size_t i = 0; i < entity_count; i++) {
            entity_list[i].move();
        }
    }
    auto endTime = std::chrono::high_resolution_clock::now();
    auto time = endTime - startTime;
    for (size_t i = 0; i < entity_count; i++) {
        // entity_list[i].toString(static_cast<int>(i));
    }
    std::cout << "tick time: "
              << std::chrono::duration_cast<std::chrono::microseconds>(time).count() << std::endl;
}

int main() {
    mainEcs();
    mainOop();
    return 0;
}