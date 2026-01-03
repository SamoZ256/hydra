#pragma once

#include "core/audio/const.hpp"

namespace hydra::audio {

using buffer_id_t = u64;

typedef std::function<void(buffer_id_t)> buffer_finished_callback_fn_t;

class IStream {
  public:
    enum class Error {
        InitializationFailed,
    };

    IStream(PcmFormat format_, u32 sample_rate_, u16 channel_count_,
            buffer_finished_callback_fn_t buffer_finished_callback_)
        : format{format_}, sample_rate{sample_rate_},
          channel_count{channel_count_}, buffer_finished_callback{
                                             buffer_finished_callback_} {}
    virtual ~IStream() {}

    virtual void Start() = 0;
    virtual void Stop() = 0;

    virtual void EnqueueBuffer(buffer_id_t id, sized_ptr buffer) = 0;

    StreamState GetState() const { return state; }

  protected:
    PcmFormat format;
    u32 sample_rate;
    u16 channel_count;
    buffer_finished_callback_fn_t buffer_finished_callback;

    std::atomic<StreamState> state{StreamState::Stopped};
};

} // namespace hydra::audio
