#pragma once
#include <algorithm>
#include <memory>
#include <unordered_map>
#include <vector>

namespace ecs {

using EntityId = unsigned int;

namespace {
using ComponentId = unsigned int;
using ArchetypeSignature = std::vector<ComponentId>;
inline ComponentId nextComponentId = 0;
template <typename T>
ComponentId getComponentId() {
    static int id = nextComponentId++;
    return id;
}

bool matchArchetypeSignature(const ArchetypeSignature& sig, const ArchetypeSignature& query) {
    return std::includes(sig.begin(), sig.end(), query.begin(), query.end());
}

struct IComponentArray {
    virtual ~IComponentArray() = default;
};
template <typename T>
struct ComponentArray : IComponentArray {
    std::vector<T> data;

    void push_back(const T& value) { data.push_back(value); }

    T& get(size_t index) { return data[index]; }

    std::vector<T>& getVector() { return data; }
};

struct Archetype {
    ArchetypeSignature signature;
    std::vector<EntityId> entities;
    std::unordered_map<ComponentId, std::shared_ptr<IComponentArray>> componentData;

    template <typename T>
    ComponentArray<T>* getOrCreateComponentArray() {
        ComponentId id = getComponentId<T>();
        auto it = componentData.find(id);
        if (it == componentData.end()) {
            auto array = std::make_shared<ComponentArray<T>>();
            ComponentArray<T>* ptr = array.get();
            componentData[id] = std::move(array);
            return ptr;
        }
        return static_cast<ComponentArray<T>*>(componentData[id].get());
    }
};
}  // namespace

class World {
   public:
    template <typename... Components>
    EntityId createEntity(Components&&... components) {
        EntityId id = generateEntityId();
        ArchetypeSignature sig = {getComponentId<std::decay_t<Components>>()...};
        std::sort(sig.begin(), sig.end());

        Archetype* archetype = getOrCreateArchetype(sig);
        archetype->entities.push_back(id);

        (archetype->getOrCreateComponentArray<std::decay_t<Components>>()->push_back(
             std::forward<Components>(components)),
         ...);
        return id;
    }

    template <typename... Components, typename Func>
    void forEach(Func func) {
        ArchetypeSignature query = {getComponentId<Components>()...};
        std::sort(query.begin(), query.end());

        for (auto& arch : archetypes) {
            if (!matchArchetypeSignature(arch.signature, query)) continue;

            size_t count = arch.entities.size();
            auto comps = std::make_tuple(arch.getOrCreateComponentArray<Components>()...);

            for (size_t i = 0; i < count; ++i) {
                std::apply([&](auto*... arrays) { func((arrays->get(i))...); }, comps);
            }
        }
    }

   private:
    std::vector<Archetype> archetypes;
    EntityId nextEntityId = 0;
    EntityId generateEntityId() { return nextEntityId++; }
    Archetype* getOrCreateArchetype(const ArchetypeSignature& sig) {
        for (auto& a : archetypes) {
            if (a.signature == sig) return &a;
        }
        archetypes.push_back(Archetype{sig});
        return &archetypes.back();
    }
};

}  // namespace ecs
