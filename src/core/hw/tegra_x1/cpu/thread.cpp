#include "core/hw/tegra_x1/cpu/thread.hpp"

#include "core/hw/tegra_x1/cpu/mmu.hpp"

namespace hydra::hw::tegra_x1::cpu {

void IThread::GetStackTrace(const stack_frame_callback_fn_t& callback) {
    u64 fp = state.fp;
    u64 lr = state.lr;

    callback(state.pc);

    for (uint64_t frame = 0; fp != 0; frame++) {
        callback(lr - 0x4);
        if (frame == MAX_STACK_TRACE_DEPTH - 1) {
            // TODO: add some magic address?
            break;
        }

        const auto fp_ptr = mmu->UnmapAddr(fp);
        if (fp_ptr == 0x0)
            break;

        u64 new_fp = *reinterpret_cast<u64*>(fp_ptr);
        lr = *reinterpret_cast<u64*>(fp_ptr + 8);

        fp = new_fp;
    }
}

} // namespace hydra::hw::tegra_x1::cpu
