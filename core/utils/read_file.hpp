#pragma once

#include <fstream>
#include <string_view>
#include <utility>

namespace Aporia
{
    inline std::string read_file(std::string_view path)
    {
        std::ifstream file(path.data(), std::ios::binary);
        return std::string((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    }
}
