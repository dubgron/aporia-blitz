/* Configs */
#include <configs/all_configs.hpp>
#include <configs/animation_config.hpp>
#include <configs/camera_config.hpp>
#include <configs/shader_config.hpp>
#include <configs/texture_config.hpp>
#include <configs/window_config.hpp>

/* Inputs */
#include <inputs/all_inputs.hpp>
#include <inputs/keyboard.hpp>
#include <inputs/mouse.hpp>

/* Utils */
#include <utils/color_conversion.hpp>
#include <utils/math.hpp>
#include <utils/random.hpp>
#include <utils/read_file.hpp>
#include <utils/shader_parser.hpp>
#include <utils/timer.hpp>
#include <utils/type_traits.hpp>

/* Logger */
#include <log_buffer.hpp>
#include <logger.hpp>

/* Graphics */
#include <graphics/font.hpp>
#include <graphics/framebuffer.hpp>
#include <graphics/image.hpp>
#include <graphics/index_buffer.hpp>
#include <graphics/light.hpp>
#include <graphics/render_queue.hpp>
#include <graphics/renderer.hpp>
#include <graphics/shader.hpp>
#include <graphics/shader_properties.hpp>
#include <graphics/uniform_buffer.hpp>
#include <graphics/vertex.hpp>
#include <graphics/vertex_array.hpp>
#include <graphics/vertex_buffer.hpp>

/* Animations */
#include <graphics/animations/animation.hpp>
#include <graphics/animations/animator.hpp>

/* Drawables */
#include <graphics/drawables/circle2d.hpp>
#include <graphics/drawables/group.hpp>
#include <graphics/drawables/line2d.hpp>
#include <graphics/drawables/rectangle2d.hpp>
#include <graphics/drawables/sprite.hpp>
#include <graphics/drawables/text.hpp>

/* Camera */
#include <graphics/camera.hpp>
#include <graphics/camera_controller.hpp>

/* Layers */
#include <layers/imgui_layer.hpp>
#include <layers/layer.hpp>
#include <layers/layer_stack.hpp>

/* Scenes */
#include <scene.hpp>

/* Window */
#include <window.hpp>

/* Managers */
#include <animation_manager.hpp>
#include <config_manager.hpp>
#include <event_manager.hpp>
#include <font_manager.hpp>
#include <input_manager.hpp>
#include <scene_manager.hpp>
#include <shader_manager.hpp>
#include <texture_manager.hpp>

/* Components */
#include <components/circular.hpp>
#include <components/color.hpp>
#include <components/linear2d.hpp>
#include <components/rectangular.hpp>
#include <components/texture.hpp>
#include <components/transform2d.hpp>

/* Platform */
#include <platform/opengl.hpp>
#include <platform/opengl_common.hpp>

/* Game */
#include <game.hpp>

/* Entry Point */
#include <entry_point.hpp>
