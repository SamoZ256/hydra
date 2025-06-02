#include "core/audio/cubeb/stream.hpp"

#include "core/audio/cubeb/core.hpp"

namespace hydra::audio::cubeb {

namespace {

cubeb_sample_format to_cubeb_format(const PcmFormat format) {
    // TODO: more
    switch (format) {
    case PcmFormat::Int16:
        return CUBEB_SAMPLE_S16NE; // TODO: correct?
    case PcmFormat::Float:
        return CUBEB_SAMPLE_FLOAT32NE; // TODO: correct?
    default:
        LOG_NOT_IMPLEMENTED(Cubeb, "PCM format {}", format);
        return CUBEB_SAMPLE_S16NE;
    }
}

cubeb_channel_layout to_cubeb_layout(u16 channel_count) {
    // TODO: correct?
    return (channel_count >= 2 ? CUBEB_LAYOUT_STEREO : CUBEB_LAYOUT_MONO);
}

} // namespace

Stream::Stream(Core& core_, PcmFormat format, u32 sample_rate,
               u16 channel_count,
               buffer_finished_callback_fn_t buffer_finished_callback)
    : StreamBase(format, sample_rate, channel_count, buffer_finished_callback),
      core{core_} {
    // TODO: allow different channel counts
    ASSERT(channel_count == 2, Cubeb, "Unsupported channel count {}",
           channel_count);

    cubeb_stream_params params;
    params.format = to_cubeb_format(format);
    params.rate = sample_rate;
    params.channels = channel_count;
    params.layout = to_cubeb_layout(channel_count);
    params.prefs = CUBEB_STREAM_PREF_NONE;

    // TODO: device
    // TODO: buffer size
    CUBEB_ASSERT_SUCCESS(cubeb_stream_init(
        core.context, &stream, "Hydra stream", nullptr, nullptr, nullptr,
        &params, 512, &Stream::DataCallback, &Stream::StateCallback, this));
}

Stream::~Stream() { cubeb_stream_destroy(stream); }

void Stream::Start() { cubeb_stream_start(stream); }

void Stream::Stop() {
    // TODO: wait to finish?
    cubeb_stream_stop(stream);
}

void Stream::EnqueueBuffer(buffer_id_t id, sized_ptr buffer) {
    std::unique_lock lock(buffer_mutex);
    buffer_queue.push({id, buffer});
}

long Stream::DataCallback(cubeb_stream* stream, void* user_data,
                          const void* input_buffer, void* output_buffer,
                          long num_frames) {
    auto self = reinterpret_cast<Stream*>(user_data);

    std::unique_lock lock(self->buffer_mutex);

    // TODO: support different formats as well
    i16* output = reinterpret_cast<i16*>(output_buffer);
    for (u32 i = 0; i < num_frames * self->channel_count; i++) {
        if (self->buffer_queue.empty()) {
            // Fill the rest with silence
            memset(output, 0,
                   (num_frames * self->channel_count - i) * sizeof(i16));

            break;
        }

        const auto [buffer_id, buffer] = self->buffer_queue.front();
        const auto sample =
            reinterpret_cast<i16*>(buffer.GetPtr())[self->pos_in_buffer++];
        if (self->pos_in_buffer * sizeof(i16) >= buffer.GetSize()) {
            self->buffer_queue.pop();
            self->pos_in_buffer = 0;
            self->buffer_finished_callback((buffer_id_t)buffer.GetPtr());
        }

        output[i] = sample;
    }

    return num_frames;
}

void Stream::StateCallback(cubeb_stream* stream, void* user_data,
                           cubeb_state state) {
    auto self = reinterpret_cast<Stream*>(user_data);

    switch (state) {
    case CUBEB_STATE_STARTED:
        LOG_DEBUG(Cubeb, "Started");
        self->state = StreamState::Started;
        break;
    case CUBEB_STATE_STOPPED:
        LOG_DEBUG(Cubeb, "Stopped");
        self->state = StreamState::Stopped;
        break;
    case CUBEB_STATE_DRAINED:
        LOG_DEBUG(Cubeb, "Drained");
        self->state = StreamState::Drained;
        break;
    case CUBEB_STATE_ERROR:
        LOG_DEBUG(Cubeb, "Error");
        self->state = StreamState::Error;
        break;
    }
}

} // namespace hydra::audio::cubeb
