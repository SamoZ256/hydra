#pragma once

#include "core/horizon/services/const.hpp"

namespace hydra::horizon::services::codec {

class IHardwareOpusDecoder : public IService {
  protected:
    result_t RequestImpl([[maybe_unused]] RequestContext& context,
                         u32 id) override;

  private:
    // Commands
    result_t
    DecodeInterleavedOld(InBuffer<BufferAttr::MapAlias> in_opus_buffer,
                         i32* out_decoded_data_size,
                         i32* out_decoded_sample_count,
                         OutBuffer<BufferAttr::MapAlias> out_pcm_buffer);

    // Impl
    result_t
    DecodeInterleavedImpl(std::optional<io::MemoryStream> in_opus_stream,
                          i32* out_decoded_data_size,
                          i32* out_decoded_sample_count,
                          std::optional<io::MemoryStream> out_pcm_stream);
};

} // namespace hydra::horizon::services::codec
