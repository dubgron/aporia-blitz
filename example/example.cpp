#include <aporia.hpp>

#include <array>
#include <numeric>

using namespace Aporia;

enum class GameMode
{
    Menu,
    Play,
    End,
};

static GameMode game_mode = GameMode::Menu;

struct Vehicle
{
    EntityID id;
    v2 direction{ 0.f, 1.f };
    v2 velocity{ 0.f };
    static constexpr f32 acceleration = 20.f;
    static constexpr f32 drag = 6.5f;
    static constexpr f32 turn_rate = 0.015f;
    static constexpr f32 max_speed = 600.f;
    static constexpr f32 min_speed_to_turn = 120.f;
    static constexpr f32 reverse_acc = 12.f;
};

static bool vehicle_moves_forward()
{
    return has_been_held(Key::Up) ||
        has_been_held(Key::W) ||
        has_been_held(GamepadButton::DPadUp) ||
        get_analog_state(GamepadAxis::LeftStickY).min_value > 0.f;
}

static bool vehicle_moves_backward()
{
    return has_been_held(Key::Down) ||
        has_been_held(Key::S) ||
        has_been_held(GamepadButton::DPadDown) ||
        get_analog_state(GamepadAxis::LeftStickY).max_value < 0.f;
}

static bool vehicle_turns_left()
{
    return has_been_held(Key::Left) ||
        has_been_held(Key::A) ||
        has_been_held(GamepadButton::DPadLeft) ||
        get_analog_state(GamepadAxis::LeftStickX).max_value < 0.f;
}

static bool vehicle_turns_right()
{
    return has_been_held(Key::Right) ||
        has_been_held(Key::D) ||
        has_been_held(GamepadButton::DPadRight) ||
        get_analog_state(GamepadAxis::LeftStickX).max_value > 0.f;
}

static Vehicle player;
static u32 vehicle_shader;

#define EPS 0.01f

static constexpr f32 PI_2 = (f32)M_PI_2;

static const v2 street_lamp_pos[] =
{
    v2{ -752.f, -836.f },
    v2{ -12.f, -832.f },
    v2{ 630.f, -834.f },
    v2{ 632.f, -496.f },
    v2{ 300.f, -278.f },
    v2{ 296.f, 172.f },
    v2{ 298.f, 588.f },
    v2{ 784.f, 586.f },
    v2{ -758.f, 588.f },
    v2{ -756.f, 150.f },
    v2{ -754.f, -294.f },
    v2{ -212.f, 144.f },
    v2{ -88.f, -604.f },
    v2{ -260.f, 590.f },
    v2{ 1018.f, 70.f }
};

static constexpr u64 street_lamp_count = sizeof(street_lamp_pos) / sizeof(v2);
static LightSource street_lamp_lights[street_lamp_count];

static u64 added_lights = 0;

// @TODO(dubgron): Collisions!!!
struct CollisionBox
{
    v2 top_left{ 0.f };
    v2 bottom_right{ 0.f };
};

static CollisionBox collisions[] = {
    { v2{ -1200.f, 1200.f },    v2{ -846.f, 694.f } },
    { v2{ -660.f, 1200.f },     v2{ 1200.f, 696.f } },
    { v2{ -1200.f, 504.f },     v2{ -852.f, -1200.f } },
    { v2{ -662.f, 508.f },      v2{ 202.f, 174.f } },
    { v2{ -662.f, 508.f },      v2{ -416.f, -526.f } },
    { v2{ -662.f, -202.f },     v2{ -172.f, -530.f } },
    { v2{ -8.f, 504.f },        v2{ 202.f, -738.f } },
    { v2{ -74.f, -202.f },      v2{ 202.f, -746.f } },
    { v2{ -662.f, -622.f },     v2{ 540.f, -748.f } },
    { v2{ -72.f, -582.f },      v2{ 538.f, -744.f } },
    { v2{ 388.f, 506.f },       v2{ 1200.f, 124.f } },
    { v2{ 390.f, 26.f },        v2{ 1200.f, -404.f } },
    { v2{ 728.f, -594.f },      v2{ 1200.f, -1200.f } },
    { v2{ -662.f, -930.f },     v2{ 978.f, -1200.f } },
    { v2{ -1200.f, 1200.f },    v2{ 1200.f, 1024.f } },
    { v2{ -1200.f, 1200.f },    v2{ -1024.f, -1200.f } },
    { v2{ -1200.f, -1024.f },   v2{ 1200.f, -1200.f } },
    { v2{ 1024.f, 1200.f },     v2{ 1200.f, -1200.f } }
};
static constexpr u64 max_collisions = sizeof(collisions) / sizeof(CollisionBox);

static u64 added_collisions = 0;

