#pragma once
#include <memory>
#include <tuple>
#include <unordered_map>
#include <vector>

namespace ecs {

// define types for clearer parameter
using EntityId = unsigned int;

// ComponentManager template.
// Accepts a user-defined configuration that provides a compile-time ComponentList.
template <typename UserConfig>
struct ComponentManager {
    // This is the user-defined list of all component types.
    // Example: using ComponentList = std::tuple<Position, Velocity, Health>;
    using ComponentList = typename UserConfig::ComponentList;

    // Helper metafunction to compute the index of a type T within a std::tuple.
    // Used to map a type to its position in ComponentList.
    template <typename T, typename Tuple>
    struct IndexInTuple;

    // Base case: T is the first type in the tuple -> index = 0
    template <typename T, typename... Types>
    struct IndexInTuple<T, std::tuple<T, Types...>> {
        static constexpr std::size_t value = 0;
    };

    // Recursive case: T is not the first -> recurse on the rest of the tuple,
    // and increment index by 1 at each level.
    template <typename T, typename U, typename... Types>
    struct IndexInTuple<T, std::tuple<U, Types...>> {
        static constexpr std::size_t value = 1 + IndexInTuple<T, std::tuple<Types...>>::value;
    };

    // Returns the unique component ID for type T.
    template <typename T>
    static constexpr std::size_t GetComponentID() {
        return IndexInTuple<T, ComponentList>::value;
    }

    // Returns the unique component bitmask for type T.
    // Internally shifts 1 by the index of T in the ComponentList.
    // E.g.: if T is at index 2 -> result is 1 << 2 = 0b0100
    template <typename T>
    static constexpr std::size_t GetComponentMask() {
        return 1 << GetComponentID<T>();
    }

    // Maps a component index back to its type.
    // E.g.: ComponentType<1> gives you the second component type in ComponentList.
    template <std::size_t ID>
    using ComponentType = std::tuple_element_t<ID, ComponentList>;
};

namespace detail {
// define types for clearer parameter
using ComponentId = size_t;
using ComponentMask = size_t;
using ArchetypeSignature = size_t;

// check if the signature has all components of the query
inline bool matchArchetypeSignatures(const ArchetypeSignature sig, const ArchetypeSignature query) {
    return (sig & query) == query;
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

    template <typename T, typename ComponentManager>
    ComponentArray<T>* getOrCreateComponentArray() {
        ComponentId id = ComponentManager::template GetComponentID<T>();
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
}  // namespace detail

template <typename ComponentManager>
class World {
   public:
    template <typename... Components>
    EntityId createEntity(Components&&... components) {
        EntityId id = generateEntityId();
        detail::ArchetypeSignature sig =
            (ComponentManager::template GetComponentMask<std::decay_t<Components>>() | ...);

        detail::Archetype* archetype = getOrCreateArchetype(sig);
        archetype->entities.push_back(id);
        size_t index = archetype->entities.size() - 1;

        (archetype->getOrCreateComponentArray<std::decay_t<Components>, ComponentManager>()
             ->push_back(std::forward<Components>(components)),
         ...);

        entityLocationMap[id] = {archetype->signature, index};
        return id;
    }

    template <typename... Components, typename Func>
    void apply(EntityId entityId, Func func) {
        auto it = entityLocationMap.find(entityId);
        if (it == entityLocationMap.end()) throw std::out_of_range("Entity not found.");
        auto signature = it->second.signature;
        detail::Archetype* arch = getOrCreateArchetype(signature);

        detail::ArchetypeSignature query =
            (ComponentManager::template GetComponentMask<std::decay_t<Components>>() | ...);

        if (!detail::matchArchetypeSignatures(arch->signature, query))
            throw std::runtime_error("Entity does not contain the given Component.");

        size_t index = it->second.indexInArchetype;

        func((arch->getOrCreateComponentArray<std::decay_t<Components>, ComponentManager>()->get(
            index))...);
    }

   private:
    std::vector<detail::Archetype> archetypes{};
    std::unordered_map<EntityId, detail::EntityLocation> entityLocationMap{};
    EntityId generateEntityId() {
        static int nextEntityId = 0;
        return nextEntityId++;
    }
    detail::Archetype* getOrCreateArchetype(const detail::ArchetypeSignature& sig) {
        for (auto& a : archetypes) {
            if (a.signature == sig) return &a;
        }
        archetypes.push_back(detail::Archetype{sig});
        return &archetypes.back();
    }
};
}  // namespace ecs