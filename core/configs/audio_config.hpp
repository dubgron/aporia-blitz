#pragma once

#include <string>
#include <vector>

#include "platform.hpp"

namespace Aporia
{
	struct APORIA_API AudioConfig final
	{
		std::vector<std::string> musics;
		std::vector<std::string> sounds;
	};
}