bool place_meeting(f32 x, f32 y, f32 width, f32 height)
{
    for (u64 idx = 0; idx < max_collisions; ++idx)
    {
        const v2 position{ x, y };
        const f32 dist_to_top = collisions[idx].top_left.y - position.y;
        const f32 dist_to_bottom = position.y - collisions[idx].bottom_right.y;
        const f32 dist_to_left = position.x - collisions[idx].top_left.x;
        const f32 dist_to_right = collisions[idx].bottom_right.x - position.x;

        const bool below_top = dist_to_top > EPS;
        const bool above_bottom = dist_to_bottom > EPS;
        const bool inside_from_left = dist_to_left > EPS;
        const bool inside_from_right = dist_to_right > EPS;

        if (below_top && above_bottom && inside_from_left && inside_from_right)
        {
            return true;
        }
    }

    return false;
}

static f32 sign(f32 value)
{
    return value > 0.f ? 1.f : (value < 0.f ? -1.f : 0.f);
}


static v2 delivery_points[5] = {
    v2{ -754.f, 592.f },
    v2{ 630.f, -828.f },
    v2{ 296.f, -286.f },
    v2{ 824.f, 66.f },
    v2{ -228.f, -122.f }
};
static constexpr u64 delivery_points_count = sizeof(delivery_points) / sizeof(v2);
static bool collected_delivery[delivery_points_count] = { false };
static f32 delivery_point_range = 50.f;
static u64 added_delivery_points = 0;

static f32 start_time = 0.f;
static f32 end_time = 0.f;

static const Font* font;

static void set_entities_visible(bool visible)
{
    for (u64 idx = 0; idx < world.entity_count; ++idx)
    {
        if (visible)
        {
            set_flag(world.entity_array[idx], EntityFlag_Visible);
        }
        else
        {
            unset_flag(world.entity_array[idx], EntityFlag_Visible);
        }
    }
}

static v2_u32 parse_seconds(f32 seconds)
{
    u32 minutes = 0.f;
    while (seconds >= 60.f)
    {
        minutes += 1.f;
        seconds -= 60.f;
    }
    return v2_u32{ minutes, (u32)seconds };
}

void play_update(f32 time, f32 delta_time)
{
    if (Entity* player_entity = world.get_entity(player.id))
    {
        f32 velocity_len = length(player.velocity);

        if (vehicle_moves_forward())
        {
            player.velocity += player.direction * player.acceleration;
            velocity_len = length(player.velocity);
            if (velocity_len > player.max_speed)
            {
                player.velocity = player.velocity / velocity_len * player.max_speed;
            }
        }
        if (vehicle_moves_backward())
        {
            player.velocity -= player.direction * player.reverse_acc;
            velocity_len = length(player.velocity);
            if (velocity_len > player.max_speed / 2.f)
            {
                player.velocity = player.velocity / velocity_len * player.max_speed / 2.f;
            }
        }

        f32& x = player_entity->position.x;
        f32& y = player_entity->position.y;

        v2 move = player.velocity * delta_time;
        if (move.x == 0.f && move.y == 0.f) move = player.direction * delta_time;
        const v2 sim_pos = v2{ x, y } + move;
        if (place_meeting(sim_pos.x, sim_pos.y, player_entity->width, player_entity->height))
        {
            const v2 nudge = normalize(move) / v2{ active_window->get_size() };
            //const v2 nudge = normalize(move) * 0.2f;5
            while (!place_meeting(x + nudge.x, y + nudge.y, player_entity->width, player_entity->height))
            {
                x += nudge.x * 2.f;
                y += nudge.y * 2.f;
            }

            player.velocity = v2{ 0.f };
            velocity_len = 0.f;
        }

        if (velocity_len > player.drag)
        {
            player.velocity -= player.velocity / velocity_len * player.drag;

            if (vehicle_turns_left())
            {
                if (dot(player.velocity, player.direction) > 0.f)
                {
                    const v2 turn = v2{ -player.direction.y, player.direction.x };
                    player.direction += turn * player.turn_rate;
                    player.direction /= length(player.direction);
                }
                else
                {
                    const v2 turn = v2{ player.direction.y, -player.direction.x };
                    player.direction += turn * player.turn_rate;
                    player.direction /= length(player.direction);
                }
            }
            if (vehicle_turns_right())
            {
                if (dot(player.velocity, player.direction) > 0.f)
                {
                    const v2 turn = v2{ player.direction.y, -player.direction.x };
                    player.direction += turn * player.turn_rate;
                    player.direction /= length(player.direction);
                }
                else
                {
                    const v2 turn = v2{ -player.direction.y, player.direction.x };
                    player.direction += turn * player.turn_rate;
                    player.direction /= length(player.direction);
                }
            }
        }
        else
        {
            player.velocity = v2{ 0.f };
        }

        player_entity->position += player.velocity * delta_time;
        player_entity->rotation = atan2(player.direction.y, player.direction.x) - PI_2;

        bool finished = true;

        for (u64 idx = 0; idx < delivery_points_count; ++idx)
        {
            if (!collected_delivery[idx])
            {
                if (distance(delivery_points[idx], player_entity->position) < delivery_point_range*1.5f)
                {
                    collected_delivery[idx] = true;
                }
                else
                {
                    finished = false;
                }
            }
        }

        if (finished)
        {
            game_mode = GameMode::End;
            set_entities_visible(false);
            end_time = time;

            player_entity->position = v2{ -752.f, -836.f };
            player.direction = v2{ 0.f, 1.f };
            player.velocity = v2{ 0.f };
            for (u64 idx = 0; idx < delivery_points_count; ++idx)
            {
                collected_delivery[idx] = false;
            }
        }
    }

    for (u64 idx = 0; idx < street_lamp_count; idx += 1)
    {
        street_lamp_lights[idx].range = 0.2f + sin(time * 5.f) * 0.01f;
    }
}

