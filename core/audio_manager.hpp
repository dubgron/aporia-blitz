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

        void play_sound(const std::string& name, float volume);
        void play_sound(const std::string& name, float volume, const sf::Vector3f& position, bool relative = false);
        void play_music(const std::string& name, float volume, const sf::Vector3f& position, bool loop = false);

        void pause_music(const std::string& name);
        void pause_all_music();

        void stop_music(const std::string& name);
        void stop_all_music();

        void stop_all_sounds();

    private:
        Logger& _logger;

        std::array<sf::Sound, 64> _sounds;
        
        std::unordered_map<std::string, sf::SoundBuffer> _buffers;
        std::unordered_map<std::string, sf::Music*> _musics;

        unsigned int _head_sounds = 0;
    };
}