#pragma once

#include "core/hw/tegra_x1/gpu/renderer/shader_decompiler/iterator_base.hpp"

namespace hydra::hw::tegra_x1::gpu::renderer::shader_decomp {

class AllPathsIterator : public IteratorBase {
  public:
    using IteratorBase::IteratorBase;

    void Iterate(ObserverBase* observer) override;

  private:
    std::queue<u32> block_queue;
    std::vector<u32> visited_blocks;

    bool JumpToNextBlock(ObserverBase* observer, u32 label = invalid<u32>()) {
        if (label != invalid<u32>() &&
            std::find(visited_blocks.begin(), visited_blocks.end(), label) ==
                visited_blocks.end()) {
            Jump(observer, label);
            visited_blocks.push_back(label);
            return true;
        } else if (!block_queue.empty()) {
            Jump(observer, block_queue.front());
            block_queue.pop();
            return true;
        }

        return false;
    }

    void PushBlock(u32 label) {
        if (std::find(visited_blocks.begin(), visited_blocks.end(), label) ==
            visited_blocks.end()) {
            block_queue.push(label);
            visited_blocks.push_back(label);
        }
    }
};

} // namespace hydra::hw::tegra_x1::gpu::renderer::shader_decomp