static void play_draw(f32 time, f32 frame_time)
{
    for (u64 idx = 0; idx < street_lamp_count; idx += 1)
    {
        add_light_source(street_lamp_lights[idx]);
    }

    //add_light_source(LightSource{ active_window->get_mouse_position() });

    bind_shader(vehicle_shader);
    const m4& view_projection_matrix = active_camera->calculate_view_projection_matrix();
    shader_set_mat4("u_vp_matrix", view_projection_matrix);
    shader_set_uint("u_num_lights", light_source_count());
    shader_set_float2("u_forward", player.direction);
    std::array<i32, OPENGL_MAX_TEXTURE_UNITS> sampler{};
    std::iota(sampler.begin(), sampler.end(), 0);
    shader_set_int_array("u_atlas", sampler.data(), OPENGL_MAX_TEXTURE_UNITS);

    bind_shader(circle_shader);
    shader_set_float("u_time", time);
    unbind_shader();

    for (u64 i = 0; i < delivery_points_count; ++i)
    {
        if (!collected_delivery[i])
        {
            draw_circle(delivery_points[i], delivery_point_range, Aporia::Color::Yellow);
        }
    }

    v2_u32 minutes_seconds = parse_seconds(time - start_time);
    Text timer;
    timer.caption = fmt::format("{:02}:{:02}", minutes_seconds.x, minutes_seconds.y);
    timer.font = font;
    timer.position = v2{ 700.f, 900.f };
    timer.z = 0.5f;
    timer.color = Aporia::Color::White;
    timer.shader_id = font_shader;
    timer.font_size = 100.f;
    draw_text(timer);

#if 0
    for (u64 idx = 0; idx < max_collisions; ++idx)
    {
        const f32 width = collisions[idx].bottom_right.x - collisions[idx].top_left.x;
        const f32 height = collisions[idx].top_left.y - collisions[idx].bottom_right.y;
        draw_rectangle(collisions[idx].top_left - v2{ 0.f, height }, width, height, Aporia::Color::Cyan);
    }
#endif
}

void game_init()
{
    enable_lighting();
    load_texture_atlas("content/vehicle.json");
    vehicle_shader = create_shader("content/shaders/vehicle.glsl");

    Entity* player_entity;
    player.id = world.create_entity(&player_entity);
    player_entity->position = v2{ -752.f, -836.f };
    player_entity->texture = get_subtexture("vehicle_albedo");
    player_entity->width = 64.f;
    player_entity->height = 128.f;
    player_entity->shader_id = vehicle_shader;
    player_entity->center_of_rotation = v2{ 0.5f };
    unset_flag(*player_entity, EntityFlag_BlockingLight);

    for (u64 idx = 0; idx < street_lamp_count; ++idx)
    {
        street_lamp_lights[idx].origin = street_lamp_pos[idx];
        street_lamp_lights[idx].color = v3(0.95, 0.75, 0.35);
        street_lamp_lights[idx].range = 0.1f;
        street_lamp_lights[idx].intensity = 0.75f;
    }

    load_texture_atlas("content/streets.json");
    Entity* streets;
    world.create_entity(&streets);
    streets->texture = get_subtexture("streets");
    streets->width = streets->height = 2048.f;
    streets->center_of_rotation = v2{ 0.5f };
    streets->z = -0.5f;
    unset_flag(*streets, EntityFlag_BlockingLight);

    load_texture_atlas("content/buildings.json");
    Entity* buildings;
    world.create_entity(&buildings);
    buildings->texture = get_subtexture("buildings");
    buildings->width = buildings->height = 2048.f;
    buildings->center_of_rotation = v2{ 0.5f };
    buildings->z = 0.25f;

    load_font(create_string("arial"), "content/fonts/arial");
    font = &get_font(create_string("arial"));

    set_entities_visible(false);
}

