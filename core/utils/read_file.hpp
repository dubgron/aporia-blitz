#pragma once

#include <fstream>
#include <string>
#include <string_view>
#include <utility>

#include <nlohmann/json.hpp>

namespace Aporia
{
    inline std::string read_file(std::string_view path)
    {
        std::ifstream file{ path.data(), std::ios::in };
        return std::string{ std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>() };
    }

    inline nlohmann::json load_json(std::string_view path)
    {
        std::string data = read_file(path);
        return nlohmann::json::parse(std::move(data));
    }
}
