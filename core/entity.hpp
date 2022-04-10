#pragma once

#include <tuple>
#include <type_traits>
#include <utility>

#include "utils/type_traits.hpp"

namespace Aporia
{
    template<typename... Ts>
    class Entity
    {
    public:
        using Components = std::tuple<Ts...>;

        template<typename T>
        static constexpr bool IsComponent = has_type_v<Components, T>;

        Entity() = default;

        Entity(Ts&&... args)
            : components(std::make_tuple<Ts...>(std::forward<Ts>(args)...)) {}

        template<typename T> requires IsComponent<T>
        T& get_component()
        {
            return std::get<T>(components);
        }

        template<typename T>
        const T& get_component() const requires IsComponent<T>
        {
            return std::get<T>(components);
        }

    private:
        Components components;
    };
}
