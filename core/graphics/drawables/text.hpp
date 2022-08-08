#pragma once

#include <cstdint>
#include <string>
#include <type_traits>

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
        Text();
        Text(glm::vec3 position, std::string caption, float size, const Font& font, Color color = Colors::White, ShaderRef shader = 0);

        std::string caption;
        std::reference_wrapper<const Font> font;
    };
}
