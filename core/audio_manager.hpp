#pragma once

#include <array>
#include <map>
#include <string>

#include <SFML/Audio/Music.hpp>
#include <SFML/Audio/Sound.hpp>
#include <SFML/Audio/SoundBuffer.hpp>

#include "logger.hpp"
#include "platform.hpp"
#include "configs/audio_config.hpp"

namespace Aporia
{
    class APORIA_API AudioManager final
    {
    public:
        AudioManager(Logger& logger, const AudioConfig& config);
        ~AudioManager();

        void play(const std::string& name, float volume);
        void play(const std::string& name, float volume, const sf::Vector3f& position, bool relative = false);
        void play_music(const std::string& name, float volume, const sf::Vector3f& position, bool loop = false);

        void pause(const std::string& name);
        void pause_all();

        void stop(const std::string& name);
        void stop_all();

        void stop_sounds();

    private:
        Logger& _logger;

        std::array<sf::Sound, 64> _queue_play_sound;
        
        std::unordered_map<std::string, sf::SoundBuffer> _buffers;
        std::unordered_map<std::string, sf::Music*> _queue_play_music;

        unsigned int _head_sounds = 0;
    };
}