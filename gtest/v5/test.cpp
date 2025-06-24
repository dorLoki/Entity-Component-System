#include <gtest/gtest.h>

#include "../../src/v5/ecs.hpp"

struct Position {
    int x, y;
};
struct Velocity {
    int dx, dy;
};
struct Health {};

struct MyECSConfig {
    using ComponentList = std::tuple<Position, Velocity>;
};

using MyECS = ecs::ComponentManager<MyECSConfig>;

TEST(V5, testComponentId) {
    EXPECT_EQ(0, MyECS::GetComponentID<Position>());
    EXPECT_EQ(1, MyECS::GetComponentID<Velocity>());
}

TEST(V5, testComponentMask) {
    EXPECT_EQ(0b01, MyECS::GetComponentMask<Position>());
    EXPECT_EQ(0b10, MyECS::GetComponentMask<Velocity>());
}

TEST(V5, testComponentType) {
    EXPECT_TRUE((std::is_same_v<MyECS::ComponentType<0>, Position>));
    EXPECT_TRUE((std::is_same_v<MyECS::ComponentType<1>, Velocity>));
}

TEST(V5, testMatchArchetypeSignature) {
    ecs::detail::ArchetypeSignature sig = 0b1;
    ecs::detail::ArchetypeSignature query = 0b1;
    EXPECT_TRUE(ecs::detail::matchArchetypeSignatures(sig, query));

    sig = 0b11;
    query = 0b01;
    EXPECT_TRUE(ecs::detail::matchArchetypeSignatures(sig, query));

    sig = 0b0;
    query = 0b1;
    EXPECT_FALSE(ecs::detail::matchArchetypeSignatures(sig, query));

    sig = 0b10;
    query = 0b01;
    EXPECT_FALSE(ecs::detail::matchArchetypeSignatures(sig, query));
}

TEST(V5, testCreateArchetype) {
    ecs::World<MyECS> world;
    world.createEntity<Position>(Position{1, 2});
    world.createEntity<Velocity>(Velocity{3, 4});
    world.createEntity<Position, Velocity>(Position{5, 6}, Velocity{7, 8});
    world.createEntity<Position, Velocity>(Position{10, 11}, Velocity{12, 13});
}