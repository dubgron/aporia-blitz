#pragma once

#include <cstdint>
#include <string>

#include <glm/vec3.hpp>

#include "entity.hpp"
#include "components/color.hpp"
#include "components/transform2d.hpp"
#include "graphics/font.hpp"
#include "graphics/shader.hpp"

namespace Aporia
{
    struct Text final : public Entity<Transform2D, Color, ShaderRef>
    {
        Text() = default;
        Text(glm::vec3 position, std::string caption, float size, const Font& font, Color color = Aporia::Colors::White, ShaderRef shader = 0u);

        std::string caption;
        const Font& font;
    };
}
