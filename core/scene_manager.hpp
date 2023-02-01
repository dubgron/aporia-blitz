#pragma once

#include <memory>
#include <vector>

#include "aporia_types.hpp"

namespace Aporia
{
    class Scene;

    class SceneManager final
    {
        using Ref = std::shared_ptr<Scene>;
        using Container = std::vector<Ref>;

    public:
        SceneManager();

        u64 add(Ref scene);
        void remove(Ref scene);

        void load_scene(u64 id);
        void load_scene(Ref scene);

        Ref get_current_scene() const { return _scenes[_current_scene]; };

    private:
        u64 _index(Ref scene);
        bool _load(u64 id);

    private:
        Container _scenes;
        u64 _current_scene = 0;
    };
}
