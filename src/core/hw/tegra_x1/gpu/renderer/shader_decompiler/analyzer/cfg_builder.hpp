#pragma once

#include "core/hw/tegra_x1/gpu/renderer/shader_decompiler/cfg.hpp"
#include "core/hw/tegra_x1/gpu/renderer/shader_decompiler/observer_base.hpp"

namespace hydra::hw::tegra_x1::gpu::renderer::shader_decomp::Analyzer {

class CfgBuilder : public ObserverBase {
  public:
    CfgBuilder();

    void SetPredCond(const PredCond pred_cond_) override {
        pred_cond = pred_cond_;
    }
    void ClearPredCond() override { pred_cond = nullptr; }
    void BlockChanged() override;

    // Operations
    void OpSetSync(u32 target) override;
    void OpSync() override;
    void OpBranch(u32 target) override;
    void OpExit() override;

    // Debug
    void LogBlocks() const {
        for (const auto& [label, block] : blocks)
            block.Log();
    }

    // Getters
    const CfgBasicBlock* GetEntryBlock() const { return entry_point_block; }

  private:
    std::map<u32, CfgBasicBlock> blocks;
    CfgBasicBlock* entry_point_block;

    CfgBasicBlock* crnt_block;

    nullable<PredCond> pred_cond;

    CfgBasicBlock& VisitBlock(u32 label) {
        auto& block = blocks[label];
        ASSERT_DEBUG(block.status == CfgBlockStatus::Unvisited,
                     ShaderDecompiler, "Block 0x{:x} already visited", label);
        block.status = CfgBlockStatus::Visited;
        block.code_range = range(label);

        return block;
    }

    void FinishBlock(CfgBasicBlock& block, const u32 end,
                     const CfgBlockEdge& edge) {
        ASSERT_DEBUG(block.status == CfgBlockStatus::Visited, ShaderDecompiler,
                     "Block 0x{:x} finished without being visited",
                     block.code_range.begin);

        block.code_range.end = end;
        block.edge = edge;
        block.status = CfgBlockStatus::Finished;
    }

    void EndBlock(const CfgBlockEdge& edge) {
        FinishBlock(*crnt_block,
                    pc + (edge.type == CfgBlockEdgeType::Exit ? 1 : 0), edge);
        crnt_block = nullptr;
    }

    CfgBasicBlock& GetBranchTarget(u32 label, u32 return_sync_point) {
        auto& block = blocks[label];
        block.return_sync_point = return_sync_point;
        return block;
    }
};

} // namespace hydra::hw::tegra_x1::gpu::renderer::shader_decomp::Analyzer
