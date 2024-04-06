#pragma once

#include "aporia_memory.hpp"
#include "aporia_types.hpp"
#include "aporia_utils.hpp"

struct Particle
{
    v2 position{ 0.f };
    v2 velocity{ 0.f };
    v2 acceleration{ 0.f };

    f32 life = 0.f;

    v2 size{ 1.f };
    Color color;
};

using EmitterFlag = u32;
enum EmitterFlag_ : EmitterFlag
{
    EmitterFlag_None        = 0x00,
    EmitterFlag_FadeOut     = 0x01,
    EmitterFlag_FizzleOut   = 0x02,
};

// @TODO(dubgron): Add emitters that fire only once.

struct Emitter
{
    Particle* particles = nullptr;
    i64 max_particles = 0;
    i64 particles_count = 0;

    f32 time_since_last_particle_emitted = 0.f;

    // @TODO(dubgron): The settings below should probably
    // be in a separate struct, so they can be reusable by
    // many emitters without a need to duplicate this data
    // in all of them.

    EmitterFlag flags = EmitterFlag_None;

    f32 particles_per_second = 0.f;

    f32 life_min = 0.0f, life_max = 0.f;

    v2 initial_position{ 0.f };

    v2 velocity_direction{ 0.f };
    f32 velocity_min = 0.f, velocity_max = 0.f;
    f32 direction_spread = 0.f;

    v2 acceleration_direction{ 0.f };
    f32 acceleration_min = 0.f, acceleration_max = 0.f;

    f32 size_min = 5.f, size_max = 5.f;

    i32 hue_min = 0, hue_max = 0;
    f32 saturation_min = 0.f, saturation_max = 0.f;
    f32 brightness_min = 1.f, brightness_max = 1.f;
};

Emitter emitter_create(MemoryArena* arena, i64 max_particles);
void emitter_update(Emitter* emitter, f32 delta_time);
    
void particles_create(Emitter* emitter);
void particles_simulate(Emitter* emitter, f32 delta_time);
void particles_draw(Emitter* emitter);
