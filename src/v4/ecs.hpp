#pragma once
#include <algorithm>
#include <memory>
#include <stdexcept>
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

ArchetypeSignature intersectionOfArchetypeSignature(const ArchetypeSignature& sig,
                                                    const ArchetypeSignature& query) {
    ArchetypeSignature intersection;
    std::set_intersection(sig.begin(), sig.end(), query.begin(), query.end(),
                          std::back_inserter(intersection));
    return intersection;
}

struct IComponentArray {
    virtual ~IComponentArray() = default;
    virtual void copyElementFrom(IComponentArray* source, size_t sourceIndex) = 0;
    virtual void moveElement(size_t fromIndex, size_t toIndex) = 0;
    virtual void removeLast() = 0;
};
template <typename T>
struct ComponentArray : IComponentArray {
    std::vector<T> data;

    void push_back(const T& value) { data.push_back(value); }

    T& get(size_t index) { return data[index]; }

    std::vector<T>& getVector() { return data; }

    void copyElementFrom(IComponentArray* source, size_t sourceIndex) override {
        auto* src = static_cast<ComponentArray<T>*>(source);
        data.push_back(src->get(sourceIndex));
    }

    void moveElement(size_t fromIndex, size_t toIndex) override {
        data[toIndex] = std::move(data[fromIndex]);
    }

    void removeLast() override { data.pop_back(); }
};

struct Archetype {
    ArchetypeSignature signature;
    std::vector<EntityId> entities;
    std::unordered_map<ComponentId, std::unique_ptr<IComponentArray>> componentData;

    Archetype() = default;
    explicit Archetype(ArchetypeSignature sig) : signature(std::move(sig)) {}

    // uncopyble, because componentData contains unique ptr
    Archetype(const Archetype&) = delete;
    Archetype& operator=(const Archetype&) = delete;
    // movable
    Archetype(Archetype&&) noexcept = default;
    Archetype& operator=(Archetype&&) noexcept = default;

    template <typename T>
    ComponentArray<T>* getOrCreateComponentArray() {
        ComponentId id = getComponentId<T>();
        auto it = componentData.find(id);
        if (it == componentData.end()) {
            auto array = std::make_unique<ComponentArray<T>>();
            ComponentArray<T>* ptr = array.get();
            componentData[id] = std::move(array);
            return ptr;
        }
        return static_cast<ComponentArray<T>*>(componentData[id].get());
    }
};

struct EntityLocation {
    Archetype* archetype;
    size_t indexInArchetype;
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
        size_t index = archetype->entities.size() - 1;

        (archetype->getOrCreateComponentArray<std::decay_t<Components>>()->push_back(
             std::forward<Components>(components)),
         ...);

        entityLocationMap[id] = {archetype, index};
        return id;
    }

    template <typename... Components>
    EntityId addComponent(EntityId entityId, Components&&... components) {
        // look up the entity
        auto it = entityLocationMap.find(entityId);
        if (it == entityLocationMap.end()) throw std::out_of_range("Entity not found.");

        Archetype* arch = it->second.archetype;
        size_t oldIndex = it->second.indexInArchetype;

        // check if the entity already has the given component
        ArchetypeSignature query = {getComponentId<Components>()...};
        std::sort(query.begin(), query.end());
        if (intersectionOfArchetypeSignature(arch->signature, query).size() > 0)
            throw std::runtime_error("Entity already has one of the given components.");

        // new signature
        ArchetypeSignature newSig = arch->signature;
        newSig.insert(newSig.end(), query.begin(), query.end());
        std::sort(newSig.begin(), newSig.end());

        // add entity to new archetype
        Archetype* newArch = getOrCreateArchetype(newSig);
        newArch->entities.push_back(entityId);
        size_t newIndex = newArch->entities.size() - 1;

        // copy old data
        for (auto componentId : arch->signature) {
            auto* source = arch->componentData.at(componentId).get();
            // auto* target = newArch->getOrCreateComponentArray();
        }

        // todo

        return entityId;
    }

    template <typename... Components, typename Func>
    void apply(EntityId entityId, Func func) {
        auto it = entityLocationMap.find(entityId);
        if (it == entityLocationMap.end()) throw std::out_of_range("Entity not found.");
        Archetype* arch = it->second.archetype;

        ArchetypeSignature query = {getComponentId<Components>()...};
        std::sort(query.begin(), query.end());

        if (!matchArchetypeSignature(arch->signature, query))
            throw std::runtime_error("Entity does not contain the given Component.");

        size_t index = it->second.indexInArchetype;
        func((arch->getOrCreateComponentArray<Components>()->get(index))...);
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
    std::vector<Archetype> archetypes{};
    EntityId nextEntityId = 0;
    std::unordered_map<EntityId, EntityLocation> entityLocationMap{};

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
