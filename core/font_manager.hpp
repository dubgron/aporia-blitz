#pragma once

#include <filesystem>
#include <string>

#include "logger.hpp"
#include "graphics/font.hpp"

namespace Aporia
{
    class FontManager final
    {
    public:
        FontManager(Logger& logger);

        void load_font(std::string name, std::filesystem::path filepath);
        const Font& get(const std::string& name);

    private:
        Logger& _logger;

        std::unordered_map<std::string, Font> _fonts;
    };
}
