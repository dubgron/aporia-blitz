#pragma once

#include <tuple>
#include <type_traits>

namespace Aporia
{
    template<typename Tuple, typename T>
    struct has_type;

    template<typename T, typename... Us>
    struct has_type<std::tuple<Us...>, T> : std::disjunction<std::is_same<T, Us>...> {};
    
    template<typename Tuple, typename T>
    constexpr bool has_type_v = has_type<Tuple, T>::value;

    template<typename Tuple, typename... Ts>
    struct has_types : std::conjunction<has_type<Tuple, Ts>...> {};

    template<typename Tuple, typename... Ts>
    constexpr bool has_types_v = has_types<Tuple, Ts...>::value;
}
