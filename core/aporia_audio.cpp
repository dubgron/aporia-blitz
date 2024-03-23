#include "aporia_audio.hpp"

#define SOKOL_AUDIO_IMPL
#include "sokol_audio.h"

#define STB_VORBIS_NO_STDIO
#define STB_VORBIS_NO_PUSHDATA_API
#include "stb_vorbis.c"
#undef R
#undef C
#undef L

#include "aporia_debug.hpp"
#include "aporia_utils.hpp"
#include "platform/aporia_os.hpp"

namespace Aporia
{
    Mutex audio_mutex;

    constexpr i64 MAX_AUDIO_SOURCES = 64;
    AudioSource audio_sources[MAX_AUDIO_SOURCES];
    i64 audio_sources_count = 0;

    constexpr i64 MAX_ACTIVE_AUDIO_STREAMS = 128;
    AudioStream* active_streams[MAX_ACTIVE_AUDIO_STREAMS];
    i64 active_streams_count = 0;

    f32 master_volume = 1.f;

    static void audio_thread_function(f32* buffer, i32 num_samples, i32 num_channels)
    {
        memset(buffer, 0, sizeof(f32) * num_samples * num_channels);

        constexpr f32 sample_scale = 1.f / SHRT_MAX;

        mutex_lock(&audio_mutex);

        for (i64 idx = 0; idx < active_streams_count; ++idx)
        {
            AudioStream* stream = active_streams[idx];
            AudioSource* source = stream->source;

            i64 samples_requested = num_samples;
            f32 final_volume = stream->volume * master_volume;

            while (samples_requested > 0)
            {
                i64 samples_left = source->samples_count - stream->source_cursor;
                i64 samples_provided = min(samples_requested, samples_left);

                for (i64 sample = 0; sample < samples_provided; sample++)
                {
                    for (i64 dst_channel = 0; dst_channel < MAX_AURIO_OUTPUT_CHANNELS; ++dst_channel)
                    {
                        i64 dst_index = MAX_AURIO_OUTPUT_CHANNELS * sample + dst_channel;

                        i64 src_channel = min(dst_channel, (i64)source->channels);
                        i64 src_index = stream->source_cursor + src_channel * source->samples_count;

                        buffer[dst_index] += source->samples[src_index] * sample_scale * final_volume;
                    }

                    stream->source_cursor += 1;
                }

                if (stream->source_cursor == source->samples_count)
                {
                    stream->source_cursor = 0;

                    if (!(stream->flags & AudioFlag_Repeating))
                    {
                        active_streams[idx] = active_streams[active_streams_count - 1];
                        active_streams_count -= 1;
                        idx -= 1;
                        break;
                    }
                }

                samples_requested -= samples_provided;
            }
        }

        mutex_unlock(&audio_mutex);
    }

    void audio_init()
    {
        audio_mutex = mutex_create();

        saudio_desc desc = {};
        desc.num_channels = 2;
        desc.stream_cb = audio_thread_function;
        saudio_setup(&desc);
    }

    void audio_deinit()
    {
        saudio_shutdown();
        mutex_destroy(&audio_mutex);
    }

    i64 audio_load(MemoryArena* arena, String filepath)
    {
        AudioSource source;
        source.source_file = filepath;

        ScratchArena temp = scratch_begin(arena);

        String audio_file = read_entire_file(temp.arena, filepath);
        stb_vorbis* audio_data = stb_vorbis_open_memory(audio_file.data, audio_file.length, nullptr, nullptr);

        source.channels = audio_data->channels;
        source.sample_rate = audio_data->sample_rate;

        source.samples_count = stb_vorbis_stream_length_in_samples(audio_data);

        APORIA_ASSERT(source.channels <= MAX_AUDIO_SOURCE_CHANNELS);
        source.samples = arena_push<i16>(arena, source.samples_count * source.channels);

        i16* samples[MAX_AUDIO_SOURCE_CHANNELS] = { nullptr };
        for (i64 channel = 0; channel < source.channels; ++channel)
        {
            i64 channel_offset = source.samples_count * channel;
            samples[channel] = &source.samples[channel_offset];
        }
        stb_vorbis_get_samples_short(audio_data, source.channels, samples, source.samples_count);

        stb_vorbis_close(audio_data);

        scratch_end(temp);

        APORIA_ASSERT(audio_sources_count < MAX_AUDIO_SOURCES);
        i64 result = audio_sources_count;
        audio_sources[result] = source;
        audio_sources_count += 1;

        return result;
    }

    AudioStream audio_create_stream(i64 source_id)
    {
        AudioStream result;
        result.source = &audio_sources[source_id];
        return result;
    }

    void audio_play(AudioStream* stream)
    {
        if (active_streams_count < MAX_ACTIVE_AUDIO_STREAMS)
        {
            mutex_lock(&audio_mutex);

            active_streams[active_streams_count] = stream;
            active_streams_count += 1;

            mutex_unlock(&audio_mutex);
        }
    }

    void audio_stop(AudioStream* stream)
    {
        for (i64 idx = 0; idx < active_streams_count; ++idx)
        {
            if (active_streams[idx] == stream)
            {
                mutex_lock(&audio_mutex);

                active_streams[idx] = active_streams[active_streams_count - 1];
                active_streams_count -= 1;

                mutex_unlock(&audio_mutex);

                break;
            }
        }
    }
}
