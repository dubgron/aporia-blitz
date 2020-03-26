#pragma once

#include <array>
#include <string>
#include <unordered_map>

#include <SFML/Audio/Music.hpp>
#include <SFML/Audio/Sound.hpp>
#include <SFML/Audio/SoundBuffer.hpp>

#include "configs/audio_config.hpp"
#include "logger.hpp"
#include "platform.hpp"

namespace Aporia
{
    class APORIA_API AudioManager final
    {
    public:
        AudioManager(Logger& logger, const AudioConfig& config);
        ~AudioManager();

        void play_sound(const std::string& name, float volume);
        void play_sound(const std::string& name, float volume, const sf::Vector3f& position, bool relative = false);
        void stop_all_sounds();

        void play_music(const std::string& name, float volume, bool loop = false, const sf::Vector3f& position = sf::Vector3f());
        void pause_music(const std::string& name);
        void pause_all_music();
        void stop_music(const std::string& name);
        void stop_all_music();

    private:
        bool _find_music(const std::string& name) const; 
        bool _find_sound(const std::string& name) const;

        Logger& _logger;

        std::array<sf::Sound, 64> _sounds;
        
        std::unordered_map<std::string, sf::SoundBuffer> _buffers;
        std::unordered_map<std::string, sf::Music*> _musics;

        unsigned int _head_sounds = 0;
    };
}