#include "aporia_animation.hpp"

#include <nlohmann/json.hpp>

#include "aporia_debug.hpp"
#include "aporia_entity.hpp"
#include "aporia_game.hpp"
#include "aporia_strings.hpp"
#include "aporia_textures.hpp"

namespace Aporia
{
    static std::vector<Animation> animations;

    static Animation* find_animation(std::vector<Animation>& animations, String animation_name)
    {
        for (Animation& animation : animations)
        {
            if (animation.name == animation_name)
            {
                return &animation;
            }
        }
        return nullptr;
    }

    void load_animations(String filepath)
    {
        const String data = read_file(&persistent_arena, filepath);
        APORIA_ASSERT(data.is_valid());

        using json = nlohmann::json;
        const json animation_json = json::parse<std::string_view>(data);

        for (auto& animation_data : animation_json["animations"])
        {
            const String animation_name = animation_data["name"].get<std::string_view>();

            Animation animation;
            animation.name = push_string(&persistent_arena, animation_name);

            for (auto& frame_name_str : animation_data["frames"])
            {
                AnimationFrame frame;
                const String frame_name = frame_name_str.get<std::string_view>();
                frame.texture = get_subtexture(frame_name);
                animation.frames.push_back(frame);
            }

            animations.push_back(std::move(animation));
        }
    }

    void animation_tick(Entity& entity, f32 frame_time)
    {
        Animator& animator = entity.animator;
        Animation* animation = animator.current_animation;
        if (!animation)
        {
            animator.current_animation = animator.requested_animation;
        }

        if (animation)
        {
            animator.elapsed_time += frame_time;
            while (animator.elapsed_time >= animation->frame_length)
            {
                animator.elapsed_time -= animation->frame_length;

                if (animator.requested_animation)
                {
                    animator.current_animation = animator.requested_animation;
                    animator.requested_animation = nullptr;
                }

                animation->current_frame += 1;
                if (animation->current_frame > animation->frames.size() - 1)
                {
                    animation->current_frame = 0;
                }

                entity.texture = animation->frames[animation->current_frame].texture;
            }
        }
    }

    void animation_add(Animator& animator, String animation_name)
    {
        if (Animation* animation = find_animation(animations, animation_name))
        {
            animator.animations.push_back(*animation);
        }
    }

    void animation_request(Animator& animator, String animation_name)
    {
        if (Animation* animation = find_animation(animator.animations, animation_name))
        {
            animator.requested_animation = animation;
        }
        else
        {
            APORIA_LOG(Error, "Failed to find animation '{}'!", animation_name);
        }
    }
}
