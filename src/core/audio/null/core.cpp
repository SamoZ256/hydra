#include "core/audio/null/core.hpp"

#include "core/audio/null/stream.hpp"

namespace hydra::audio::null {

StreamBase*
Core::CreateStream(PcmFormat format, u32 sample_rate, u16 channel_count,
                   buffer_finished_callback_fn_t buffer_finished_callback) {
    return new Stream(format, sample_rate, channel_count,
                      buffer_finished_callback);
}

} // namespace hydra::audio::null
