#include "core/hw/tegra_x1/gpu/renderer/shader_decompiler/analyzer/cfg.hpp"

#include "core/hw/tegra_x1/gpu/renderer/shader_decompiler/ir/function.hpp"

namespace hydra::hw::tegra_x1::gpu::renderer::shader_decomp::analyzer {

CfgBasicBlock* CfgBuilder::build(const ir::Function& function) {
    for (const auto& [label, block] : function.getBlocks()) {
        auto cfg_block = getBlock(label);
        const auto& last_inst = block.getInstructions().back();
        switch (last_inst.getOpcode()) {
        case ir::Opcode::Branch:
            cfg_block->edge = {
                .type = CfgBlockEdgeType::Branch,
                .branch.target = getBlock(last_inst.getOperand(0).getLabel()),
            };
            break;
        case ir::Opcode::BranchConditional:
            cfg_block->edge = {
                .type = CfgBlockEdgeType::BranchConditional,
                .branch_conditional = {
                    .cond = last_inst.getOperand(0),
                    .target_true = getBlock(last_inst.getOperand(1).getLabel()),
                    .target_false =
                        getBlock(last_inst.getOperand(2).getLabel()),
                }};
            break;
        case ir::Opcode::Exit:
            cfg_block->edge = {
                .type = CfgBlockEdgeType::Exit,
            };
            break;
        default:
            LOG_ERROR(ShaderDecompiler, "Invalid last instruction opcode {}",
                      last_inst.getOpcode());
            break;
        }
    }

    return blocks.at(0x0);
}

} // namespace hydra::hw::tegra_x1::gpu::renderer::shader_decomp::analyzer
