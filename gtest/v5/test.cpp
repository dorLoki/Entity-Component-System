#include <gtest/gtest.h>

#include "../../src/v5/ecs.hpp"

struct Position {};
struct Velocity {};
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