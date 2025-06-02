#pragma once

#include "core/horizon/services/const.hpp"

namespace hydra::horizon::services::codec {

class IHardwareOpusDecoderManager : public ServiceBase {
  protected:
    result_t RequestImpl(RequestContext& context, u32 id) override;

  private:
    // Commands
    // TODO: pack as u64?
    // TODO: in tmem handle
    result_t OpenHardwareOpusDecoder(add_service_fn_t add_service,
                                     i32 sample_rate, i32 channel_count,
                                     u32 work_buffer_size);
    // TODO: pack as u64?
    result_t GetWorkBufferSize(i32 sample_rate, i32 channel_count,
                               u32* out_size);
};

} // namespace hydra::horizon::services::codec
