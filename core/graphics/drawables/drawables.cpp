#include "graphics/renderer.hpp"
#include "graphics/drawables/circle2d.hpp"
#include "graphics/drawables/line2d.hpp"
#include "graphics/drawables/rectangle2d.hpp"
#include "graphics/drawables/sprite.hpp"
#include "graphics/drawables/text.hpp"

namespace Aporia
{
    Circle2D::Circle2D()
        : shader(Renderer::default_shader)
    {}

    Line2D::Line2D()
        : shader(Renderer::default_shader)
    {}

    Rectangle2D::Rectangle2D()
        : shader(Renderer::default_shader)
    {}

    Sprite::Sprite()
        : shader(Renderer::default_shader)
    {}

    Text::Text()
        : shader(Renderer::font_shader)
    {}
}
