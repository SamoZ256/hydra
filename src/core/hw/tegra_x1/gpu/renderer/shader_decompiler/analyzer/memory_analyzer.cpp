#include "core/hw/tegra_x1/gpu/renderer/shader_decompiler/analyzer/memory_analyzer.hpp"

namespace hydra::hw::tegra_x1::gpu::renderer::shader_decomp::Analyzer {

namespace {

void push_sv(std::vector<SvSemantic>& svs, std::vector<u8>& stage_in_outs,
             u64 addr) {
    const auto sv = get_sv_access_from_addr(addr).sv;
    if (sv.semantic == SvSemantic::UserInOut)
        push_unique(stage_in_outs, sv.index);
    else
        push_unique(svs, sv.semantic);
}

} // namespace

ValueBase* MemoryAnalyzer::OpAttributeMemory(bool load, const AMem& amem,
                                             DataType data_type, bool neg) {
    if (load)
        HandleAMemLoad(amem);
    else
        HandleAMemStore(amem);
    return nullptr;
}

ValueBase* MemoryAnalyzer::OpConstMemoryL(const CMem& cmem, DataType data_type,
                                          bool neg) {
    HandleCMemLoad(cmem);
    return nullptr;
}

void MemoryAnalyzer::OpTextureSample(ValueBase* dstA, ValueBase* dstB,
                                     ValueBase* dstC, ValueBase* dstD,
                                     u32 const_buffer_index,
                                     ValueBase* coords_x, ValueBase* coords_y) {
    push_unique(textures, const_buffer_index);
}

void MemoryAnalyzer::HandleAMemLoad(const AMem amem) {
    // TODO: support indexing with src
    ASSERT_DEBUG(amem.reg == RZ, ShaderDecompiler,
                 "Indexing not implemented (src: r{})", amem.reg);
    push_sv(input_svs, stage_inputs, amem.imm);
}

void MemoryAnalyzer::HandleCMemLoad(const CMem cmem) {
    if (cmem.reg != RZ) {
        LOG_WARN(ShaderDecompiler, "Indexing not implemented (src: r{})",
                 cmem.reg);
        return;
    }

    auto& size = uniform_buffers[cmem.idx];
    size = std::max(size, static_cast<usize>(cmem.imm) + sizeof(u32));
}

void MemoryAnalyzer::HandleAMemStore(const AMem amem) {
    // TODO: support indexing with src
    ASSERT_DEBUG(amem.reg == RZ, ShaderDecompiler,
                 "Indexing not implemented (src: r{})", amem.reg);
    push_sv(output_svs, stage_outputs, amem.imm);
}

} // namespace hydra::hw::tegra_x1::gpu::renderer::shader_decomp::Analyzer
