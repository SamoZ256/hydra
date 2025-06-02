#include "core/horizon/kernel/process.hpp"

#include "core/horizon/kernel/kernel.hpp"
#include "core/horizon/kernel/thread.hpp"
#include "core/hw/tegra_x1/cpu/mmu_base.hpp"

namespace hydra::horizon::kernel {

constexpr usize MAIN_THREAD_STACK_SIZE =
    0x4000000; // TODO: why does the size need to be double the size it used to
               // be before the thread rework? InstructionAbortSameEl otherwise

Process::Process()
    : main_thread(new Thread(STACK_REGION_BASE + MAIN_THREAD_STACK_SIZE - 0x10,
                             20)) /* TODO: priority */ {
    // Stack memory
    auto& mmu = hw::tegra_x1::cpu::MMUBase::GetInstance();
    stack_mem = mmu.AllocateMemory(MAIN_THREAD_STACK_SIZE);
    mmu.Map(STACK_REGION_BASE, stack_mem,
            {MemoryType::Stack, MemoryAttribute::None,
             MemoryPermission::ReadWrite});
}

Process::~Process() {
    hw::tegra_x1::cpu::MMUBase::GetInstance().Unmap(STACK_REGION_BASE,
                                                    stack_mem->GetSize());
    hw::tegra_x1::cpu::MMUBase::GetInstance().FreeMemory(stack_mem);
}

void Process::Run() { main_thread.handle->Run(); }

} // namespace hydra::horizon::kernel
