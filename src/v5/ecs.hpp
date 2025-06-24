#pragma once
#include <memory>
#include <tuple>
#include <unordered_map>
#include <vector>

namespace ecs {

// Define types for clearer parameters
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
// Define types for clearer parameters
using ComponentId = size_t;
using ComponentMask = size_t;
using ArchetypeSignature = size_t;

// Check if the signature has all components of the query.
inline bool matchArchetypeSignatures(const ArchetypeSignature sig, const ArchetypeSignature query) {
    return (sig & query) == query;
}

// Base interface for component arrays, allowing polymorphic behavior.
struct IComponentArray {
    virtual ~IComponentArray() = default;
    virtual void copyElementFrom(IComponentArray* source, size_t sourceIndex) = 0;
    virtual void moveElement(size_t fromIndex, size_t toIndex) = 0;
    virtual void removeLast() = 0;
};

// A generic component array that stores the actual components (data).
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

// Archetype stores entities and their component arrays.
struct Archetype {
    ArchetypeSignature signature;
    std::vector<EntityId> entities;
    std::unordered_map<ComponentId, std::unique_ptr<IComponentArray>> componentData;

    Archetype() = default;
    explicit Archetype(ArchetypeSignature sig) : signature(std::move(sig)) {}

    // Disable copy, as componentData contains unique pointers
    Archetype(const Archetype&) = delete;
    Archetype& operator=(const Archetype&) = delete;

    // Allow move operations
    Archetype(Archetype&&) noexcept = default;
    Archetype& operator=(Archetype&&) noexcept = default;

    // Creates or retrieves a ComponentArray for a given component type T.
    // Needs the ComponentManager to convert the type T to an ID.
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

// EntityLocation stores the archetype signature and index of an entity.
// Used to map every entity to it's corresponding archetype plus the location of it's data in the
// tables of components
struct EntityLocation {
    detail::ArchetypeSignature signature;
    size_t indexInArchetype;
};
}  // namespace detail

// The main World class holds all entities, archetypes, and manages their interactions.
// World needs all used Components at compile-time via the ComponentManager.
template <typename ComponentManager>
class World {
   public:
    template <typename... Components>
    // Creates an entity with the specified components
    EntityId createEntity(Components&&... components) {
        EntityId id = generateEntityId();
        detail::ArchetypeSignature sig =
            (ComponentManager::template GetComponentMask<std::decay_t<Components>>() | ...);

        detail::Archetype* archetype = getOrCreateArchetype(sig);
        archetype->entities.push_back(id);
        size_t index = archetype->entities.size() - 1;

        // Add components to the archetype's component arrays
        (archetype->getOrCreateComponentArray<std::decay_t<Components>, ComponentManager>()
             ->push_back(std::forward<Components>(components)),
         ...);

        entityLocationMap[id] = {archetype->signature, index};
        return id;
    }

    // Applies a function to an entity
    template <typename... Components, typename Func>
    void apply(EntityId entityId, Func func) {
        auto it = entityLocationMap.find(entityId);
        if (it == entityLocationMap.end()) throw std::out_of_range("Entity not found.");
        auto signature = it->second.signature;
        detail::Archetype* arch = getOrCreateArchetype(signature);

        // Build the query signature from the components
        detail::ArchetypeSignature query =
            (ComponentManager::template GetComponentMask<std::decay_t<Components>>() | ...);

        // Check if the archetype matches the component signature
        if (!detail::matchArchetypeSignatures(arch->signature, query))
            throw std::runtime_error("Entity does not contain the given Component.");

        size_t index = it->second.indexInArchetype;

        // Apply the function to the entity's components
        func((arch->getOrCreateComponentArray<std::decay_t<Components>, ComponentManager>()->get(
            index))...);
    }

    // Applies a function to each entity that matches the specified components.
    template <typename... Components, typename Func>
    void forEach(Func func) {
        detail::ArchetypeSignature query =
            (ComponentManager::template GetComponentMask<std::decay_t<Components>>() | ...);

        // Iterate over all archetypes
        for (auto& arch : archetypes) {
            // Check if archetype has atleast the components of the query
            if (!detail::matchArchetypeSignatures(arch.signature, query)) continue;

            // Cache the component arrays for efficiency
            auto comps = std::make_tuple(
                arch.getOrCreateComponentArray<std::decay_t<Components>, ComponentManager>()...);

            size_t count = arch.entities.size();

            // Apply the function to each entity in the archetype
            for (size_t i = 0; i < count; ++i) {
                std::apply([&](auto*... arrays) { func((arrays->get(i))...); }, comps);
            }
        }
    }

