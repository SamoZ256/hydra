#pragma once

#include "core/horizon/services/audio/const.hpp"
#include "core/horizon/services/audio/internal/mempool.hpp"
#include "core/horizon/services/const.hpp"

namespace hydra::horizon::services::audio::internal {

class Renderer {
  public:
    Renderer(const AudioRendererParameters& params, std::span<u8> work_buffer_);

    result_t UpdateBehaviorInfoIn(io::MemoryStream* in_stream);
    result_t UpdateMemoryPools(io::MemoryStream* in_stream,
                               hw::tegra_x1::cpu::IMmu* mmu,
                               UpdateDataHeader& out_header,
                               io::MemoryStream* out_stream);
    result_t UpdateChannels(io::MemoryStream* in_stream,
                            UpdateDataHeader& out_header,
                            io::MemoryStream* out_stream);
    result_t UpdateVoices(io::MemoryStream* in_stream,
                          UpdateDataHeader& out_header,
                          io::MemoryStream* out_stream);
    result_t UpdateEffects(io::MemoryStream* in_stream,
                           UpdateDataHeader& out_header,
                           io::MemoryStream* out_stream);
    // TODO: splitter
    result_t UpdateMixes(io::MemoryStream* in_stream,
                         UpdateDataHeader& out_header,
                         io::MemoryStream* out_stream);
    result_t UpdateSinks(io::MemoryStream* in_stream,
                         UpdateDataHeader& out_header,
                         io::MemoryStream* out_stream);
    result_t UpdatePerformanceBuffer(io::MemoryStream* in_stream,
                                     UpdateDataHeader& out_header,
                                     io::MemoryStream* out_stream,
                                     io::MemoryStream* out_perf_stream);
    result_t UpdateBehaviorInfoOut(UpdateDataHeader& out_header,
                                   io::MemoryStream* out_stream);
    // TODO: renderer info

  private:
    std::span<u8> work_buffer;

    std::vector<MemPool> mempools;
    u32 voice_count;
    u32 effect_count;
    u32 sink_count;
};

} // namespace hydra::horizon::services::audio::internal
