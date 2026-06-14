#include "core/horizon/services/audio/internal/mempool.hpp"

#include "core/hw/tegra_x1/cpu/mmu.hpp"

namespace hydra::horizon::services::audio::internal {

bool MemPool::Update(hw::tegra_x1::cpu::IMmu* mmu, const MemPoolInfoIn& info_in,
                     MemPoolInfoOut& info_out) {
    auto state = info_in.state;
    if (state == MemPoolState::RequestAttach) {
        if (!Map(std::span(
                reinterpret_cast<u8*>(mmu->UnmapAddr(info_in.address)),
                info_in.size)))
            return false;
        state = MemPoolState::Attached;
    } else if (state == MemPoolState::RequestDetach) {
        if (!Unmap())
            return false;
        state = MemPoolState::Detached;
    }

    info_out.new_state = state;
    return true;
}

} // namespace hydra::horizon::services::audio::internal
