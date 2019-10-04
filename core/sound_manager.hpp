#pragma once

#include <string>

#include <SFML/Audio/Music.hpp>

#include "logger.hpp"
#include "platform.hpp"

namespace Aporia
{
    class APORIA_API SoundManager final
    {
    public:
        SoundManager(std::shared_ptr<Logger> logger);

        void play(const std::string& name, bool loop = false);
        void pause();
        void stop();
        void retrece();

    private:
        sf::Music _music;
        std::string _music_name;
        bool _loop;

        std::shared_ptr<Logger> _logger;
    };
}