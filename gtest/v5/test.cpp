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

TEST(V5, testApply) {
    ecs::World<MyECS> world;
    auto e1 = world.createEntity<Position>(Position{1, 2});
    world.apply<Position>(e1, [](Position& pos) { pos.x = 100; });
    world.apply<Position>(e1, [](Position& pos) {
        EXPECT_EQ(100, pos.x);
        EXPECT_EQ(2, pos.y);
    });
}

TEST(V5, testApplyFail) {
    EXPECT_THROW(
        {
            ecs::World<MyECS> world;
            auto id = world.createEntity<Velocity>(Velocity{0, 0});
            world.apply<Position>(id, [](Position& pos) { pos.x = 100; });
        },
        std::runtime_error);
}

TEST(V5, testApplyFailOutOfBounds) {
    EXPECT_THROW(
        {
            ecs::World<MyECS> world;
            world.apply<Position>(1, [](Position& pos) { pos.x = 100; });
        },
        std::out_of_range);
}

TEST(V5, testForeach) {
    ecs::World<MyECS> world;
    auto e1 = world.createEntity<Position>(Position{1, 2});
    auto e2 = world.createEntity<Position>(Position{3, 4});
    world.forEach<Position>([](Position& pos) { pos.x = 100; });
    world.apply<Position>(e1, [](Position& pos) { EXPECT_EQ(100, pos.x); });
    world.apply<Position>(e2, [](Position& pos) { EXPECT_EQ(100, pos.x); });
}

TEST(V5, testForeachMultipleArches) {
    ecs::World<MyECS> world;
    auto e1 = world.createEntity<Position>(Position{11, 22});
    auto e2 = world.createEntity<Position, Velocity>(Position{33, 44}, Velocity{55, 66});
    world.forEach<Position>([](Position& pos) { pos.x = 100; });
    world.apply<Position>(e1, [](Position& pos) { EXPECT_EQ(100, pos.x); });
    world.apply<Position>(e2, [](Position& pos) { EXPECT_EQ(100, pos.x); });
}

TEST(V5, testAddComponent) {
    ecs::World<MyECS> world;
    auto e1 = world.createEntity<Position>(Position{11, 22});
    auto e2 = world.createEntity<Position, Velocity>(Position{33, 44}, Velocity{55, 66});
    world.addComponent<Position, Velocity>(e1, Velocity{77, 88});

    world.apply<Position, Velocity>(e1, [](Position& pos, Velocity& vel) {
        EXPECT_EQ(11, pos.x);
        EXPECT_EQ(22, pos.y);
        EXPECT_EQ(77, vel.dx);
        EXPECT_EQ(88, vel.dy);
    });
    world.apply<Position, Velocity>(e2, [](Position& pos, Velocity& vel) {
        EXPECT_EQ(33, pos.x);
        EXPECT_EQ(44, pos.y);
        EXPECT_EQ(55, vel.dx);
        EXPECT_EQ(66, vel.dy);
    });
}

TEST(V5, testAddComponentDeleteOldData) {
    ecs::World<MyECS> world;
    auto e1 = world.createEntity<Position>(Position{1, 1});
    auto e2 = world.createEntity<Position>(Position{2, 2});
    auto e3 = world.createEntity<Position>(Position{3, 3});
    auto e4 = world.createEntity<Position>(Position{4, 4});
    auto e5 = world.createEntity<Position>(Position{5, 5});

    auto e6 = world.createEntity<Position, Velocity>(Position{6, 6}, Velocity{7, 7});
    auto e7 = world.createEntity<Position, Velocity>(Position{8, 8}, Velocity{9, 9});
    auto e8 = world.createEntity<Position, Velocity>(Position{10, 10}, Velocity{11, 11});
    auto e9 = world.createEntity<Position, Velocity>(Position{12, 12}, Velocity{13, 13});
    auto e10 = world.createEntity<Position, Velocity>(Position{14, 14}, Velocity{15, 15});

    world.addComponent<Position, Velocity>(e3, Velocity{16, 16});

    world.apply<Position>(e1, [](Position& pos) {
        EXPECT_EQ(1, pos.x);
        EXPECT_EQ(1, pos.y);
    });

    world.apply<Position>(e2, [](Position& pos) {
        EXPECT_EQ(2, pos.x);
        EXPECT_EQ(2, pos.y);
    });

    world.apply<Position, Velocity>(e3, [](Position& pos, Velocity& vel) {
        EXPECT_EQ(3, pos.x);
        EXPECT_EQ(3, pos.y);
        EXPECT_EQ(16, vel.dx);
        EXPECT_EQ(16, vel.dy);
    });

    world.apply<Position>(e4, [](Position& pos) {
        EXPECT_EQ(4, pos.x);
        EXPECT_EQ(4, pos.y);
    });

    world.apply<Position>(e5, [](Position& pos) {
        EXPECT_EQ(5, pos.x);
        EXPECT_EQ(5, pos.y);
    });

    world.apply<Position, Velocity>(e6, [](Position& pos, Velocity& vel) {
        EXPECT_EQ(6, pos.x);
        EXPECT_EQ(6, pos.y);
        EXPECT_EQ(7, vel.dx);
        EXPECT_EQ(7, vel.dy);
    });

    world.apply<Position, Velocity>(e7, [](Position& pos, Velocity& vel) {
        EXPECT_EQ(8, pos.x);
        EXPECT_EQ(8, pos.y);
        EXPECT_EQ(9, vel.dx);
        EXPECT_EQ(9, vel.dy);
    });

    world.apply<Position, Velocity>(e8, [](Position& pos, Velocity& vel) {
        EXPECT_EQ(10, pos.x);
        EXPECT_EQ(10, pos.y);
        EXPECT_EQ(11, vel.dx);
        EXPECT_EQ(11, vel.dy);
    });

    world.apply<Position, Velocity>(e9, [](Position& pos, Velocity& vel) {
        EXPECT_EQ(12, pos.x);
        EXPECT_EQ(12, pos.y);
        EXPECT_EQ(13, vel.dx);
        EXPECT_EQ(13, vel.dy);
    });

    world.apply<Position, Velocity>(e10, [](Position& pos, Velocity& vel) {
        EXPECT_EQ(14, pos.x);
        EXPECT_EQ(14, pos.y);
        EXPECT_EQ(15, vel.dx);
        EXPECT_EQ(15, vel.dy);
    });
}