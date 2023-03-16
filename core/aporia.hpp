#pragma once

/* Common */
#include <common.hpp>

/* Aporia Headers */
#include "aporia_config.hpp"
#include "aporia_inputs.hpp"
#include "aporia_memory.hpp"
#include "aporia_rendering.hpp"
#include "aporia_shaders.hpp"
#include "aporia_textures.hpp"
#include "aporia_types.hpp"

/* Utils */
#include <utils/color_conversion.hpp>
#include <utils/math.hpp>
#include <utils/random.hpp>
#include <utils/read_file.hpp>
#include <utils/timer.hpp>
#include <utils/type_traits.hpp>

/* Graphics */
#include <graphics/font.hpp>

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
#include <font_manager.hpp>
#include <scene_manager.hpp>

/* Components */
#include <components/circular.hpp>
#include <components/color.hpp>
#include <components/linear2d.hpp>
#include <components/rectangular.hpp>
#include <components/transform2d.hpp>

/* Platform */
#include <platform/opengl.hpp>
#include <platform/opengl_common.hpp>

/* Game */
#include <game.hpp>

/* Entry Point */
#include <entry_point.hpp>
