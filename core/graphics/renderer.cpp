#include "renderer.hpp"

#include <SFML/Graphics/RenderStates.hpp>

namespace Aporia
{
    Renderer::Renderer(Logger& logger)
        : _logger(logger)
    {
        _tranformation_stack.emplace();
    }

    void Renderer::render(Window& window, const Camera& camera)
    {
        sf::RenderStates states(camera.get_view_projection_matrix());

        for (auto& [texture, vertex_array] : _textured_queue)
        {
            states.texture = texture.get();

            window.draw(vertex_array, states);

            states.texture = nullptr;
            vertex_array.clear();
        }

        window.draw(_circle_queue, states);
        _circle_queue.clear();

        window.draw(_quad_queue, states);
        _quad_queue.clear();

        window.draw(_line_queue, states);
        _line_queue.clear();
    }

    void Renderer::push_transform(Transform2D transform)
    {
        _tranformation_stack.push(std::move(_tranformation_stack.top() * transform));
    }

    void Renderer::pop_transform()
    {
        if (_tranformation_stack.size() > 1)
            _tranformation_stack.pop();
    }
}
