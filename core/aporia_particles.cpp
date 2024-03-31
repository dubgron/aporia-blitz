#include "aporia_particles.hpp"

#include "aporia_debug.hpp"
#include "aporia_rendering.hpp"

namespace Aporia
{
    Emitter emitter_create(MemoryArena* arena, i64 max_particles)
    {
        Emitter result;
        result.max_particles = max_particles;
        result.particles = arena_push<Particle>(arena, max_particles);
        return result;
    }

    void emitter_update(Emitter* emitter, f32 delta_time)
    {
        f32 emittion_rate = 1.f / emitter->particles_per_second;

        emitter->time_since_last_particle_emitted += delta_time;
        while (emitter->time_since_last_particle_emitted >= emittion_rate)
        {
            emitter->time_since_last_particle_emitted -= emittion_rate;

            particles_create(emitter);
        }
    }

    void particles_create(Emitter* emitter)
    {
        if (emitter->particles_count >= emitter->max_particles)
        {
            APORIA_LOG(Warning, "Can't emit new particle! Consider making the emitter bigger.");
            return;
        }

        Particle* particle = &emitter->particles[emitter->particles_count];
        emitter->particles_count += 1;
        *particle = Particle{};

        particle->life = random_range(emitter->life_min, emitter->life_max);

        particle->position = emitter->initial_position;

        v2 velocity_direction = emitter->velocity_direction;

        if (emitter->direction_spread > 0.f)
        {
            f32 atan = atan2(emitter->velocity_direction.y, emitter->velocity_direction.x);
            atan += M_PI * random_range(-emitter->direction_spread, emitter->direction_spread);
            velocity_direction = v2{ cos(atan), sin(atan) };
        }

        f32 speed = random_range(emitter->velocity_min, emitter->velocity_max);
        particle->velocity = velocity_direction * speed;

        f32 acceleration = random_range(emitter->acceleration_min, emitter->acceleration_max);
        particle->acceleration = emitter->acceleration_direction * acceleration;

        f32 size = random_range(emitter->size_min, emitter->size_max);
        particle->size.x = particle->size.y = size;

        // @NOTE(dubgron): Not sure, if it should be here.
        // Maybe this should be a responsibility of the caller.
        if (emitter->hue_min > emitter->hue_max)
        {
            i64 unwrap = ((emitter->hue_min - emitter->hue_max) / 360) + 1;
            emitter->hue_max += 360 * unwrap;
        }

        i64 hue = random_range(emitter->hue_min, emitter->hue_max);
        f32 saturation = random_range(emitter->saturation_min, emitter->saturation_max);
        f32 brightness = random_range(emitter->brightness_min, emitter->brightness_max);
        particle->color = hsv_to_rgb(hue, saturation, brightness);
    }

    void particles_simulate(Emitter* emitter, f32 delta_time)
    {
        for (i64 idx = 0; idx < emitter->particles_count; ++idx)
        {
            Particle* particle = &emitter->particles[idx];

            particle->velocity += particle->acceleration * delta_time;
            particle->position += particle->velocity * delta_time;

            if (particle->life > delta_time)
            {
                f32 progress = delta_time / particle->life;

                if (emitter->flags & EmitterFlag_FadeOut)
                {
                    u8 step = particle->color.a * progress;
                    particle->color.a -= step;
                }

                if (emitter->flags & EmitterFlag_FizzleOut)
                {
                    particle->size *= (1.f - progress);
                }
            }
            else
            {
                emitter->particles[idx] = emitter->particles[emitter->particles_count - 1];
                emitter->particles_count -= 1;
            }

            particle->life -= delta_time;
        }
    }

    void particles_draw(Emitter* emitter)
    {
        for (i64 idx = 0; idx < emitter->particles_count; ++idx)
        {
            Particle* particle = &emitter->particles[idx];
            draw_rectangle(particle->position - particle->size / 2.f, particle->size.x, particle->size.y, particle->color);
        }
    }
}
