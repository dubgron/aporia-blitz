#include "scene_manager.hpp"

#include <algorithm>

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
            _scenes.emplace_back(std::move(scene));
        else
            _logger.log(LOG_WARNING) << "Scene '" << scene->get_name() << "' has already been added!";

        return index;
    }

    void SceneManager::remove(Ref scene)
    {
        if (scene != _scenes[_current_scene])
        {
            auto remove = std::remove(_scenes.begin(), _scenes.end(), scene);
            if (remove != _scenes.end())
                _scenes.erase(remove);
            else
                _logger.log(LOG_WARNING) << "No scene named '" << scene->get_name() << "'!";
        }
        else
            _logger.log(LOG_ERROR) << "Scene '" << scene->get_name() << "' is currently in use!";
    }

    void SceneManager::load_scene(size_t id)
    {
        if (!_load(id))
            _logger.log(LOG_ERROR) << "No scene with id = " << id << "!";
    }

    void SceneManager::load_scene(Ref scene)
    {
        size_t id = _index(scene);
        if (!_load(id))
            _logger.log(LOG_WARNING) << "No scene named '" << scene->get_name() << "'!";
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
            _logger.log(LOG_INFO) << "Unloading '" << _scenes[_current_scene]->get_name() << "' scene";
            _scenes[_current_scene]->on_unload();

            _logger.log(LOG_INFO) << "Loading '" << _scenes[id]->get_name() << "' scene";
            _scenes[id]->on_load();

            _current_scene = id;
        }

        return _current_scene == id;
    }
}
