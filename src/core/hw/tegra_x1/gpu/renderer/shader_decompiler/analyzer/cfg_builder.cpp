#include "core/hw/tegra_x1/gpu/renderer/shader_decompiler/analyzer/cfg_builder.hpp"

namespace hydra::hw::tegra_x1::gpu::renderer::shader_decomp::Analyzer {

CfgBuilder::CfgBuilder() {
    entry_point_block = &VisitBlock(0);
    crnt_block = entry_point_block;
}

void CfgBuilder::BlockChanged() {
    ASSERT_DEBUG(!crnt_block, ShaderDecompiler,
                 "Starting a new block without finishing the previous one");
    crnt_block = &VisitBlock(pc);
}

void CfgBuilder::OpSetSync(u32 target) {
    ASSERT_DEBUG(crnt_block, ShaderDecompiler,
                 "Cannot set sync point without a block");
    crnt_block->return_sync_point = target;
}

void CfgBuilder::OpSync() {
    const auto target = crnt_block->return_sync_point;
    ASSERT_DEBUG(target != invalid<u32>(), ShaderDecompiler,
                 "Invalid sync point");

    CfgBlockEdge edge;
    edge.type = CfgBlockEdgeType::Branch;
    edge.branch.target = &blocks[target];
    EndBlock(edge);
}

void CfgBuilder::OpBranch(u32 target) {
    CfgBlockEdge edge;
    if (pred_cond) {
        edge.type = CfgBlockEdgeType::BranchConditional;
        edge.branch_conditional.pred_cond = pred_cond;
        edge.branch_conditional.target_true =
            &GetBranchTarget(target, crnt_block->return_sync_point);
        edge.branch_conditional.target_false =
            &GetBranchTarget(pc + 1, crnt_block->return_sync_point);
    } else {
        edge.type = CfgBlockEdgeType::Branch;
        edge.branch.target =
            &GetBranchTarget(target, crnt_block->return_sync_point);
    }
    EndBlock(edge);
}

void CfgBuilder::OpExit() {
    CfgBlockEdge edge;
    edge.type = CfgBlockEdgeType::Exit;
    EndBlock(edge);
}

} // namespace hydra::hw::tegra_x1::gpu::renderer::shader_decomp::Analyzer
