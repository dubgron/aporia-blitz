#pragma once

#include <algorithm>
#include <functional>
#include <tuple>
#include <type_traits>
#include <utility>
#include <vector>

#include "components/transform2d.hpp"
#include "graphics/drawables/line2d.hpp"
#include "graphics/drawables/circle2d.hpp"
#include "graphics/drawables/rectangle2d.hpp"
#include "graphics/drawables/sprite.hpp"
#include "graphics/drawables/text.hpp"
#include "utils/type_traits.hpp"

namespace Aporia
{
    class Group final
    {
        template<typename T>
        using Container = std::vector<std::reference_wrapper<T>>;

        using Lines2D = Container<Line2D>;
        using Circles2D = Container<Circle2D>;
        using Rectangles2D = Container<Rectangle2D>;
        using Sprites = Container<Sprite>;
        using Texts = Container<Text>;
        using Groups = Container<Group>;

        using Drawables = std::tuple<Lines2D, Circles2D, Rectangles2D, Sprites, Texts, Groups>;

    public:
        template<typename T, typename D = std::decay_t<T>> requires has_type_v<Drawables, Container<D>>
        void add(T& drawable)
        {
            Container<D>& vec = std::get<Container<D>>(_drawables);
            vec.push_back(drawable);
        }

        template<typename T> requires has_type_v<Drawables, Container<T>>
        void remove(const T& drawable)
        {
            Container<T>& vec = std::get<Container<T>>(_drawables);
            const auto pred = [&drawable](const auto& ref){ return std::addressof(ref.get()) == std::addressof(drawable); };
            const auto remove = std::remove_if(vec.begin(), vec.end(), pred);
            if (remove != vec.end())
            {
                vec.erase(remove);
            }
        }

        template<typename T> requires has_type_v<Drawables, Container<T>>
        const Container<T>& get_container() const
        {
            return std::get<Container<T>>(_drawables);
        }

        Transform2D transform;

    private:
        Drawables _drawables;
    };
}
