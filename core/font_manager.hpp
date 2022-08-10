#pragma once

#include <filesystem>
#include <unordered_map>
#include <string>

#include "graphics/font.hpp"

namespace Aporia
{
    class Logger;

    class FontManager final
    {
    public:
        FontManager(Logger& logger);

        void load_font(std::string name, std::filesystem::path filepath);
        const Font& get(const std::string& name) const;

        static const Font default_font;

    private:
        Logger& _logger;

        std::unordered_map<std::string, Font> _fonts;
    };
}
