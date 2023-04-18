#pragma once

#include "aporia_shaders.hpp"
#include "aporia_textures.hpp"
#include "aporia_utils.hpp"

namespace Aporia
{
    struct Entity
    {
        u64 index = 0;
        u64 flags = 0;

        v2 position{ 0.f };
        f32 z = 0.f;

        f32 rotation = 0.f;
        v2 center_of_rotation{ 0.f };

        f32 width = 0.f;
        f32 height = 0.f;
        v2 scale{ 1.f };

        SubTexture texture;
        Color color;
        u32 shader_id = default_shader;
    };
}
