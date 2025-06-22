#include <gtest/gtest.h>

#include "../../src/v4/ecs.hpp"

TEST(V4, createEntityWithoutComponents) {
    ecs::World world{};
    int id = world.createEntity<>();
    EXPECT_EQ(0, id);
}

struct Position {
    int x, y;
};

struct Velocity {
    int dx, dy;
};

struct Health {
    int health;
};

struct EnemyTag {};

TEST(V4, createMultipleEntity) {
    ecs::World world{};
    world.createEntity<Position, Velocity>(Position{0, 0}, Velocity{0, 0});
    world.createEntity<Position, Velocity, Health>(Position{0, 0}, Velocity{0, 0}, Health{0});
    world.createEntity<Position, Velocity, EnemyTag>(Position{0, 0}, Velocity{0, 0}, EnemyTag{});
    world.createEntity<EnemyTag>(EnemyTag{});

    int entityWithPositions = 0;
    world.forEach<Position>([&entityWithPositions](Position& pos) { entityWithPositions++; });
    EXPECT_EQ(3, entityWithPositions);

    int entityWithEnemyTag = 0;
    world.forEach<EnemyTag>([&entityWithEnemyTag](EnemyTag& pos) { entityWithEnemyTag++; });
    EXPECT_EQ(2, entityWithEnemyTag);
}