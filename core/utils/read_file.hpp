#pragma once

#include <fstream>
#include <string>
#include <utility>

namespace Aporia
{
    inline std::string read_file(std::string path)
    {
        std::ifstream file(std::move(path), std::ios::binary);
        return std::string((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    }
}
