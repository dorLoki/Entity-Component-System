#pragma once
#include <array>
#include <functional>
#include <typeindex>
#include <unordered_map>
#include <vector>

#ifndef ECS_MAX_ENTITIES
#define ECS_MAX_ENTITIES 1000
#endif

namespace ecs {

inline int nextEntityId = 0;

/**
 * The components are mapped by there ID to an array.
 */
template <typename T>
inline std::unordered_map<int, std::array<T, ECS_MAX_ENTITIES>> components;

/**
 * Retrieve the id of a struct.
 */
template <typename T>
int getTypeId() {
    return static_cast<int>(std::type_index(typeid(T)).hash_code());
}

inline std::vector<std::function<void()>> systems;

struct Entity {
    int id;
};

template <typename T>
Entity createEntity() {
    return Entity{nextEntityId++};
}

template <typename T>
void addComponent(Entity entity, T component) {
    auto hashId = getTypeId<T>();
    auto entityId = entity.id;
    components<T>[hashId][entityId] = component;
}

template <typename T>
std::array<T, ECS_MAX_ENTITIES>& getComponentList() {
    auto hashId = getTypeId<T>();
    return components<T>[hashId];
}

void addSystem(std::function<void()> func) { systems.push_back(func); }

void tick() {
    for (const auto& f : systems) {
        f();
    }
}

}  // namespace ecs