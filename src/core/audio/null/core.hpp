#pragma once

#include "core/audio/core.hpp"

namespace hydra::audio::null {

class Core final : public ICore {
  public:
    IStream* CreateStream(
        PcmFormat format, u32 sample_rate, u16 channel_count,
        buffer_finished_callback_fn_t buffer_finished_callback) override;
};

} // namespace hydra::audio::null
