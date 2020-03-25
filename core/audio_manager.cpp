#include "audio_manager.hpp"

namespace Aporia
{
    AudioManager::AudioManager(Logger& logger, const AudioConfig& config)
        : _logger(logger)
    {
        for (const auto& sound : config.sounds)
        {
            std::string name = sound;
            name.erase(name.find_last_of("."));
            name.erase(0, name.find_last_of("/") + 1);

            sf::SoundBuffer buffer;
            buffer.loadFromFile(sound);

            _buffers.try_emplace(name, buffer);
        }      

        _logger.log(LOG_INFO) << "All sounds loaded successfully";

        for (const auto& music : config.musics)
        {
            std::string name = music;
            name.erase(name.find_last_of("."));
            name.erase(0, name.find_last_of("/") + 1);

            if (_musics.find(name) == _musics.end())
            {
                sf::Music* music_tmp = new sf::Music();
                music_tmp->openFromFile(music);

                _musics.emplace(name, music_tmp);
            }
            else
            {
                _logger.log(LOG_WARNING) << "There are two musics named '" << name << "'! One of them will be overwritten!";
            }
        }

        _logger.log(LOG_INFO) << "All music loaded successfully";
    }

    AudioManager::~AudioManager()
    {
        for (const auto& music : _musics)
        {
            delete music.second;
        }
    }

    void AudioManager::play_sound(const std::string& name, float volume)
    {
        play_sound(name, volume, sf::Vector3f(0.0f, 0.0f, 0.0f), true);
    }

    void AudioManager::play_sound(const std::string& name, float volume, const sf::Vector3f& position, bool relative)
    {
        _sounds[_head_sounds].setBuffer(_buffers[name]);

        _sounds[_head_sounds].setVolume(volume);
        _sounds[_head_sounds].setPosition(position);
        _sounds[_head_sounds].setRelativeToListener(relative);

        _sounds[_head_sounds].play();

        _head_sounds = (_head_sounds + 1u) % _sounds.size();

        _logger.log(LOG_INFO) << "Sound '" << name << "' is playing.";
    }

    void AudioManager::play_music(const std::string& name, float volume, const sf::Vector3f& position, bool loop)
    {
        if (_musics[name]->getStatus() != sf::Music::Status::Paused)
            _musics[name]->stop();

        _musics[name]->setVolume(volume);
        _musics[name]->setPosition(position);
        _musics[name]->setLoop(loop);

        _musics[name]->play();

        _logger.log(LOG_INFO) << "Music '" << name <<"' is playing.";
    }

    void AudioManager::pause_music(const std::string& name)
    {
        if (_musics.find(name) != _musics.end())
        {
            _musics[name]->pause();
        }
        else
        {
            _logger.log(LOG_ERROR) << "Music '" << name << "' does not excist!";
        }
    }

    void AudioManager::pause_all_music()
    {
        for (const auto& music : _musics)
        {
            music.second->pause();
        }
    }

    void AudioManager::stop_music(const std::string& name)
    {
        if (_musics.find(name) != _musics.end())
        {
            _musics[name]->stop();
        }
        else
        {
            _logger.log(LOG_ERROR) << "Music '" << name << "' does not excist!";
        }
    }

    void AudioManager::stop_all_music()
    {
        for (const auto& music : _musics)
        {
            music.second->stop();
        }
    }

    void AudioManager::stop_all_sounds()
    {
        for (sf::Sound& sound : _sounds)
        {
            sound.stop();
        }
    }
}