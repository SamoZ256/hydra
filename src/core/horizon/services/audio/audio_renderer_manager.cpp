#include "core/horizon/services/audio/audio_renderer_manager.hpp"

#include "core/horizon/kernel/process.hpp"
#include "core/horizon/kernel/transfer_memory.hpp"
#include "core/horizon/services/audio/audio_device.hpp"
#include "core/horizon/services/audio/audio_renderer.hpp"

namespace hydra::horizon::services::audio {

DEFINE_SERVICE_COMMAND_TABLE(IAudioRendererManager, 0, OpenAudioRenderer, 1,
                             GetWorkBufferSize, 2, GetAudioDeviceService, 4,
                             GetAudioDeviceServiceWithRevisionInfo)

result_t IAudioRendererManager::OpenAudioRenderer(
    RequestContext* ctx, aligned<AudioRendererParameters, 56> params,
    u64 work_buffer_size, u64 aruid,
    InHandle<HandleAttr::Copy> work_buffer_handle,
    InHandle<HandleAttr::Copy> process_handle) {
    (void)aruid;
    (void)process_handle;

    // Work buffer
    const auto work_buffer =
        ctx->process->GetHandle<kernel::TransferMemory>(work_buffer_handle);

    AddService(*ctx, new IAudioRenderer(
                         params, std::span<u8>(reinterpret_cast<u8*>(
                                                   work_buffer->GetAddress()),
                                               work_buffer_size)));
    return RESULT_SUCCESS;
}

// TODO: cleanup
result_t
IAudioRendererManager::GetWorkBufferSize(AudioRendererParameters params,
                                         u64* out_size) {
    LOG_FUNC_STUBBED(Services);

    u64 buffer_sz = align(4 * params.mix_buffer_count, 0x40u);
    buffer_sz += params.submix_count * 1024;
    buffer_sz += 0x940 * (params.submix_count + 1);
    buffer_sz += 0x3F0 * params.voice_count;
    buffer_sz += align(8 * (params.submix_count + 1), 0x10u);
    buffer_sz += align(8 * params.voice_count, 0x10u);
    buffer_sz += align((0x3C0 * (params.sink_count + params.submix_count) +
                        4 * params.sample_count) *
                           (params.mix_buffer_count + 6),
                       0x40u);

    if (IsAudioRendererFeatureSupported(AudioFeature::Splitter,
                                        params.revision)) {
        u32 count = params.submix_count + 1;
        u64 node_count = align(count, 0x40u);
        u64 node_state_buffer_sz = 4 * (node_count * node_count) +
                                   0xC * node_count + 2 * (node_count / 8);
        u64 edge_matrix_buffer_sz = 0;
        node_count = align(count * count, 0x40u);
        if (node_count >> 31 != 0) {
            edge_matrix_buffer_sz = (node_count | 7) / 8;
        } else {
            edge_matrix_buffer_sz = node_count / 8;
        }
        buffer_sz +=
            align(node_state_buffer_sz + edge_matrix_buffer_sz, 0x10ull);
    }

    buffer_sz += 0x20 * (params.effect_count + 4 * params.voice_count) + 0x50;
    if (IsAudioRendererFeatureSupported(AudioFeature::Splitter,
                                        params.revision)) {
        buffer_sz += 0xE0 * params._unknown_x2c;
        buffer_sz += 0x20 * params.splitter_count;
        buffer_sz += align(4 * params._unknown_x2c, 0x10u);
    }
    buffer_sz = align(buffer_sz, 0x40ull) + 0x170 * params.sink_count;
    u64 output_sz = buffer_sz + 0x280 * params.sink_count +
                    0x4B0 * params.effect_count +
                    ((params.voice_count * 256) | 0x40);

    if (params.unknown_x1c >= 1) {
        output_sz = align(((16 * params.sink_count + 16 * params.effect_count +
                            16 * params.voice_count + 16) +
                           0x658) *
                                  (params.unknown_x1c + 1) +
                              0xc0,
                          0x40u) +
                    output_sz;
    }
    output_sz = align(output_sz + 0x1807e, 0x1000ull);

    *out_size = output_sz;
    LOG_INFO(Services, "Audio renderer work buffer size: 0x{:x}", *out_size);

    return RESULT_SUCCESS;
}

result_t IAudioRendererManager::GetAudioDeviceService(RequestContext* ctx,
                                                      u64 aruid) {
    (void)aruid;

    AddService(*ctx, new IAudioDevice());
    return RESULT_SUCCESS;
}

result_t IAudioRendererManager::GetAudioDeviceServiceWithRevisionInfo(
    RequestContext* ctx) {
    // TODO: revision info
    AddService(*ctx, new IAudioDevice());
    return RESULT_SUCCESS;
}

} // namespace hydra::horizon::services::audio
