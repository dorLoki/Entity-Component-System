#pragma once
#include <functional>

namespace ecs {

inline int nextEntityId = 0;

struct Entity {
    int id;
};

template <typename T>
struct ComponentStorage {
    std::vector<T> components;  // Komponente[n]
    std::vector<int> dense;     // entityIds[i] gehört zu components[i]
    std::vector<int> sparse;    // entityId → index in dense/comp, oder -1

    ComponentStorage() {
        sparse.resize(1000, -1);  // maxEntityCount z. B. 100k
    }

    void add(int entityId, const T& component) {
        if (sparse[entityId] != -1) return;  // schon vorhanden
        sparse[entityId] = dense.size();
        dense.push_back(entityId);
        components.push_back(component);
    }

    void remove(int entityId) {
        int index = sparse[entityId];
        if (index == -1) return;

        int last = dense.size() - 1;

        std::swap(dense[index], dense[last]);
        std::swap(components[index], components[last]);

        sparse[dense[index]] = index;
        sparse[entityId] = -1;

        dense.pop_back();
        components.pop_back();
    }

    T* get(int entityId) {
        int index = sparse[entityId];
        return (index == -1) ? nullptr : &components[index];
    }

    std::vector<T>& getAllComponents() { return components; }
    std::vector<int>& getAllEntities() { return dense; }
};

template <typename T>
ComponentStorage<T>& getStorage() {
    static ComponentStorage<T> storage;
    return storage;
}

template <typename T>
void addComponent(Entity e, const T& comp) {
    getStorage<T>().add(e.id, comp);
}

template <typename T>
T* getComponent(Entity e) {
    return getStorage<T>().get(e.id);
}

template <typename T>
bool hasComponent(Entity e) {
    return getStorage<T>().get(e.id) != nullptr;
}

Entity createEntity() { return Entity{nextEntityId++}; }

inline std::vector<std::function<void()>> systems;

void addSystem(const std::function<void()>& func) { systems.push_back(func); }

void tick() {
    for (auto& sys : systems) sys();
}

}  // namespace ecs