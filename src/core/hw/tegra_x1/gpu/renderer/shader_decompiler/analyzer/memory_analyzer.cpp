#include "core/hw/tegra_x1/gpu/renderer/shader_decompiler/analyzer/memory_analyzer.hpp"

namespace hydra::hw::tegra_x1::gpu::renderer::shader_decomp::analyzer {

namespace {

void push_sv(std::unordered_set<SvSemantic>& svs,
             std::unordered_set<u8>& stage_in_outs, u64 addr) {
    const auto sv = get_sv_access_from_addr(addr).sv;
    if (sv.semantic == SvSemantic::UserInOut)
        stage_in_outs.insert(sv.index);
    else
        svs.insert(sv.semantic);
}

} // namespace

void MemoryAnalyzer::Analyze(const ir::Module& modul) {
    for (const auto& [name, function] : modul.GetFunctions()) {
        for (const auto& [label, block] : function.GetBlocks()) {
            for (const auto& instruction : block.GetInstructions()) {
                // Dst
                if (instruction.HasDst()) {
                    const auto& dst = instruction.GetDst();
                    switch (dst.GetKind()) {
                    case ir::ValueKind::AttrMemory:
                        HandleAMemStore(dst.GetAttrMemory());
                        break;
                    default:
                        break;
                    }
                }

                // Operands
                for (const auto& operand : instruction.GetOperands()) {
                    switch (operand.GetKind()) {
                    case ir::ValueKind::AttrMemory:
                        HandleAMemLoad(operand.GetAttrMemory());
                        break;
                    case ir::ValueKind::ConstMemory:
                        HandleCMemLoad(operand.GetConstMemory());
                        break;
                    default:
                        break;
                    }
                }

                // Texture
                switch (instruction.GetOpcode()) {
                case ir::Opcode::TextureSample: {
                    const auto const_buffer_index =
                        instruction.GetOperand(0).GetRawValue<u32>();
                    const auto type =
                        instruction.GetOperand(1).GetRawValue<TextureType>();
                    const auto flags = instruction.GetOperand(2)
                                           .GetRawValue<TextureSampleFlags>();
                    bool is_depth =
                        any(flags & TextureSampleFlags::DepthCompare);
                    HandleTextureAccess(const_buffer_index,
                                        TextureInfo{type, is_depth});
                    break;
                }
                case ir::Opcode::TextureGather: {
                    const auto const_buffer_index =
                        instruction.GetOperand(0).GetRawValue<u32>();
                    // TODO: is_depth
                    HandleTextureAccess(const_buffer_index,
                                        TextureInfo{TextureType::_2D, false});
                    break;
                }
                // TODO: TextureQueryDimension?
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
    const_buffers.insert(cmem.idx);
}

void MemoryAnalyzer::HandleAMemStore(const AMem amem) {
    // TODO: support indexing with src
    ASSERT_DEBUG(amem.reg == RZ, ShaderDecompiler,
                 "Indexing not implemented (src: {})", amem.reg);
    push_sv(output_svs, stage_outputs, amem.imm);
}

void MemoryAnalyzer::HandleTextureAccess(u32 const_buffer_index,
                                         const TextureInfo& info) {
    const auto res = textures.emplace(const_buffer_index, info);
    if (!res.second) {
        if (res.first->second.type != info.type ||
            res.first->second.is_depth != info.is_depth) {
            // TODO: handle this
            LOG_WARN(ShaderDecompiler, "Texture type mismatch");
        }
    }
}

} // namespace hydra::hw::tegra_x1::gpu::renderer::shader_decomp::analyzer
