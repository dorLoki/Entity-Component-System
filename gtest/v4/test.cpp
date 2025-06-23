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

TEST(V4, applyMethod) {
    ecs::World world{};
    auto id = world.createEntity<Position, Velocity>(Position{0, 0}, Velocity{0, 0});
    world.apply<Position>(id, [](Position& pos) { pos.x = 100; });
}

TEST(V4, apply) {
    EXPECT_THROW(
        {
            ecs::World world{};
            auto id = world.createEntity<Velocity>(Velocity{0, 0});
            world.apply<Position>(id, [](Position& pos) { pos.x = 100; });
        },
        std::runtime_error);
}

TEST(V4, applyFail) {
    EXPECT_THROW(
        {
            ecs::World world{};
            auto id = world.createEntity<Velocity>(Velocity{0, 0});
            world.apply<Position>(id, [](Position& pos) { pos.x = 100; });
        },
        std::runtime_error);
}

TEST(V4, applyFailOutOfBounds) {
    EXPECT_THROW(
        {
            ecs::World world{};
            world.apply<Position>(1, [](Position& pos) { pos.x = 100; });
        },
        std::out_of_range);
}

TEST(V4, addComponent) {
    ecs::World world{};
    world.createEntity<Velocity>(Velocity{0, 0});
    world.createEntity<Velocity>(Velocity{0, 0});
    auto id = world.createEntity<Position>(Position{0, 0});
    world.createEntity<Velocity>(Velocity{0, 0});
    world.createEntity<Velocity>(Velocity{0, 0});

    world.addComponent<Velocity>(id, Velocity{0, 0});

    int entityWithVelocity = 0;
    world.apply<Velocity>(id, [&entityWithVelocity](Velocity& pos) { entityWithVelocity++; });
    EXPECT_EQ(5, entityWithVelocity);
}
