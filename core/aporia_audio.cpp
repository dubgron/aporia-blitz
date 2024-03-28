#include "aporia_audio.hpp"

#define SOKOL_AUDIO_IMPL
#include "sokol_audio.h"

#define STB_VORBIS_NO_STDIO
#define STB_VORBIS_NO_PUSHDATA_API
#include "stb_vorbis.c"
#undef R
#undef C
#undef L

#include "aporia_camera.hpp"
#include "aporia_debug.hpp"
#include "aporia_utils.hpp"

namespace Aporia
{
    constexpr i64 MAX_AUDIO_SOURCES = 64;
    AudioSource audio_sources[MAX_AUDIO_SOURCES];
    i64 audio_sources_count = 0;

    constexpr i64 MAX_ACTIVE_AUDIO_STREAMS = 128;
    AudioStream* active_streams[MAX_ACTIVE_AUDIO_STREAMS];
    i64 active_streams_count = 0;

    static void get_icursor_and_remainder(f32 cursor, i64* out_icursor, f32* out_remainder)
    {
        *out_icursor = (i64)cursor;
        *out_remainder = cursor - *out_icursor;
    }

    static void calculate_spatial_sound(AudioStream* stream, f32* out_left, f32* out_right)
    {
        v2 stream_position = stream->position;
        v2 listener_position = active_camera->view.position;

        if (stream_position == listener_position)
        {
            constexpr f32 ONE_OVER_SQUARE_ROOT_OF_TWO = 0.7071067f;
            *out_left = *out_right = ONE_OVER_SQUARE_ROOT_OF_TWO;
            return;
        }

        v2 from_listener_to_stream = stream_position - listener_position;

        v2 listener_right = active_camera->view.right_vector;
        v2 listener_up = active_camera->view.up_vector;

        v2 direction = glm::normalize(from_listener_to_stream);
        direction.x = glm::dot(direction, listener_right);
        direction.y = glm::dot(direction, listener_up);

        constexpr v2 speaker_direction_front_left{ -1.f, 0.f };
        constexpr v2 speaker_direction_front_right{ 1.f, 0.f };

        *out_left = glm::dot(direction, speaker_direction_front_left);
        *out_right = glm::dot(direction, speaker_direction_front_right);

        *out_left = clamp(*out_left, 0.f, 1.f);
        *out_right = clamp(*out_right, 0.f, 1.f);

        constexpr f32 stereo_base = 0.667f;
        *out_left += stereo_base;
        *out_right += stereo_base;

        f32 distance = glm::length(from_listener_to_stream);
        if (distance < stream->inner_radius)
        {
            f32 closeness = 1.f - (distance / stream->inner_radius);

            *out_left = lerp(*out_left, 1.f, closeness);
            *out_right = lerp(*out_right, 1.f, closeness);
        }

        f32 length = sqrt(*out_left * *out_left + *out_right * *out_right);
        *out_left /= length;
        *out_right /= length;

        f32 falloff = inverse_lerp(stream->outer_radius, stream->inner_radius, distance);
        falloff = clamp(falloff, 0.f, 1.f);

        *out_left *= falloff;
        *out_right *= falloff;
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

            f32 play_direction = (stream->playback_speed < 0.f) ? -1.f : 1.f;
            if (play_direction < 0.f && stream->play_cursor == 0.f)
            {
                stream->play_cursor = source->samples_count - 1;
            }

            i64 samples_provided = 0;
            i64 samples_requested = num_samples;

            f32 final_volume = stream->volume * master_volume;

            f32 panning[2] = { 1.f, 1.f };
            if (stream->flags & AudioFlag_Spatial)
            {
                calculate_spatial_sound(stream, &panning[0], &panning[1]);
            }

            while (samples_provided < samples_requested)
            {
                i64 icursor; f32 remainder;
                get_icursor_and_remainder(stream->play_cursor, &icursor, &remainder);

                i64 icursor0 = icursor;
                i64 icursor1 = icursor + play_direction;

                if (icursor1 < 0 || icursor1 >= source->samples_count)
                {
                    if (stream->flags & AudioFlag_Looped)
                    {
                        icursor1 = wrap_around_once(icursor1, source->samples_count);
                    }
                    else
                    {
                        icursor1 = icursor0;
                    }
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

                    buffer[dst_index] += sampled_value * sample_scale * final_volume * panning[dst_channel];
                }

                samples_provided += 1;

                stream->play_cursor += stream->playback_speed;

                if (stream->play_cursor < 0.f || stream->play_cursor >= source->samples_count)
                {
                    // @NOTE(dubgron): Restore the cursor to the beginning or the end of the source.
                    stream->play_cursor = wrap_around(stream->play_cursor, (f32)source->samples_count);

                    if (!(stream->flags & AudioFlag_Looped))
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
