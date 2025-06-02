#include "core/hw/tegra_x1/gpu/renderer/shader_decompiler/all_paths_iterator.hpp"

namespace hydra::hw::tegra_x1::gpu::renderer::shader_decomp {

void AllPathsIterator::Iterate(ObserverBase* observer) {
    while (true) {
        const auto res = ParseNextInstruction(observer);
        switch (res.code) {
        case ResultCode::None:
            break;
        case ResultCode::Branch:
            if (!JumpToNextBlock(observer, res.target))
                return;
            break;
        case ResultCode::BranchConditional:
            if (!JumpToNextBlock(observer, GetPC() + 1))
                return;
            PushBlock(res.target);
            break;
        case ResultCode::SyncPoint:
            PushBlock(res.target);
            break;
        case ResultCode::EndBlock: {
            if (!JumpToNextBlock(observer))
                return;
            break;
        }
        case ResultCode::Error:
            LOG_ERROR(ShaderDecompiler, "Error");
            return;
        }
    }
}

} // namespace hydra::hw::tegra_x1::gpu::renderer::shader_decomp
