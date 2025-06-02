#include "core/audio/null/stream.hpp"

namespace hydra::audio::null {

void Stream::EnqueueBuffer(buffer_id_t id, sized_ptr buffer) {
    buffer_finished_callback(id);
}

} // namespace hydra::audio::null
