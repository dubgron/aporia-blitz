#include "renderer.hpp"

#include <array>
#include <cmath>
#include <functional>

#include <SFML/Graphics/RenderStates.hpp>
#include <SFML/Graphics/Vertex.hpp>
#include <SFML/System/Vector2.hpp>

namespace Aporia
{
    Renderer::Renderer(Logger& logger)
        : _logger(logger)
    {
    }

    void Renderer::render(Window& window, const Camera& camera)
    {
        sf::RenderStates states(camera.get_view_projection_matrix());

        for (auto& [texture, vertex_array] : _queue)
        {
            states.texture = texture.get();

            window.draw(vertex_array, states);
            vertex_array.clear();
        }
    }
}
