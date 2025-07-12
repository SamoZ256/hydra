#include "core/hw/tegra_x1/cpu/thread.hpp"

#include "core/hw/tegra_x1/cpu/mmu.hpp"

namespace hydra::hw::tegra_x1::cpu {

void IThread::GetStackTrace(stack_frame_callback_fn_t callback) {
    u64 fp = GetFP();
    u64 lr = GetLR();
    u64 sp = GetSP();

    callback(GetPC());
    callback(GetElr());

    for (uint64_t frame = 0; fp != 0; frame++) {
        callback(lr - 0x4);
        if (frame == MAX_STACK_TRACE_DEPTH - 1) {
            // TODO: add some magic address?
            break;
        }

        // HACK
        // if (fp < 0x10000000 || fp >= 0x20000000) {
        //    LOG_WARN(Mmu, "Currputed stack");
        //    break;
        //}

        u64 new_fp = mmu->Load<u64>(fp);
        lr = mmu->Load<u64>(fp + 8);

        fp = new_fp;
    }
}

} // namespace hydra::hw::tegra_x1::cpu
