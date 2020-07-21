#pragma once

#include <tuple>
#include <type_traits>
#include <utility>

#include "utils/type_traits.hpp"

namespace Aporia
{
    template<typename... Comps>
    struct Entity
    {
        using Components = std::tuple<Comps...>;

        Entity() = default;

        Entity(Comps&&... args)
            : components(std::make_tuple<Comps...>(std::forward<Comps>(args)...))
        {
        }

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

        Components components;
    };
}