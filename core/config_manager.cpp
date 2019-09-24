#include "config_manager.hpp"

#include <filesystem>

#include <nlohmann/json.hpp>

#include "utils/read_file.hpp"

namespace Aporia
{
    ConfigManager::ConfigManager(const std::shared_ptr<Logger>& logger, const std::string& config)
        : _logger(logger)
    {
        if (!std::filesystem::exists(config))
        {
            _logger->log(LOG_CRITICAL) << "Config file '" << config << "' doesn't exist!";
            return;
        }

        using json = nlohmann::json;
        
        std::string data = Utils::read_file(config);
        json config_json = json::parse(data);

        /* TODO: Handling when json file is not correct */

        /* Getting Window Data */
        for (auto& window : config_json["window_config"])
        {
            std::string title = window["title"];
            sf::Vector2i position = { window["position"][0], window["position"][1] };
            unsigned int width = window["width"];
            unsigned int height = window["height"];

            window_data.emplace_back(title, position, width, height);
        }

        _ok = true;
    }

    bool ConfigManager::is_ok() const
    {
        return _ok;
    }
}
