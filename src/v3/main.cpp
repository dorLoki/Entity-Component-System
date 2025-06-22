#include <array>
#include <chrono>
#include <iostream>
#include <random>

#include "ecs.hpp"

struct Player {};
struct Enemy {};

struct Coordinates {
    double x, y;
};

struct Velocity {
    double xVel, yVel;
};

double getRandom() { return (static_cast<double>(rand() % 1000 + 1)) / 100.0; }

const size_t entity_count = 1000;
const int tick_amount = 1000;

void mainEcs() {
    // random for value generation
    std::random_device rd;
    std::mt19937 rng(rd());

    // init entity list
    for (size_t i = 0; i < entity_count; i++) {
        ecs::createEntityWithComponents<Coordinates, Velocity>(
            Coordinates{getRandom(), getRandom()}, Velocity{getRandom(), getRandom()});
    }

    // tick n times and mess time
    auto startTime = std::chrono::high_resolution_clock::now();
    for (size_t i = 0; i < tick_amount; i++) {
        ecs::forEach<Coordinates, Velocity>([](Coordinates& coords, Velocity& vel) {
            coords.x += vel.xVel;
            coords.y += vel.yVel;
        });
    }
    auto endTime = std::chrono::high_resolution_clock::now();
    auto time = endTime - startTime;
    // ecs::forEach<Coordinates, Velocity>([](Coordinates& coords, Velocity& vel) {
    //     std::cout << coords.x << ",\t" << coords.y << '\n';
    // });

    std::cout << "tick time ecs: "
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
    std::array<PlayerOOP, entity_count> entity_list{};

    auto startTime = std::chrono::high_resolution_clock::now();
    // tick 1000 times
    for (size_t i = 0; i < tick_amount; i++) {
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
    std::cout << "tick time oop: "
              << std::chrono::duration_cast<std::chrono::microseconds>(time).count() << std::endl;
}

int main() {
    mainEcs();
    mainOop();
    return 0;
}