#pragma once 

#include "aporia_string.hpp"
#include "aporia_types.hpp"
#include "platform/aporia_os.hpp"

// @TODO(dubgron): Missing audio features:
//
// [x] Variable playback speed
// [x] Playing audio in reverse
// [x] Playing audio at location (spartial)
// [ ] Doing fade-in and fade-out
// [ ] Handling .wav files
//

constexpr i64 MAX_AURIO_OUTPUT_CHANNELS = 2;
constexpr i64 MAX_AUDIO_SOURCE_CHANNELS = 2;

struct AudioSource
{
    i16* samples = nullptr; // Samples from all channels, interleaved.
    i64 samples_count = 0; // Samples per channel, i.e. mono-samples.

    i32 channels = 0;
    i32 sample_rate = 0;

    String source_file;
};

using AudioFlags = u8;
enum AudioFlag_ : AudioFlags
{
    AudioFlag_None      = 0x00,
    AudioFlag_Looped    = 0x01,
    AudioFlag_Spatial   = 0x02,
};

struct AudioStream
{
    AudioSource* source = nullptr;
    f32 play_cursor = 0.f;

    f32 volume = 1.f;
    f32 playback_speed = 1.f;

    v2 position = v2{ 0.f };
    f32 inner_radius = 250.f;
    f32 outer_radius = 2500.f;

    AudioFlags flags = AudioFlag_None;
};

Mutex audio_mutex;

f32 master_volume = 1.f;

void audio_init();
void audio_deinit();

i64 audio_load(MemoryArena* arena, String filepath);
AudioStream audio_create_stream(i64 source_id);

void audio_play(AudioStream* stream);
void audio_stop(AudioStream* stream);
