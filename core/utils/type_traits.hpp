#pragma once

#include <tuple>
#include <type_traits>

namespace Aporia::Utils
{
    template<typename T, typename Tuple>
    struct is_in_tuple;

    template<typename T, typename... Us>
    struct is_in_tuple<T, std::tuple<Us...>> : std::disjunction<std::is_same<T, Us>...> {};
    
    template<typename T, typename... Us>
    constexpr bool is_in_tuple_v = is_in_tuple<T, Us...>::value;
}
