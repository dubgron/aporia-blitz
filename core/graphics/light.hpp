#pragma once

#include <vector>

#include "aporia_types.hpp"
#include "graphics/framebuffer.hpp"
#include "graphics/uniform_buffer.hpp"
#include "graphics/drawables/group.hpp"

namespace Aporia
{
    struct LightSource final
    {
        v2 origin{ 0.f };
        f32 range = 1.f;
        f32 falloff = 2.f;

        v3 color{ 1.f };
        f32 intensity = 1.f;
    };

    struct LightRenderer final
    {
        static constexpr u64 MAX_LIGHTS = 1000;

        ShaderRef raymarching_shader = 0;
        ShaderRef shadowcasting_shader = 0;

        Framebuffer masking;
        Framebuffer raymarching;

        std::vector<LightSource> sources;
        Group blockers;

        UniformBuffer<LightSource[MAX_LIGHTS], 0> uniform_buffer{ "Lights" };

        bool enabled = false;

        LightRenderer()
        {
            sources.reserve(MAX_LIGHTS);
        }

        void begin()
        {
            sources.clear();
        }

        void end()
        {
            uniform_buffer.set_data(sources.data());
        }
    };
}
