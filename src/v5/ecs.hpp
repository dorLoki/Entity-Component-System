#pragma once
#include <tuple>

// Main ECS template.
// Accepts a user-defined configuration that provides a compile-time ComponentList.
namespace ecs {
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
}  // namespace ecs