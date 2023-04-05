#include "aporia_rendering.hpp"
#include "graphics/drawables/circle2d.hpp"
#include "graphics/drawables/line2d.hpp"
#include "graphics/drawables/rectangle2d.hpp"
#include "graphics/drawables/sprite.hpp"

namespace Aporia
{
    Circle2D::Circle2D()
        : shader(default_shader)
    {
    }

    Line2D::Line2D()
        : shader(default_shader)
    {
    }

    Rectangle2D::Rectangle2D()
        : shader(default_shader)
    {
    }

    Sprite::Sprite()
        : shader(default_shader)
    {
    }
}
