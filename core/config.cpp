#include "config.hpp"

#include <type_traits>

#include <nlohmann/json.hpp>

#include "utils/read_file.hpp"

namespace Aporia
{
    Config::Config(const std::string& config)
    {
        using json = nlohmann::json;
        
        std::string data = Utils::read_file(config);
        json config_json = json::parse(data);

        /* TODO: Handling when json file is not correct */

        /* Getting Window Data */
        for (auto& window : config_json["window_config"])
        {
            std::string title = window["title"];
            sf::Vector2i position = { window["position"][0], window["position"][0] };
            unsigned int width = window["width"];
            unsigned int height = window["height"];

            window_data.emplace_back(title, position, width, height);
        }
    }
}