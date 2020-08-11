#include "renderer.hpp"

#include <glm/ext/matrix_transform.hpp>
#include <SFML/Graphics/RenderStates.hpp>

#include "graphics/line2d.hpp"
#include "graphics/circle2d.hpp"
#include "graphics/rectangle2d.hpp"
#include "graphics/sprite.hpp"

namespace Aporia
{
    Renderer::Renderer(Logger& logger)
        : _logger(logger)
    {
        _tranformation_stack.emplace(glm::mat4{ 1.0f });
    }

    void Renderer::draw(const Group& group)
    {
        push_transform(group.get_component<Transform2D>());

        for (const Line2D& line : group.get_container<Line2D>())
            draw(line);

        for (const Circle2D& circle : group.get_container<Circle2D>())
            draw(circle);

        for (const Rectangle2D& rectangle : group.get_container<Rectangle2D>())
            draw(rectangle);

        for (const Sprite& sprite : group.get_container<Sprite>())
            draw(sprite);

        for (const Group& group : group.get_container<Group>())
            draw(group);

        pop_transform();
    }

    void Renderer::render(Window& window, const Camera& camera)
    {
        sf::RenderStates states;

        for (auto& [texture, vertex_array] : _textured_queue)
        {
            states.texture = texture.get();

            //window.draw(vertex_array, states);

            states.texture = nullptr;
            vertex_array.clear();
        }

        //window.draw(_circle_queue, states);
        _circle_queue.clear();

        //window.draw(_quad_queue, states);
        _quad_queue.clear();

        //window.draw(_line_queue, states);
        _line_queue.clear();
    }

    void Renderer::push_transform(const Transform2D& transform)
    {
        _tranformation_stack.push(std::move(_tranformation_stack.top() * to_mat4(transform)));
    }

    void Renderer::pop_transform()
    {
        if (_tranformation_stack.size() > 1)
            _tranformation_stack.pop();
    }
}
