#pragma once

#include <cstdint>
#include <string>

#include "entity.hpp"
#include "font.hpp"
#include "components/color.hpp"
#include "components/transform2d.hpp"

namespace Aporia
{
    struct Text final : public Entity<Transform2D, Color>
    {
        Text() = default;

        Text(glm::vec3 position, std::string caption, float size, const Font& font, Color color = Aporia::Colors::White)
            : Entity(Transform2D{ .position = std::move(position), .scale = glm::vec2{ size } },
                     Color{ std::move(color) }),
                caption(std::move(caption)),
                font(font) {}

        std::string caption;
        const Font& font;
    };
}
