#pragma once

#include "core/hw/tegra_x1/gpu/renderer/shader_decompiler/cfg.hpp"

namespace hydra::hw::tegra_x1::gpu::renderer::shader_decomp::Lang {

struct CfgNode {
    virtual ~CfgNode() = default;

    virtual bool IsSameAs(const CfgNode* other) const = 0;

    // Debug
    virtual void Log(const u32 indent = 0) const = 0;
};

struct CfgStructuredNodeWithEdge : public CfgNode {
    CfgNode* node;
    CfgNode* branch_target;

    CfgStructuredNodeWithEdge(CfgNode* node_, CfgNode* branch_target_)
        : node{node_}, branch_target{branch_target_} {}

    bool IsSameAs(const CfgNode* other) const override {
        if (auto other_with_edge =
                dynamic_cast<const CfgStructuredNodeWithEdge*>(other)) {
            return node->IsSameAs(other_with_edge->node) &&
                   branch_target->IsSameAs(other_with_edge->branch_target);
        }

        return false;
    }

    void Log(const u32 indent = 0) const override {
        LOG_DEBUG(ShaderDecompiler, INDENT_FMT "Structured node with edge",
                  PASS_INDENT(indent));
        LOG_DEBUG(ShaderDecompiler,
                  INDENT_FMT "Node:", PASS_INDENT(indent + 1));
        node->Log(indent + 2);
        LOG_DEBUG(ShaderDecompiler,
                  INDENT_FMT "Branch target:", PASS_INDENT(indent + 1));
        branch_target->Log(indent + 2);
    }
};

struct CfgCodeBlock : public CfgNode {
    range<u32> code_range;

    CfgCodeBlock(range<u32> code_range_) : code_range{code_range_} {}

    bool IsSameAs(const CfgNode* other) const override {
        if (auto other_code_block = dynamic_cast<const CfgCodeBlock*>(other)) {
            return code_range == other_code_block->code_range;
        }

        return false;
    }

    void Log(const u32 indent = 0) const override {
        LOG_DEBUG(ShaderDecompiler,
                  INDENT_FMT "Code block:", PASS_INDENT(indent));
        LOG_DEBUG(ShaderDecompiler, INDENT_FMT "Range: {}",
                  PASS_INDENT(indent + 1), code_range);
    }
};

struct CfgBlock : public CfgNode {
    std::vector<CfgNode*> nodes;

    CfgBlock(const std::vector<CfgNode*>& nodes_) {
        for (const auto node : nodes_) {
            if (auto block_node = dynamic_cast<CfgBlock*>(node))
                nodes.insert(nodes.end(), block_node->nodes.begin(),
                             block_node->nodes.end());
            else
                nodes.push_back(node);
        }
    }

    bool IsSameAs(const CfgNode* other) const override {
        if (auto other_block = dynamic_cast<const CfgBlock*>(other)) {
            if (nodes.size() != other_block->nodes.size())
                return false;

            for (u32 i = 0; i < nodes.size(); i++) {
                if (!nodes[i]->IsSameAs(other_block->nodes[i]))
                    return false;
            }

            return true;
        }

        return false;
    }

    void Log(const u32 indent = 0) const override {
        LOG_DEBUG(ShaderDecompiler, INDENT_FMT "Block:", PASS_INDENT(indent));
        LOG_DEBUG(ShaderDecompiler,
                  INDENT_FMT "Nodes:", PASS_INDENT(indent + 1));
        for (const auto& node : nodes)
            node->Log(indent + 2);
    }
};

struct CfgIfElseBlock : public CfgNode {
    PredCond pred_cond;
    CfgNode* then_block;
    CfgNode* else_block;

    bool IsSameAs(const CfgNode* other) const override {
        if (auto other_if_else_block =
                dynamic_cast<const CfgIfElseBlock*>(other)) {
            return pred_cond == other_if_else_block->pred_cond &&
                   then_block->IsSameAs(other_if_else_block->then_block) &&
                   else_block->IsSameAs(other_if_else_block->else_block);
        }

        return false;
    }

    CfgIfElseBlock(const PredCond pred_cond_, CfgNode* then_block_,
                   CfgNode* else_block_)
        : pred_cond{pred_cond_}, then_block{then_block_}, else_block{
                                                              else_block_} {}

    void Log(const u32 indent = 0) const override {
        LOG_DEBUG(ShaderDecompiler,
                  INDENT_FMT "If-else block:", PASS_INDENT(indent));
        LOG_DEBUG(ShaderDecompiler, INDENT_FMT "if {}p{}",
                  PASS_INDENT(indent + 1), pred_cond.not_ ? "!" : "",
                  pred_cond.pred);
        LOG_DEBUG(ShaderDecompiler,
                  INDENT_FMT "Then block:", PASS_INDENT(indent + 1));
        then_block->Log(indent + 2);
        LOG_DEBUG(ShaderDecompiler,
                  INDENT_FMT "Else block:", PASS_INDENT(indent + 1));
        else_block->Log(indent + 2);
    }
};

CfgBlock* Structurize(const CfgBasicBlock* entry_bb);

} // namespace hydra::hw::tegra_x1::gpu::renderer::shader_decomp::Lang
