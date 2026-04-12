#include "core/hw/tegra_x1/gpu/renderer/shader_decompiler/analyzer/cfg.hpp"

#include "core/hw/tegra_x1/gpu/renderer/shader_decompiler/ir/function.hpp"

namespace hydra::hw::tegra_x1::gpu::renderer::shader_decomp::analyzer {

CfgBasicBlock* CfgBuilder::Build(const ir::Function& function) {
    for (const auto& [label, block] : function.GetBlocks()) {
        auto cfg_block = GetBlock(label);
        const auto& last_inst = block.GetInstructions().back();
        switch (last_inst.GetOpcode()) {
        case ir::Opcode::Branch:
            cfg_block->edge = {
                .type = CfgBlockEdgeType::Branch,
                .branch = {GetBlock(last_inst.GetOperand(0).GetLabel())}};
            break;
        case ir::Opcode::BranchConditional:
            cfg_block->edge = {
                .type = CfgBlockEdgeType::BranchConditional,
                .branch_conditional = {
                    .cond = last_inst.GetOperand(0),
                    .target_true = GetBlock(last_inst.GetOperand(1).GetLabel()),
                    .target_false =
                        GetBlock(last_inst.GetOperand(2).GetLabel()),
                }};
            break;
        case ir::Opcode::Exit:
            cfg_block->edge = {
                .type = CfgBlockEdgeType::Exit,
            };
            break;
        default:
            LOG_ERROR(ShaderDecompiler, "Invalid last instruction opcode {}",
                      last_inst.GetOpcode());
            break;
        }
    }

    return blocks.at(0x0);
}

} // namespace hydra::hw::tegra_x1::gpu::renderer::shader_decomp::analyzer
