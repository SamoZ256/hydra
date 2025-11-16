#include "core/hw/tegra_x1/gpu/renderer/shader_decompiler/analyzer/memory_analyzer.hpp"

namespace hydra::hw::tegra_x1::gpu::renderer::shader_decomp::analyzer {

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

void MemoryAnalyzer::Analyze(const ir::Module& modul) {
    for (const auto& [name, function] : modul.GetFunctions()) {
        for (const auto& [label, block] : function.GetBlocks()) {
            for (const auto& instruction : block.GetInstructions()) {
                // Dst
                if (instruction.HasDst()) {
                    const auto& dst = instruction.GetDst();
                    switch (dst.GetType()) {
                    case ir::ValueType::AttrMemory:
                        HandleAMemStore(dst.GetAttrMemory());
                        break;
                    default:
                        break;
                    }
                }

                // Operands
                for (const auto& operand : instruction.GetOperands()) {
                    switch (operand.GetType()) {
                    case ir::ValueType::AttrMemory:
                        HandleAMemLoad(operand.GetAttrMemory());
                        break;
                    case ir::ValueType::ConstMemory:
                        HandleCMemLoad(operand.GetConstMemory());
                        break;
                    default:
                        break;
                    }
                }

                // Texture
                switch (instruction.GetOpcode()) {
                case ir::Opcode::TextureSample:
                case ir::Opcode::TextureRead:
                case ir::Opcode::TextureGather:
                case ir::Opcode::TextureQueryDimension: {
                    const auto const_buffer_index =
                        instruction.GetOperand(0).GetRawValue<u32>();
                    push_unique(textures, const_buffer_index);
                    break;
                }
                default:
                    break;
                }
            }
        }
    }
}

void MemoryAnalyzer::HandleAMemLoad(const AMem amem) {
    // TODO: support indexing with src
    ASSERT_DEBUG(amem.reg == RZ, ShaderDecompiler,
                 "Indexing not implemented (src: {})", amem.reg);
    push_sv(input_svs, stage_inputs, amem.imm);
}

void MemoryAnalyzer::HandleCMemLoad(const CMem cmem) {
    if (cmem.reg != RZ)
        LOG_WARN(ShaderDecompiler, "Indexing not implemented (src: {})",
                 cmem.reg);

    auto& size = uniform_buffers[cmem.idx];
    size = std::max(size, static_cast<usize>(cmem.imm) + sizeof(u32));
}

void MemoryAnalyzer::HandleAMemStore(const AMem amem) {
    // TODO: support indexing with src
    ASSERT_DEBUG(amem.reg == RZ, ShaderDecompiler,
                 "Indexing not implemented (src: {})", amem.reg);
    push_sv(output_svs, stage_outputs, amem.imm);
}

} // namespace hydra::hw::tegra_x1::gpu::renderer::shader_decomp::analyzer
