#pragma once

#include <cstddef>
#include <vector>

#include <glm/glm.hpp>

#include "graphics/framebuffer.hpp"
#include "graphics/uniform_buffer.hpp"
#include "graphics/drawables/group.hpp"

namespace Aporia
{
    struct LightSource final
    {
        glm::vec2 origin{ 0.0f };
        float range = 1.0f;
        float falloff = 2.0f;

        glm::vec3 color{ 1.0f };
        float intensity = 1.0f;
    };

    struct LightRenderer final
    {
        static constexpr size_t MAX_LIGHTS = 1000;

        ShaderRef raymarching_shader = 0u;
        ShaderRef shadowcasting_shader = 0u;

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
