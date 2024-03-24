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

    static void get_icursor_and_remainder(f32 cursor, i64* out_icursor, f32* out_remainder)
    {
        *out_icursor = (i64)cursor;
        *out_remainder = cursor - *out_icursor;
    }

    static void audio_thread_function(f32* buffer, i32 num_samples, i32 num_channels)
    {
        memset(buffer, 0, sizeof(f32) * num_samples * num_channels);

        constexpr f32 sample_scale = 1.f / SHRT_MAX;

        mutex_lock(&audio_mutex);

        for (i64 idx = 0; idx < active_streams_count; ++idx)
        {
            AudioStream* stream = active_streams[idx];
            AudioSource* source = stream->source;

            i64 samples_provided = 0;
            i64 samples_requested = num_samples;

            f32 final_volume = stream->volume * master_volume;

            i64 icursor; f32 remainder;
            get_icursor_and_remainder(stream->source_cursor, &icursor, &remainder);

            while (samples_provided < samples_requested)
            {
                i64 icursor0 = icursor;
                i64 icursor1 = icursor + 1;

                if (icursor1 == source->samples_count)
                {
                    icursor1 = (stream->flags & AudioFlag_Repeating) ? 0 : icursor0;
                }

                for (i64 dst_channel = 0; dst_channel < MAX_AURIO_OUTPUT_CHANNELS; ++dst_channel)
                {
                    i64 dst_index = MAX_AURIO_OUTPUT_CHANNELS * samples_provided + dst_channel;

                    i64 src_channel = min(dst_channel, (i64)source->channels - 1);
                    i64 src_index0 = icursor0 * source->channels + src_channel;
                    i64 src_index1 = icursor1 * source->channels + src_channel;

                    f32 sample0 = source->samples[src_index0];
                    f32 sample1 = source->samples[src_index1];
                    f32 sampled_value = lerp(sample0, sample1, remainder);

                    buffer[dst_index] += sampled_value * sample_scale * final_volume;
                }

                samples_provided += 1;

                stream->source_cursor += stream->playback_speed;
                get_icursor_and_remainder(stream->source_cursor, &icursor, &remainder);

                if (icursor >= source->samples_count)
                {
                    stream->source_cursor -= source->samples_count;
                    get_icursor_and_remainder(stream->source_cursor, &icursor, &remainder);

                    if (!(stream->flags & AudioFlag_Repeating))
                    {
                        active_streams[idx] = active_streams[active_streams_count - 1];
                        active_streams_count -= 1;
                        idx -= 1;
                        break;
                    }
                }
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
        {
            String audio_file = read_entire_file(temp.arena, filepath);
            stb_vorbis* audio_data = stb_vorbis_open_memory(audio_file.data, audio_file.length, nullptr, nullptr);

            source.channels = audio_data->channels;
            source.sample_rate = audio_data->sample_rate;

            source.samples_count = stb_vorbis_stream_length_in_samples(audio_data);

            APORIA_ASSERT(source.channels <= MAX_AUDIO_SOURCE_CHANNELS);
            source.samples = arena_push<i16>(arena, source.samples_count * source.channels);

            stb_vorbis_get_samples_short_interleaved(audio_data, source.channels, source.samples, source.samples_count * source.channels);

            stb_vorbis_close(audio_data);
        }
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
