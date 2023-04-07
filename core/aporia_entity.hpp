#pragma once

#include "aporia_shaders.hpp"
#include "aporia_textures.hpp"

namespace Aporia
{
    struct Transform2D
    {
        v3 position{ 0.f };

        v2 origin{ 0.f };
        f32 rotation = 0.f;

        v2 scale{ 1.f };
    };

    union Rectangular
    {
        struct
        {
            f32 width;
            f32 height;
        };
        v2 size;
    };

    struct Color final
    {
        u8 r = 255;
        u8 g = 255;
        u8 b = 255;
        u8 a = 255;
    };

    namespace Colors
    {
        inline constexpr Color Black       = Color{  0,   0,   0,  255 };
        inline constexpr Color White       = Color{ 255, 255, 255, 255 };
        inline constexpr Color Red         = Color{ 255,  0,   0,  255 };
        inline constexpr Color Green       = Color{  0,  255,  0,  255 };
        inline constexpr Color Blue        = Color{  0,   0,  255, 255 };
        inline constexpr Color Yellow      = Color{ 255, 255,  0,  255 };
        inline constexpr Color Magenta     = Color{ 255,  0,  255, 255 };
        inline constexpr Color Cyan        = Color{  0,  255, 255, 255 };
        inline constexpr Color Transparent = Color{  0,   0,   0,   0  };
    }

    struct Circle2D final
    {
        Transform2D transform;
        f32 radius = 0.f;
        Color color = Colors::White;
        ShaderID shader = 0;
    };

    struct Line2D
    {
        Transform2D transform;
        v2 offset{ 0.f };

        Color color = Colors::White;
        ShaderID shader = 0;
    };

    struct Rectangle2D
    {
        Transform2D transform;
        Rectangular size;
        Color color = Colors::White;
        ShaderID shader = 0;
    };

    struct Sprite
    {
        Transform2D transform;
        SubTexture texture;
        Rectangular rect;
        Color color = Colors::White;
        ShaderID shader = 0;
    };

    struct Entity
    {
        u64 index = 0;

        u64 flags = 0;
        Sprite sprite;
        /* etc. */
    };
}
