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

        Entity() = default;

        Entity(Ts&&... args)
            : components(std::make_tuple<Ts...>(std::forward<Ts>(args)...)) {}

        template<typename T, std::enable_if_t<has_type_v<Components, T>, int> = 0>
        T& get_component()
        {
            return std::get<T>(components);
        }

        template<typename T, std::enable_if_t<has_type_v<Components, T>, int> = 0>
        const T& get_component() const
        {
            return std::get<T>(components);
        }

    private:
        Components components;
    };
}