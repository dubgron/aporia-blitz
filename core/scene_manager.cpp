#include "scene_manager.hpp"

#include <algorithm>

#include "common.hpp"
#include "scene.hpp"

namespace Aporia
{
    SceneManager::SceneManager()
    {
        _scenes.emplace_back(std::make_shared<Scene>("Default Scene"));
    }

    u64 SceneManager::add(Ref scene)
    {
        u64 index = _index(scene);
        if (index == _scenes.size())
        {
            _scenes.emplace_back( std::move(scene) );
        }
        else
        {
            APORIA_LOG(Warning, "Scene '{}' has already been added!", scene->get_name());
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
                APORIA_LOG(Warning, "No scene named '{}'!", scene->get_name());
            }
        }
        else
        {
            APORIA_LOG(Error, "Scene '{}' is currently in use!", scene->get_name());
        }
    }

    void SceneManager::load_scene(u64 id)
    {
        if (!_load(id))
        {
            APORIA_LOG(Error, "No scene with id = '{}'!", id);
        }
    }

    void SceneManager::load_scene(Ref scene)
    {
        u64 id = _index(scene);
        if (!_load(id))
        {
            APORIA_LOG(Warning, "No scene named '{}'!", scene->get_name());
        }
    }

    u64 SceneManager::_index(Ref scene)
    {
        auto find = std::find(_scenes.begin(), _scenes.end(), scene);
        return find - _scenes.begin();
    }

    bool SceneManager::_load(u64 id)
    {
        if (id < _scenes.size())
        {
            APORIA_LOG(Info, "Unloading '{}' scene", _scenes[_current_scene]->get_name());
            _scenes[_current_scene]->on_unload();

            APORIA_LOG(Info, "Loading '{}' scene", _scenes[id]->get_name());
            _scenes[id]->on_load();

            _current_scene = id;
        }

        return _current_scene == id;
    }
}
