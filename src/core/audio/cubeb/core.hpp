#pragma once

#include "core/audio/core_base.hpp"
#include "core/audio/cubeb/const.hpp"

namespace hydra::audio::cubeb {

class Core final : public CoreBase {
    friend class Stream;

  public:
    Core();

    StreamBase* CreateStream(
        PcmFormat format, u32 sample_rate, u16 channel_count,
        buffer_finished_callback_fn_t buffer_finished_callback) override;

  private:
    struct cubeb* context;
};

} // namespace hydra::audio::cubeb
