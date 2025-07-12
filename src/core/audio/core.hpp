#pragma once

#include "core/audio/stream.hpp"

namespace hydra::audio {

class ICore {
  public:
    virtual ~ICore() = default;

    virtual IStream*
    CreateStream(PcmFormat format, u32 sample_rate, u16 channel_count,
                 buffer_finished_callback_fn_t buffer_finished_callback) = 0;
};

} // namespace hydra::audio