    // check if type t is in a list of types
    template <typename T, typename... Ts>
    constexpr bool contains_type() {
        return (std::is_same_v<T, Ts> || ...);
    }

    // Add a Component to an Entity.
    // Requires Type list of all Components the Entity has plus the new Component.
    // Intern:
    // 1. Adds the Entity to an archetype of the given Type list (components)
    // 2. Copies the data from the old archetype to the new one.
    // 3. Adds the data from the newComponents to the new archetype.
    // 4. To remove the entity from the old list, the entity is swapped with the last entity from
    // the entities list. Same for the Components. Finally pop last element to remove effectively
    // the element. -> Remove is O(1)
    template <typename... AllComponents, typename... NewComponents>
    void addComponent(EntityId entityId, NewComponents&&... newComponents) {
        static_assert(sizeof...(NewComponents) <= sizeof...(AllComponents),
                      "You must pass exactly the new components for the added types");

        // Look up the entity
        auto it = entityLocationMap.find(entityId);
        if (it == entityLocationMap.end()) throw std::out_of_range("Entity not found.");

        // Temp save old meta data
        detail::Archetype* oldArch = getOrCreateArchetype(it->second.signature);
        size_t oldIndex = it->second.indexInArchetype;

        size_t lastIndex = oldArch->entities.size() - 1;

        // Build the new signature from all components
        detail::ArchetypeSignature newSignature =
            (ComponentManager::template GetComponentMask<std::decay_t<AllComponents>>() | ...);

        // Early-out: no change
        if (oldArch->signature == newSignature) return;

        detail::Archetype* newArch = getOrCreateArchetype(newSignature);
        newArch->entities.push_back(entityId);
        size_t newIndex = newArch->entities.size() - 1;

        // function to move existing components (not part of newComponents) from old to new
        auto moveExisting = [&]<typename T>() {
            if (!contains_type<T, NewComponents...>()) {
                detail::ComponentArray<T>* src =
                    oldArch->getOrCreateComponentArray<T, ComponentManager>();
                detail::ComponentArray<T>* dst =
                    newArch->getOrCreateComponentArray<T, ComponentManager>();
                dst->copyElementFrom(src, oldIndex);

                // remove old data
                // if the component is not the last index, swap data with last index
                if (oldIndex != lastIndex) src->moveElement(lastIndex, oldIndex);
                // remove component data of last index
                src->removeLast();
            }
        };

        // execute move fuction for each Component
        (moveExisting.template operator()<AllComponents>(), ...);

        // insert new component data into each table
        (newArch->getOrCreateComponentArray<std::decay_t<NewComponents>, ComponentManager>()
             ->push_back(std::forward<NewComponents>(newComponents)),
         ...);

        // update entity location
        entityLocationMap[entityId] = {newSignature, newIndex};

        // if the entity is not the last index, swap entityId with last index
        if (oldIndex != lastIndex) {
            std::swap(oldArch->entities[lastIndex], oldArch->entities[oldIndex]);
            EntityId swapId = oldArch->entities[oldIndex];
            // update
            entityLocationMap[swapId] = detail::EntityLocation{oldArch->signature, oldIndex};
        }
        // remove entity from old arch
        oldArch->entities.pop_back();
    }

   private:
    // All archetypes in the world
    std::vector<detail::Archetype> archetypes{};
    // Map of entity ID to their location
    std::unordered_map<EntityId, detail::EntityLocation> entityLocationMap{};
    // EntityId generator
    EntityId generateEntityId() {
        static int nextEntityId = 0;  // TODO: global static, not per instance!
        return nextEntityId++;
    }
    // Retrieves or creates an archetype based on the signature.
    detail::Archetype* getOrCreateArchetype(const detail::ArchetypeSignature& sig) {
        // Check if an Archetype exists for the given signature.
        for (auto& a : archetypes) {
            if (a.signature == sig) return &a;
        }
        // If no Archetype exist for the given signature, create new one.
        archetypes.push_back(detail::Archetype{sig});
        return &archetypes.back();
    }
};
}  // namespace ecs