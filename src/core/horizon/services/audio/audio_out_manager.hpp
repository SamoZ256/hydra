#pragma once

#include "core/audio/const.hpp"
#include "core/horizon/services/audio/const.hpp"
#include "core/horizon/services/const.hpp"

namespace hydra::horizon::services::audio {

class IAudioOutManager : public ServiceBase {
  protected:
    result_t RequestImpl(RequestContext& context, u32 id) override;

  private:
    // Commands
    result_t ListAudioOuts(u32* out_count,
                           OutBuffer<BufferAttr::MapAlias> out_buffer);
    result_t
    OpenAudioOut(add_service_fn_t add_service, u32 sample_rate,
                 u16 channel_count, u16 _reserved, u64 aruid,
                 InBuffer<BufferAttr::MapAlias> in_device_name_buffer,
                 u32* out_sample_rate, u32* out_channel_count,
                 PcmFormat* out_format, AudioOutState* out_state,
                 OutBuffer<BufferAttr::MapAlias> out_device_name_buffer);
    result_t ListAudioOutsAuto(u32* out_count,
                               OutBuffer<BufferAttr::AutoSelect> out_buffer);
    result_t
    OpenAudioOutAuto(add_service_fn_t add_service, u32 sample_rate,
                     u16 channel_count, u16 _reserved, u64 aruid,
                     InBuffer<BufferAttr::AutoSelect> in_device_name_buffer,
                     u32* out_sample_rate, u32* out_channel_count,
                     PcmFormat* out_format, AudioOutState* out_state,
                     OutBuffer<BufferAttr::AutoSelect> out_device_name_buffer);

    // Impl
    result_t ListAudioOutsImpl(u32* out_count, Writer writer);
    result_t OpenAudioOutImpl(add_service_fn_t add_service, u32 sample_rate,
                              u16 channel_count, u16 _reserved, u64 aruid,
                              Reader& device_name_reader, u32* out_sample_rate,
                              u32* out_channel_count, PcmFormat* out_format,
                              AudioOutState* out_state,
                              Writer& device_name_writer);
};

} // namespace hydra::horizon::services::audio
