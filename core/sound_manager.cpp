#include "sound_manager.hpp"

#include <filesystem>

namespace Aporia
{
    SoundManager::SoundManager(std::shared_ptr<Logger> logger)
        : _logger(logger)
    {
    }

    void SoundManager::play(const std::string& name)
    {
        _music_name = name;

        if (!std::filesystem::exists(name))
        {
            _logger->log(LOG_ERROR) << "File '" << name << "' not open!";
            return;
        }

        _music.openFromFile(name);
        _music.play();
        _logger->log(LOG_INFO) << "Music is playing.";
    }

    void SoundManager::pause()
    {
        _music.pause();
        _logger->log(LOG_INFO) << "Music is paused.";
    }

    void SoundManager::stop()
    {
        _music.stop();
        _logger->log(LOG_INFO) << "Music is stopped.";
    }

    void SoundManager::retrece()
    {
        _logger->log(LOG_INFO) << "Retrece music.";
        play(_music_name);
    }
}