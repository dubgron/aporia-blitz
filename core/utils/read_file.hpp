#pragma once

#include <filesystem>
#include <fstream>
#include <string>
#include <string_view>
#include <utility>

namespace Aporia
{
    inline std::string read_file(std::string_view filepath)
    {
        APORIA_ASSERT( std::filesystem::exists(filepath) );

        std::ifstream file{ filepath.data(), std::ios::in };
        APORIA_LOG(Info, "Opened '{}' successfully!", filepath);

        return std::string{ std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>() };
    }
}
