#pragma once

#include <algorithm>
#include <functional>
#include <tuple>
#include <type_traits>
#include <utility>
#include <vector>

#include "entity.hpp"
#include "components/transform2d.hpp"
#include "graphics/line2d.hpp"
#include "graphics/circle2d.hpp"
#include "graphics/rectangle2d.hpp"
#include "graphics/sprite.hpp"
#include "utils/type_traits.hpp"

namespace Aporia
{
    class Group final : public Entity<Transform2D>
    {
        template<typename T>
        using Container = std::vector<std::reference_wrapper<const T>>;

        using Lines2D = Container<Line2D>;
        using Circles2D = Container<Circle2D>;
        using Rectangles2D = Container<Rectangle2D>;
        using Sprites = Container<Sprite>;
        using Groups = Container<Group>;

        using Drawables = std::tuple<Lines2D, Circles2D, Rectangles2D, Sprites, Groups>;

    public:
        Group() = default;
        Group(sf::Vector2f position)
            : Entity(Transform2D{ std::move(position) }) {}

        template<typename T, typename D = std::decay_t<T>, std::enable_if_t<has_type_v<Drawables, Container<D>>, int> = 0>
        void add(T&& drawable)
        {
            auto& vec = std::get<Container<D>>(_drawables);
            vec.emplace_back(std::forward<T>(drawable));
        }

        template<typename T, std::enable_if_t<has_type_v<Drawables, Container<T>>, int> = 0>
        void remove(const T& drawable)
        {
            auto& vec = std::get<Container<T>>(_drawables);
            auto find = [&drawable](const auto& ref){ return std::addressof(ref.get()) == std::addressof(drawable); };
            auto remove = std::remove_if(vec.begin(), vec.end(), find);

            if (remove != vec.end())
                vec.erase(remove);
        }

        template<typename T, std::enable_if_t<has_type_v<Drawables, Container<T>>, int> = 0>
        const Container<T>& get_container() const
        {
            return std::get<Container<T>>(_drawables);
        }

    private:
        Drawables _drawables;
    };
}
