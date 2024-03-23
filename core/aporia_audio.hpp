#pragma once 

#include "aporia_string.hpp"
#include "aporia_types.hpp"

namespace Aporia
{
    constexpr i64 MAX_AURIO_OUTPUT_CHANNELS = 2;
    constexpr i64 MAX_AUDIO_SOURCE_CHANNELS = 2;

    struct AudioSource
    {
        i16* samples = nullptr; // Samples from all channels, not interleaved.
        i64 samples_count = 0; // Samples per channel, i.e. mono-samples.

        i32 channels = 0;
        i32 sample_rate = 0;

        String source_file;
    };

    using AudioFlag = u8;
    enum AudioFlag_ : AudioFlag
    {
        AudioFlag_None      = 0x00,
        AudioFlag_Repeating = 0x01,
    };

    struct AudioStream
    {
        AudioSource* source = nullptr;
        i64 source_cursor = 0;

        f32 volume = 1.f;

        // @TODO(dubgron): Missing features:
        // 
        // [ ] playback speed
        // [ ] playing in reverse
        // [ ] fade-in and fade-out
        // [ ] sound at location (spartial)
        //

        AudioFlag flags = AudioFlag_None;
    };

    void audio_init();
    void audio_deinit();

    i64 audio_load(MemoryArena* arena, String filepath);
    AudioStream audio_create_stream(i64 source_id);

    void audio_play(AudioStream* stream);
    void audio_stop(AudioStream* stream);
}
