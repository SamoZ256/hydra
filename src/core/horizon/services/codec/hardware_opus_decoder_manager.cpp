#include "core/horizon/services/codec/hardware_opus_decoder_manager.hpp"

#include "core/horizon/services/codec/hardware_opus_decoder.hpp"

namespace hydra::horizon::services::codec {

DEFINE_SERVICE_COMMAND_TABLE(IHardwareOpusDecoderManager, 0,
                             openHardwareOpusDecoder, 1, getWorkBufferSize)

result_t IHardwareOpusDecoderManager::openHardwareOpusDecoder(
    RequestContext* ctx, i32 sample_rate, i32 channel_count,
    u32 work_buffer_size) {
    // TODO: params
    (void)sample_rate;
    (void)channel_count;
    (void)work_buffer_size;
    LOG_DEBUG(Services,
              "Sample rate: {}, channel count: {}, work buffer size: 0x{:x}",
              sample_rate, channel_count, work_buffer_size);

    addService(*ctx, new IHardwareOpusDecoder());
    return RESULT_SUCCESS;
}

result_t IHardwareOpusDecoderManager::getWorkBufferSize(i32 sample_rate,
                                                        i32 channel_count,
                                                        u32* out_size) {
    LOG_FUNC_WITH_ARGS_STUBBED(Services, "sample rate: {}, channel count: {}",
                               sample_rate, channel_count);

    // HACK
    *out_size = 0x20000;
    return RESULT_SUCCESS;
}

} // namespace hydra::horizon::services::codec