void game_update(f32 time, f32 delta_time)
{
    switch (game_mode)
    {
        case GameMode::Menu:
        {
            if (has_been_pressed(Key::Space))
            {
                start_time = time;
                game_mode = GameMode::Play;
                set_entities_visible(true);
            }
        }
        break;

        case GameMode::Play:
        {
            play_update(time, delta_time);
        }
        break;

        case GameMode::End:
        {
            if (has_been_pressed(Key::Space))
            {
                game_mode = GameMode::Menu;
            }
        }
        break;
    }
    //if (added_lights < street_lamp_count && has_been_pressed(MouseButton::Left))
    //{
    //    street_lamp_lights[added_lights].origin = active_window->get_mouse_position();
    //    added_lights += 1;
    //}
    //if (has_been_pressed(Key::R))
    //{
    //    street_lamp_lights[added_lights].origin = v2{ -1000.f, -1000.f };
    //    added_lights -= 1;
    //}

    //static bool adding_second = false;

    //if (added_collisions < max_collisions && has_been_pressed(MouseButton::Left))
    //{
    //    if (!adding_second)
    //    {
    //        collisions[added_collisions].top_left = active_window->get_mouse_position();
    //        adding_second = true;
    //    }
    //    else
    //    {
    //        collisions[added_collisions].bottom_right = active_window->get_mouse_position();
    //        adding_second = false;
    //        added_collisions += 1;
    //    }
    //}
    //if (has_been_pressed(Key::R))
    //{
    //    added_collisions -= 1;
    //    adding_second = false;
    //}

    //if (added_delivery_points < delivery_points_count && has_been_pressed(MouseButton::Left))
    //{
    //    delivery_points[added_delivery_points] = active_window->get_mouse_position();
    //    added_delivery_points += 1;
    //}
    //if (has_been_pressed(Key::R))
    //{
    //    delivery_points[added_delivery_points] = v2{ -1000.f, -1000.f };
    //    added_lights -= 1;
    //}
}

void game_draw(f32 time, f32 frame_time)
{
    switch (game_mode)
    {
        case GameMode::Menu:
        {
            Text text;
            text.caption = "Ride";
            text.font = font;
            text.position = v2{ -300.f, 0.f };
            text.font_size = 300.f;
            draw_text(text);

            text.caption = "Press SPACE to start.";
            text.position = v2{ -500.f, -400.f };
            text.font_size = 100.f;
            draw_text(text);

            text.caption = "Made by dubgron";
            text.position = v2{ -200.f, 500.f };
            text.font_size = 50.f;
            draw_text(text);
        }
        break;

        case GameMode::Play:
        {
            play_draw(time, frame_time);
        }
        break;

        case GameMode::End:
        {
            Text text;
            text.caption = fmt::format("Congratulations!");
            text.font = font;
            text.position = v2{ -700.f, 0.f };
            text.font_size = 200.f;
            draw_text(text);

            v2_u32 minutes_seconds = parse_seconds(end_time - start_time);
            if (minutes_seconds.x > 0)
            {
                text.caption = fmt::format("Your score: {} minutes and {} seconds!", minutes_seconds.x, minutes_seconds.y);
                text.position = v2{ -650.f, -200.f };
            }
            else
            {
                text.caption = fmt::format("Your score: {} seconds!", minutes_seconds.y);
                text.position = v2{ -350.f, -200.f };
            }
            text.font_size = 75.f;
            draw_text(text);
        }
        break;
    }
}

void game_terminate()
{
    //for (u64 i = 0; i < added_lights; ++i)
    //{
    //    printf("v2{ %f, %f },\n", street_lamp_lights[i].origin.x, street_lamp_lights[i].origin.y);
    //}
    //int k = 10;

    //for (u64 i = 0; i < added_collisions; ++i)
    //{
    //    printf("{ v2{ %f, %f },", collisions[i].top_left.x, collisions[i].top_left.y);
    //    printf(" v2{ %f, %f } },\n", collisions[i].bottom_right.x, collisions[i].bottom_right.y);
    //}
    //int k = 10;

    //for (u64 i = 0; i < delivery_points_count; ++i)
    //{
    //    printf("v2{ %f, %f },\n", delivery_points[i].x, delivery_points[i].y);
    //}
    //APORIA_BREAKPOINT();
}

Aporia::Game create_game()
{
    Aporia::Game game;

    game.init               = game_init;
    game.update             = game_update;
    game.draw               = game_draw;
    game.terminate          = game_terminate;
    game.config_filepath    = "content/config.json";

    return game;
}
