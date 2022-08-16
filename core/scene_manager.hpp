#pragma once

#include <cstddef>
#include <memory>
#include <vector>

namespace Aporia
{
    class Scene;

    class SceneManager final
    {
        using Ref = std::shared_ptr<Scene>;
        using Container = std::vector<Ref>;

    public:
        SceneManager();

        size_t add(Ref scene);
        void remove(Ref scene);

        void load_scene(size_t id);
        void load_scene(Ref scene);

        Ref get_current_scene() const { return _scenes[_current_scene]; };

    private:
        size_t _index(Ref scene);
        bool _load(size_t id);

    private:
        Container _scenes;
        size_t _current_scene = 0;
    };
}
