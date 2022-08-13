#include "scene_manager.hpp"

#include <algorithm>

#include "common.hpp"
#include "scene.hpp"

namespace Aporia
{
    SceneManager::SceneManager(Logger& logger)
        : _logger(logger)
    {
        _scenes.emplace_back(std::make_shared<Scene>(logger, "Default Scene"));
    }

    size_t SceneManager::add(Ref scene)
    {
        size_t index = _index(scene);
        if (index == _scenes.size())
        {
            _scenes.emplace_back( std::move(scene) );
        }
        else
        {
            APORIA_LOG(_logger, Warning, "Scene '{}' has already been added!", scene->get_name());
        }

        return index;
    }

    void SceneManager::remove(Ref scene)
    {
        if (scene != _scenes[_current_scene])
        {
            auto remove = std::remove(_scenes.begin(), _scenes.end(), scene);
            if (remove != _scenes.end())
            {
                _scenes.erase(remove);
            }
            else
            {
                APORIA_LOG(_logger, Warning, "No scene named '{}'!", scene->get_name());
            }
        }
        else
        {
            APORIA_LOG(_logger, Error, "Scene '{}' is currently in use!", scene->get_name());
        }
    }

    void SceneManager::load_scene(size_t id)
    {
        if (!_load(id))
        {
            APORIA_LOG(_logger, Error, "No scene with id = '{}'!", id);
        }
    }

    void SceneManager::load_scene(Ref scene)
    {
        size_t id = _index(scene);
        if (!_load(id))
        {
            APORIA_LOG(_logger, Warning, "No scene named '{}'!", scene->get_name());
        }
    }

    size_t SceneManager::_index(Ref scene)
    {
        auto find = std::find(_scenes.begin(), _scenes.end(), scene);
        return find - _scenes.begin();
    }

    bool SceneManager::_load(size_t id)
    {
        if (id < _scenes.size())
        {
            APORIA_LOG(_logger, Info, "Unloading '{}' scene", _scenes[_current_scene]->get_name());
            _scenes[_current_scene]->on_unload();

            APORIA_LOG(_logger, Info, "Loading '{}' scene", _scenes[id]->get_name());
            _scenes[id]->on_load();

            _current_scene = id;
        }

        return _current_scene == id;
    }
}
