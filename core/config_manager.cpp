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
        
        std::string data = read_file(config);
        json config_json = json::parse(data);

        /* TODO: Handling when json file is not correct */

        /* Getting Window Data */
        auto window = config_json["window_config"];

        window_data.title = window["title"];
        window_data.position = { window["position"][0], window["position"][1] };
        window_data.width = window["width"];
        window_data.height = window["height"];
        window_data.framerate = window["framerate"];

        /* Getting Texture Data */
        texture_data.atlas = config_json["texture_config"]["src"];

        _good = true;
    }

    bool ConfigManager::is_good() const
    {
        return _good;
    }
}
