#include "core/horizon/kernel/process.hpp"

#include <random>

#include "core/debugger/debugger_manager.hpp"
#include "core/hw/tegra_x1/cpu/cpu.hpp"
#include "core/hw/tegra_x1/cpu/mmu.hpp"
#include "core/hw/tegra_x1/gpu/gmmu.hpp"

namespace hydra::horizon::kernel {

Process::Process(const std::string_view debug_name)
    : SynchronizationObject(false, debug_name), mmu{CPU_INSTANCE.CreateMmu()},
      gmmu{new hw::tegra_x1::gpu::GMmu(mmu)} {
    // TODO: use title ID and name as debugger name?
    DEBUGGER_MANAGER_INSTANCE.AttachDebugger(
        this,
        /*fmt::format("{:016x}", title_id)*/ GetDebugName());

    // Random entropy
    std::random_device rd;
    std::mt19937_64 gen(rd());
    for (auto& random_e : random_entropy)
        random_e = gen();
}

Process::~Process() {
    CleanUp();
    DEBUGGER_MANAGER_INSTANCE.DetachDebugger(this);
}

uptr Process::CreateMemory(Range<vaddr_t> region, usize size, MemoryType type,
                           MemoryPermission perm, vaddr_t& out_base) {
    out_base = mmu->FindFreeMemory(region, size);
    ASSERT(out_base != 0x0, Kernel, "Failed to find free memory");

    auto mem = CPU_INSTANCE.AllocateMemory(size);
    mmu->Map(out_base, mem, {type, MemoryAttribute::None, perm});
    executable_mems.push_back(mem);

    return mem->GetPtr();
}

uptr Process::CreateExecutableMemory(const std::string_view module_name,
                                     CodeSet code_set, vaddr_t& out_base) {
    // TODO: use MemoryType::Static?
    auto ptr = CreateMemory(EXECUTABLE_REGION, code_set.size,
                            static_cast<MemoryType>(3), MemoryPermission::Read,
                            out_base);

    // Protect
    mmu->Protect(
        Range<vaddr_t>::FromSize(
            out_base + code_set.code.GetBegin(),
            align(code_set.code.GetSize(), hw::tegra_x1::cpu::GUEST_PAGE_SIZE)),
        MemoryPermission::ReadExecute);
    // mmu->Protect(
    //     Range<vaddr_t>::FromSize(out_base + code_set.ro_data.GetBegin(),
    //                              align(code_set.ro_data.GetSize(),
    //                                    hw::tegra_x1::cpu::GUEST_PAGE_SIZE)),
    //     MemoryPermission::Read);
    mmu->Protect(
        Range<vaddr_t>::FromSize(
            out_base + code_set.data.GetBegin(),
            align(code_set.data.GetSize(), hw::tegra_x1::cpu::GUEST_PAGE_SIZE)),
        MemoryPermission::ReadWrite);

    // Debug
    DEBUGGER_MANAGER_INSTANCE.GetDebugger(this).GetModuleTable().RegisterSymbol(
        {std::string(module_name),
         Range<vaddr_t>(out_base, out_base + code_set.size)});

    return ptr;
}

hw::tegra_x1::cpu::IMemory* Process::CreateTlsMemory(vaddr_t& base) {
    auto mem = CPU_INSTANCE.AllocateMemory(TLS_SIZE);
    base = tls_mem_base;
    mmu->Map(base, mem,
             {MemoryType::ThreadLocal, MemoryAttribute::None,
              MemoryPermission::ReadWrite});
    tls_mem_base += TLS_SIZE;

    return mem;
}

void Process::CreateStackMemory(usize stack_size) {
    // main_thread = new GuestThread(this, STACK_REGION.begin + stack_size -
    // 0x10, priority); auto handle_id = AddHandle(main_thread);

    main_thread_stack_mem = CPU_INSTANCE.AllocateMemory(stack_size);
    mmu->Map(STACK_REGION.GetBegin(), main_thread_stack_mem,
             {MemoryType::Stack, MemoryAttribute::None,
              MemoryPermission::ReadWrite});
}

void Process::Start() {
    // Main thread
    main_thread->Start();

    // Signal
    SignalStateChange(ProcessState::Started);
}

void Process::Stop() {
    std::lock_guard lock(thread_mutex);
    for (auto thread : threads)
        thread->Stop();

    // Signal
    SignalStateChange(ProcessState::Exiting);
}

void Process::SupervisorPause() {
    std::lock_guard lock(thread_mutex);
    for (auto thread : threads)
        thread->SupervisorPause();

    // Signal
    SignalStateChange(ProcessState::DebugSuspended);
}

void Process::SupervisorResume() {
    std::lock_guard lock(thread_mutex);
    for (auto thread : threads)
        thread->SupervisorResume();

    // Signal
    SignalStateChange(ProcessState::Started);
}

void Process::CleanUp() {
    // Heap memory
    if (heap_mem) {
        delete heap_mem;
        heap_mem = nullptr;
    }

    // Executable memories
    for (auto mem : executable_mems)
        delete mem;
    executable_mems.clear();

    // Main thread stack memory
    if (main_thread_stack_mem)
        delete main_thread_stack_mem;

    // Main thread
    if (main_thread) {
        main_thread->Release();
        main_thread = nullptr;
    }

    for (handle_id_t handle_id = 1; handle_id < handle_pool.GetCapacity() + 1;
         handle_id++) {
        if (handle_pool.IsValid(handle_id))
            handle_pool.Get(handle_id)->Release();
    }

    // Signal
    SignalStateChange(ProcessState::Exited);
}

void Process::SignalStateChange(ProcessState new_state) {
    state = new_state;
    Signal();
}

} // namespace hydra::horizon::kernel
