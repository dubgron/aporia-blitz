#pragma once

#include <string>

#include <SFML/Audio/SoundBuffer.hpp>

#include "platform.hpp"

namespace Aporia
{
	struct APORIA_API AudioConfig final
	{
		std::vector<std::string> musics;
		std::vector<std::string> sounds;
	};
}