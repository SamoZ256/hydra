#include "core/hw/tegra_x1/gpu/renderer/shader_decompiler/lang/structurizer.hpp"

namespace hydra::hw::tegra_x1::gpu::renderer::shader_decomp::Lang {

namespace {

CfgNode* ResolveBlockImpl(const CfgBasicBlock* block) {
    switch (block->edge.type) {
    case CfgBlockEdgeType::Branch:
        return new CfgStructuredNodeWithEdge(
            new CfgCodeBlock(block->code_range),
            ResolveBlockImpl(block->edge.branch.target));
    case CfgBlockEdgeType::BranchConditional: {
        auto block_true = block->edge.branch_conditional.target_true;
        auto block_false = block->edge.branch_conditional.target_false;
        auto resolved_block_true = ResolveBlockImpl(block_true);
        auto resolved_block_false = ResolveBlockImpl(block_false);
        if (!resolved_block_true || !resolved_block_false) {
            LOG_ERROR(ShaderDecompiler, "Failed to resolve branch conditional");
            return nullptr;
        }

        CfgNode* node_true = resolved_block_true;
        CfgNode* node_false = resolved_block_false;
        CfgNode* merge_node = nullptr;
        if (auto block_true_with_edge =
                dynamic_cast<CfgStructuredNodeWithEdge*>(resolved_block_true)) {
            node_true = block_true_with_edge->node;
            merge_node = block_true_with_edge->branch_target;
        }

        if (auto block_false_with_edge =
                dynamic_cast<CfgStructuredNodeWithEdge*>(
                    resolved_block_false)) {
            node_false = block_false_with_edge->node;
            if (merge_node) {
                // Verify that the branch targets match
                if (!block_false_with_edge->branch_target->IsSameAs(
                        merge_node)) {
                    LOG_ERROR(ShaderDecompiler, "Branch targets do not match");
                    resolved_block_true->Log();
                    resolved_block_false->Log();
                    return nullptr;
                }
            } else {
                merge_node = block_false_with_edge->branch_target;
            }
        }

        auto code_block = new CfgCodeBlock(block->code_range);
        auto if_else_block = new CfgIfElseBlock(
            block->edge.branch_conditional.pred_cond, node_true, node_false);
        auto base_block = new CfgBlock({code_block, if_else_block});
        if (merge_node)
            return new CfgStructuredNodeWithEdge(base_block, merge_node);
        else
            return base_block;
    }
    case CfgBlockEdgeType::Exit:
        return new CfgCodeBlock(block->code_range);
    }

    return nullptr;
}

CfgBlock* ResolveBlock(const CfgBasicBlock* block) {
    auto resolved_block = ResolveBlockImpl(block);
    if (!resolved_block) {
        LOG_ERROR(ShaderDecompiler, "Failed to resolve block");
        return nullptr;
    }

    if (auto code_block = dynamic_cast<CfgCodeBlock*>(resolved_block)) {
        return new CfgBlock({code_block});
    } else if (auto block_with_edge =
                   dynamic_cast<CfgStructuredNodeWithEdge*>(resolved_block)) {
        return new CfgBlock(
            {block_with_edge->node, block_with_edge->branch_target});
    } else if (auto block = dynamic_cast<CfgBlock*>(resolved_block)) {
        return block;
    } else {
        LOG_ERROR(ShaderDecompiler, "Unknown resolved block type");
        return nullptr;
    }
}

} // namespace

CfgBlock* Structurize(const CfgBasicBlock* entry_bb) {
    return ResolveBlock(entry_bb);
}

} // namespace hydra::hw::tegra_x1::gpu::renderer::shader_decomp::Lang
