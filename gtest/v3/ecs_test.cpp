#include "../../src/v3/ecs.hpp"

#include <gtest/gtest.h>

TEST(Testing, createEntityWithoutComponents) {
    ecs::createEntityWithComponents<>();
    int i = ecs::archetypes[0].entities.size();
    EXPECT_EQ(1, i);
}

TEST(Testing, createEntity) {
    ecs::createEntity();
    int i = ecs::archetypes[0].entities.size();
    EXPECT_EQ(1, i);  // should be 2
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

TEST(Testing, createMultipleEntity) {
    ecs::createEntityWithComponents<Position, Velocity>(Position{0, 0}, Velocity{0, 0});
    ecs::createEntityWithComponents<Position, Velocity, Health>(Position{0, 0}, Velocity{0, 0},
                                                                Health{0});
    ecs::createEntityWithComponents<Position, Velocity, EnemyTag>(Position{0, 0}, Velocity{0, 0},
                                                                  EnemyTag{});
    ecs::createEntityWithComponents<EnemyTag>(EnemyTag{});

    int entityWithPositions = 0;
    ecs::forEach<Position>([&entityWithPositions](Position& pos) { entityWithPositions++; });
    EXPECT_EQ(3, entityWithPositions);

    int entityWithEnemyTag = 0;
    ecs::forEach<EnemyTag>([&entityWithEnemyTag](EnemyTag& pos) { entityWithEnemyTag++; });
    EXPECT_EQ(2, entityWithEnemyTag);
}