#pragma once

#include <string>
#include <unordered_map>
#include <vector>

namespace Aporia
{
    struct AnimationFrameConfig final
    {
        std::string texture_name;
        float duration = 0.0f;
    };

    struct AnimationConfig final
    {
        std::unordered_map<std::string, std::vector<AnimationFrameConfig>> animations;
    };
}
