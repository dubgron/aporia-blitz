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

            if (_queue_play_music.find(name) == _queue_play_music.end())
            {
                sf::Music* music_tmp = new sf::Music();
                music_tmp->openFromFile(music);

                _queue_play_music.emplace(name, music_tmp);
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
        for (const auto& music : _queue_play_music)
        {
            delete music.second;
        }
    }

    void AudioManager::play(const std::string& name, float volume, const sf::Vector3f& position, bool relative)
    {       
        _queue_play_sound[_head_sounds].setBuffer(_buffers[name]);

        _queue_play_sound[_head_sounds].setVolume(volume);
        _queue_play_sound[_head_sounds].setPosition(position);
        _queue_play_sound[_head_sounds].setRelativeToListener(relative);

        _queue_play_sound[_head_sounds].play();

        _head_sounds = (_head_sounds + 1u) % _queue_play_sound.size();

        _logger.log(LOG_INFO) << "Sound '" << name << "' is playing.";
    }

    void AudioManager::play_music(const std::string& name, float volume, const sf::Vector3f& position, bool loop)
    {
        _queue_play_music[name]->setVolume(volume);
        _queue_play_music[name]->setPosition(position);
        _queue_play_music[name]->setLoop(loop);

        _queue_play_music[name]->play();

        _logger.log(LOG_INFO) << "Music '" << name <<"' is playing.";
    }

    void AudioManager::pause(const std::string& name)
    {
        if (_queue_play_music.find(name) != _queue_play_music.end())
        {
            _queue_play_music[name]->pause();
        }
        else
        {
            _logger.log(LOG_ERROR) << "Music '" << name << "' does not excist!";
        }
    }

    void AudioManager::pause_all()
    {
        for (const auto& music : _queue_play_music)
        {
            music.second->pause();
        }
    }

    void AudioManager::stop(const std::string& name)
    {
        if (_queue_play_music.find(name) != _queue_play_music.end())
        {
            _queue_play_music[name]->stop();
        }
        else
        {
            _logger.log(LOG_ERROR) << "Music '" << name << "' does not excist!";
        }
    }

    void AudioManager::stop_all()
    {
        for (const auto& music : _queue_play_music)
        {
            music.second->stop();
        }
    }

    void AudioManager::stop_sounds()
    {
        for (int i = 0; i < _queue_play_sound.size(); i++)
        {
            _queue_play_sound[i].stop();
        }
    }
}