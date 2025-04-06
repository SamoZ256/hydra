#include "hw/tegra_x1/gpu/renderer/shader_decompiler/analyzer.hpp"

namespace Hydra::HW::TegraX1::GPU::Renderer::ShaderDecompiler {

namespace {

void push_sv(std::vector<SVSemantic>& svs, std::vector<u8>& stage_in_outs,
             u64 addr) {
    const auto sv = get_sv_from_addr(addr);
    if (sv.semantic == SVSemantic::UserInOut)
        push_unique(stage_in_outs, sv.index);
    else
        push_unique(svs, sv.semantic);
}

} // namespace

void Analyzer::OpFloatAdd(reg_t dst, reg_t src1, Operand src2) {
    if (src2.type == OperandType::ConstMemory)
        HandleCMemLoad(src2.cmem);
}

void Analyzer::OpFloatMultiply(reg_t dst, reg_t src1, Operand src2) {
    if (src2.type == OperandType::ConstMemory)
        HandleCMemLoad(src2.cmem);
}

void Analyzer::OpFloatFma(reg_t dst, reg_t src1, Operand src2, reg_t src3) {
    if (src2.type == OperandType::ConstMemory)
        HandleCMemLoad(src2.cmem);
}

void Analyzer::OpLoad(reg_t dst, Operand src) {
    switch (src.type) {
    case OperandType::AttributeMemory:
        HandleAMemLoad(src.amem);
        break;
    case OperandType::ConstMemory:
        HandleCMemLoad(src.cmem);
        break;
    default:
        LOG_ERROR(ShaderDecompiler, "Invalid OpLoad src type {}", src.type);
        break;
    }
}

void Analyzer::OpStore(AMem dst, reg_t src) { HandleAMemStore(dst); }

void Analyzer::OpInterpolate(reg_t dst, AMem src) { HandleAMemLoad(src); }

void Analyzer::OpTextureSample(reg_t dst, u32 index, reg_t coords) {
    push_unique(textures, index);
}

void Analyzer::HandleAMemLoad(const AMem amem) {
    // TODO: support indexing with src
    ASSERT_DEBUG(amem.reg == RZ, ShaderDecompiler,
                 "Indexing not implemented (src: r{})", amem.reg);
    push_sv(input_svs, stage_inputs, amem.imm);
}

void Analyzer::HandleAMemStore(const AMem amem) {
    // TODO: support indexing with src
    ASSERT_DEBUG(amem.reg == RZ, ShaderDecompiler,
                 "Indexing not implemented (src: r{})", amem.reg);
    push_sv(output_svs, stage_outputs, amem.imm);
}

void Analyzer::HandleCMemLoad(const CMem cmem) {
    if (cmem.reg != RZ) {
        LOG_WARNING(ShaderDecompiler, "Indexing not implemented (src: r{})",
                    cmem.reg);
        return;
    }

    auto& size = uniform_buffers[cmem.idx];
    size = std::max(size, static_cast<usize>(cmem.imm) + sizeof(u32));
}

} // namespace Hydra::HW::TegraX1::GPU::Renderer::ShaderDecompiler
