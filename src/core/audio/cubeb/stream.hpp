#pragma once

#include "core/audio/cubeb/const.hpp"
#include "core/audio/stream_base.hpp"

struct cubeb_stream;

namespace hydra::audio::cubeb {

class Core;

class Stream final : public StreamBase {
  public:
    Stream(Core& core_, PcmFormat format, u32 sample_rate, u16 channel_count,
           buffer_finished_callback_fn_t buffer_finished_callback);
    ~Stream() override;

    void Start() override;
    void Stop() override;

    void EnqueueBuffer(buffer_id_t id, sized_ptr buffer) override;

  private:
    Core& core;

    cubeb_stream* stream;

    std::mutex buffer_mutex;
    std::queue<std::pair<buffer_id_t, sized_ptr>> buffer_queue;
    u32 pos_in_buffer;

    static long DataCallback(cubeb_stream* stream, void* user_data,
                             const void* input_buffer, void* output_buffer,
                             long num_frames);
    static void StateCallback(cubeb_stream* stream, void* user_data,
                              cubeb_state state);
};

} // namespace hydra::audio::cubeb
