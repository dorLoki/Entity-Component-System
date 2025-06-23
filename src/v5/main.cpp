#include "ecs.hpp"

struct Position {};
struct Velocity {};
struct Health {};

struct MyECSConfig {
    using ComponentList = std::tuple<Position, Velocity>;
};

int main() {
    using MyECS = ecs::ComponentManager<MyECSConfig>;

    static_assert(MyECS::GetComponentMask<Velocity>() == 0b10);
    static_assert(std::is_same_v<MyECS::ComponentType<0>, Position>);
    static_assert(std::is_same_v<MyECS::ComponentType<1>, Velocity>);

    auto i = MyECS::ComponentType<1>();
    auto j = MyECS::GetComponentID<Position>();

    return 0;
}