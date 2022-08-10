#pragma once

#include <cstddef>
#include <memory>
#include <vector>

namespace Aporia
{
    class Logger;
    class Scene;

    class SceneManager final
    {
        using Ref = std::shared_ptr<Scene>;
        using Container = std::vector<Ref>;

    public:
        SceneManager(Logger& logger);

        size_t add(Ref scene);
        void remove(Ref scene);

        void load_scene(size_t id);
        void load_scene(Ref scene);

        Ref get_current_scene() const { return _scenes[_current_scene]; };

    private:
        size_t _index(Ref scene);
        bool _load(size_t id);

    private:
        Logger& _logger;

        Container _scenes;
        size_t _current_scene = 0;
    };
}
