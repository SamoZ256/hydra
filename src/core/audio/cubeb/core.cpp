#include "core/audio/cubeb/core.hpp"

#include "core/audio/cubeb/stream.hpp"

namespace hydra::audio::cubeb {

Core::Core() {
    const auto res = cubeb_init(&context, "Hydra", nullptr);
    ASSERT(res == CUBEB_OK, Cubeb, "Failed to initialize cubeb context: {}",
           res);
}

IStream*
Core::CreateStream(PcmFormat format, u32 sample_rate, u16 channel_count,
                   buffer_finished_callback_fn_t buffer_finished_callback) {
    return new Stream(*this, format, sample_rate, channel_count,
                      buffer_finished_callback);
}

} // namespace hydra::audio::cubeb
