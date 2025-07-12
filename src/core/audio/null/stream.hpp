#pragma once

#include "core/audio/stream.hpp"

namespace hydra::audio::null {

class Stream final : public IStream {
  public:
    Stream(PcmFormat format, u32 sample_rate, u16 channel_count,
           buffer_finished_callback_fn_t buffer_finished_callback)
        : IStream(format, sample_rate, channel_count,
                  buffer_finished_callback) {}

    void Start() override { state = StreamState::Started; }
    void Stop() override { state = StreamState::Stopped; }

    void EnqueueBuffer(buffer_id_t id, sized_ptr buffer) override;
};

} // namespace hydra::audio::null
