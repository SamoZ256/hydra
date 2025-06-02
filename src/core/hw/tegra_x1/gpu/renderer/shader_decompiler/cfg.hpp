#pragma once

#include "core/hw/tegra_x1/gpu/renderer/shader_decompiler/const.hpp"

namespace hydra::hw::tegra_x1::gpu::renderer::shader_decomp {

enum class CfgBlockEdgeType {
    Branch,
    BranchConditional,
    Exit,
};

struct CfgBasicBlock;

struct CfgBlockEdge {
    CfgBlockEdgeType type;
    union {
        struct {
            CfgBasicBlock* target;
        } branch;
        struct {
            PredCond pred_cond;
            CfgBasicBlock* target_true;
            CfgBasicBlock* target_false;
        } branch_conditional;
    };

    bool operator==(const CfgBlockEdge& other) const {
        if (type != other.type)
            return false;

        switch (type) {
        case CfgBlockEdgeType::Branch:
            return branch.target == other.branch.target;
        case CfgBlockEdgeType::BranchConditional:
            return branch_conditional.pred_cond ==
                       other.branch_conditional.pred_cond &&
                   branch_conditional.target_true ==
                       other.branch_conditional.target_true &&
                   branch_conditional.target_false ==
                       other.branch_conditional.target_false;
        case CfgBlockEdgeType::Exit:
            return true;
        }
    }
};

} // namespace hydra::hw::tegra_x1::gpu::renderer::shader_decomp

ENABLE_ENUM_FORMATTING(
    hydra::hw::tegra_x1::gpu::renderer::shader_decomp::CfgBlockEdgeType, Branch,
    "branch", BranchConditional, "branch conditional", Exit, "exit")

namespace hydra::hw::tegra_x1::gpu::renderer::shader_decomp {

enum class CfgBlockStatus {
    Unvisited,
    Visited,
    Finished,
};

struct CfgBasicBlock {
    CfgBlockStatus status{CfgBlockStatus::Unvisited};
    range<u32> code_range;
    u32 return_sync_point{invalid<u32>()};
    CfgBlockEdge edge;

    void Log(const u32 indent = 0) const {
        LOG_DEBUG(ShaderDecompiler, INDENT_FMT "Block: 0x{:x}",
                  PASS_INDENT(indent), code_range.begin);
        if (status == CfgBlockStatus::Unvisited) {
            LOG_DEBUG(ShaderDecompiler, INDENT_FMT "UNVISITED",
                      PASS_INDENT(indent + 1));
            return;
        } else if (status == CfgBlockStatus::Visited) {
            LOG_DEBUG(ShaderDecompiler, INDENT_FMT "VISITED",
                      PASS_INDENT(indent + 1));
            return;
        }

        LOG_DEBUG(ShaderDecompiler, INDENT_FMT "Code: {}",
                  PASS_INDENT(indent + 1), code_range);
        if (return_sync_point != invalid<u32>())
            LOG_DEBUG(ShaderDecompiler, INDENT_FMT "Return Sync Point: {}",
                      PASS_INDENT(indent + 1), return_sync_point);

        LOG_DEBUG(ShaderDecompiler, INDENT_FMT "Edge: {}",
                  PASS_INDENT(indent + 1), edge.type);
        switch (edge.type) {
        case CfgBlockEdgeType::Branch:
            LOG_DEBUG(ShaderDecompiler, INDENT_FMT "Target: 0x{:x}",
                      PASS_INDENT(indent + 2),
                      edge.branch.target->code_range.begin);
            break;
        case CfgBlockEdgeType::BranchConditional:
            LOG_DEBUG(ShaderDecompiler, INDENT_FMT "if {}p{}",
                      PASS_INDENT(indent + 2),
                      edge.branch_conditional.pred_cond.not_ ? "!" : "",
                      edge.branch_conditional.pred_cond.pred);
            LOG_DEBUG(ShaderDecompiler, INDENT_FMT "Target True: 0x{:x}",
                      PASS_INDENT(indent + 2),
                      edge.branch_conditional.target_true->code_range.begin);
            LOG_DEBUG(ShaderDecompiler, INDENT_FMT "Target False: 0x{:x}",
                      PASS_INDENT(indent + 2),
                      edge.branch_conditional.target_false->code_range.begin);
            break;
        case CfgBlockEdgeType::Exit:
            break;
        }
    }
};

} // namespace hydra::hw::tegra_x1::gpu::renderer::shader_decomp
