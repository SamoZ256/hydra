#pragma once

#include "core/hw/tegra_x1/gpu/renderer/shader_decompiler/ir/value.hpp"

namespace hydra::hw::tegra_x1::gpu::renderer::shader_decomp::ir {
class Function;
}

namespace hydra::hw::tegra_x1::gpu::renderer::shader_decomp::analyzer {

enum class CfgBlockEdgeType {
    None,
    Branch,
    BranchConditional,
    Exit,
    Break,
    Continue,

    Invalid,
};

struct CfgBasicBlock;

struct CfgBlockEdge {
    CfgBlockEdgeType type;
    union {
        struct {
            CfgBasicBlock* target;
        } branch;
        struct {
            ir::Value cond;
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
            return branch_conditional.cond == other.branch_conditional.cond &&
                   branch_conditional.target_true ==
                       other.branch_conditional.target_true &&
                   branch_conditional.target_false ==
                       other.branch_conditional.target_false;
        default:
            return true;
        }
    }
};

} // namespace hydra::hw::tegra_x1::gpu::renderer::shader_decomp::analyzer

ENABLE_ENUM_FORMATTING(hydra::hw::tegra_x1::gpu::renderer::shader_decomp::
                           analyzer::CfgBlockEdgeType,
                       Invalid, "invalid", Branch, "branch", BranchConditional,
                       "branch conditional", Exit, "exit")

namespace hydra::hw::tegra_x1::gpu::renderer::shader_decomp::analyzer {

struct CfgBasicBlock {
    label_t label;
    CfgBlockEdge edge;

    CfgBasicBlock* Clone() const {
        auto clone = new CfgBasicBlock(*this);
        clone->Walk([](CfgBasicBlock* b) {
            switch (b->edge.type) {
            case CfgBlockEdgeType::Branch:
                b->edge.branch.target =
                    new CfgBasicBlock(*b->edge.branch.target);
                break;
            case CfgBlockEdgeType::BranchConditional:
                b->edge.branch_conditional.target_true =
                    new CfgBasicBlock(*b->edge.branch_conditional.target_true);
                b->edge.branch_conditional.target_false =
                    new CfgBasicBlock(*b->edge.branch_conditional.target_false);
                break;
            default:
                break;
            }

            return true;
        });

        return clone;
    }

    bool IsSameAs(const CfgBasicBlock* other) const {
        if (other == this)
            return true;

        if (other->label != label)
            return false;

        if (other->edge.type != edge.type)
            return false;

        switch (edge.type) {
        case CfgBlockEdgeType::Branch:
            return edge.branch.target->IsSameAs(other->edge.branch.target);
        case CfgBlockEdgeType::BranchConditional:
            return edge.branch_conditional.target_true->IsSameAs(
                       other->edge.branch_conditional.target_true) &&
                   edge.branch_conditional.target_false->IsSameAs(
                       other->edge.branch_conditional.target_false);
        default:
            return true;
        }
    }

    void Walk(std::function<bool(CfgBasicBlock*)> visitor) {
        std::vector<CfgBasicBlock*> visited;
        WalkImpl(visitor, visited);
    }

    bool CanJumpTo(const CfgBasicBlock* target) {
        bool can_jump = false;
        Walk([&can_jump, target](CfgBasicBlock* b) {
            if (b->IsSameAs(target)) {
                can_jump = true;
                return false;
            }

            return true;
        });

        return can_jump;
    }

    bool CanDirectlyJumpTo(const CfgBasicBlock* target) const {
        switch (edge.type) {
        case CfgBlockEdgeType::Branch:
            return edge.branch.target->IsSameAs(target);
        case CfgBlockEdgeType::BranchConditional:
            return edge.branch_conditional.target_true->IsSameAs(target) ||
                   edge.branch_conditional.target_false->IsSameAs(target);
        default:
            return false;
        }
    }

    CfgBasicBlock* FindMergeBlock(CfgBasicBlock* other) {
        CfgBasicBlock* merge_block = nullptr;
        Walk([&](CfgBasicBlock* b) {
            if (other->CanJumpTo(b)) {
                merge_block = b;
                return false;
            }

            return true;
        });

        return merge_block;
    }

    // Debug
    void Log(const u32 indent = 0) const {
        LOG_DEBUG(ShaderDecompiler, INDENT_FMT "Block: {}", PASS_INDENT(indent),
                  label);
        LOG_DEBUG(ShaderDecompiler, INDENT_FMT "Edge: {}",
                  PASS_INDENT(indent + 1), edge.type);
        switch (edge.type) {
        case CfgBlockEdgeType::Branch:
            LOG_DEBUG(ShaderDecompiler, INDENT_FMT "Target: {}",
                      PASS_INDENT(indent + 2), edge.branch.target->label);
            break;
        case CfgBlockEdgeType::BranchConditional:
            LOG_DEBUG(ShaderDecompiler, INDENT_FMT "if {}",
                      PASS_INDENT(indent + 2), edge.branch_conditional.cond);
            LOG_DEBUG(ShaderDecompiler, INDENT_FMT "Target True: {}",
                      PASS_INDENT(indent + 2),
                      edge.branch_conditional.target_true->label);
            LOG_DEBUG(ShaderDecompiler, INDENT_FMT "Target False: {}",
                      PASS_INDENT(indent + 2),
                      edge.branch_conditional.target_false->label);
            break;
        default:
            break;
        }
    }

  private:
    void WalkImpl(std::function<bool(CfgBasicBlock*)> visitor,
                  std::vector<CfgBasicBlock*>& visited) {
        if (std::find(visited.begin(), visited.end(), this) != visited.end())
            return;

        visited.push_back(this);

        if (!visitor(this))
            return;

        switch (edge.type) {
        case CfgBlockEdgeType::Branch:
            edge.branch.target->WalkImpl(visitor, visited);
            break;
        case CfgBlockEdgeType::BranchConditional:
            edge.branch_conditional.target_true->WalkImpl(visitor, visited);
            edge.branch_conditional.target_false->WalkImpl(visitor, visited);
            break;
        default:
            break;
        }
    }
};

class CfgBuilder {
  public:
    CfgBasicBlock* Build(const ir::Function& function);

  private:
    std::map<label_t, CfgBasicBlock*> blocks;

    // Helpers
    CfgBasicBlock* GetBlock(label_t label) {
        auto& block = blocks[label];
        if (!block)
            block = new CfgBasicBlock{.label = label};
        return block;
    }
};

} // namespace hydra::hw::tegra_x1::gpu::renderer::shader_decomp::analyzer
