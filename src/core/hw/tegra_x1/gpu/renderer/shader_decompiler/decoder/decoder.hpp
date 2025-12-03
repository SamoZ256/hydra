#pragma once

#include "core/hw/tegra_x1/gpu/renderer/shader_decompiler/ir/builder.hpp"

namespace hydra::hw::tegra_x1::gpu::renderer::shader_decomp::ir {
class Builder;
}

namespace hydra::hw::tegra_x1::gpu::renderer::shader_decomp::decoder {

enum class BlockStatus {
    Unvisited,
    Visited,
};

struct Block {
    BlockStatus status{BlockStatus::Unvisited};
    std::stack<label_t> sync_point_stack;
};

struct DecoderContext {
    const DecompilerContext& decomp_context;
    Reader code_reader;
    ir::Builder& builder;
};

class Decoder {
  public:
    Decoder(const DecoderContext& context_) : context{context_} {}

    void Decode();

  private:
    DecoderContext context;

    std::queue<label_t> to_visit_queue;
    std::map<label_t, Block> blocks;
    Block* crnt_block{nullptr};

    void ParseNextInstruction();

    // Helpers
    void Jump(u32 target) {
        context.code_reader.Seek(target * sizeof(instruction_t));
    }
    u32 GetPC() const {
        return static_cast<u32>(context.code_reader.Tell() / sizeof(instruction_t));
    }

    void EndBlock() {
        crnt_block->status = BlockStatus::Visited;
        crnt_block = nullptr;
        while (!to_visit_queue.empty()) {
            const auto label = to_visit_queue.front();
            to_visit_queue.pop();

            auto block = &blocks[label];
            if (block->status == BlockStatus::Unvisited) {
                crnt_block = block;
                context.builder.SetInsertBlock(label);
                Jump(label);
                break;
            }
        }
    }

    Block& EnsureBlock(label_t label) {
        auto& block = blocks[label];
        if (block.status == BlockStatus::Unvisited)
            to_visit_queue.push(label);

        return block;
    }

    void PushSyncPoint(label_t sync_point) {
        crnt_block->sync_point_stack.push(sync_point);
    }

    void InheritSyncPoints(label_t label) {
        auto& block = EnsureBlock(label);
        // TODO: if the block already has sync points, make sure they match
        block.sync_point_stack = crnt_block->sync_point_stack;
    }
};

} // namespace hydra::hw::tegra_x1::gpu::renderer::shader_decomp::decoder
