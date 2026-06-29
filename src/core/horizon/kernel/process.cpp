#include "core/horizon/kernel/process.hpp"

#include <random>

#include "core/debugger/debugger_manager.hpp"
#include "core/hw/tegra_x1/cpu/cpu.hpp"
#include "core/hw/tegra_x1/cpu/mmu.hpp"
#include "core/hw/tegra_x1/gpu/gmmu.hpp"
#include "core/system.hpp"

namespace hydra::horizon::kernel {

Process::Process(System& system_, std::string_view debug_name)
    : SynchronizationObject(TYPE_ID, false, debug_name), system{system_},
      mmu{system.GetCpu().CreateMmu(system)}, gmmu(mmu.get()),
      applet_state(system.GetOS().GetKernel()) {
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

uptr Process::CreateMemory(ztd::Range<vaddr_t> region, u64 size,
                           MemoryType type, MemoryPermission perm,
                           vaddr_t& out_base) {
    out_base = mmu->FindFreeMemory(region, size);
    ASSERT(out_base != 0x0, Kernel, "Failed to find free memory");

    auto mem = system.GetCpu().AllocateMemory(size);
    mmu->Map(out_base, mem,
             {.type = type, .attr = MemoryAttribute::None, .perm = perm});
    executable_mems.emplace_back(mem);

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
        ztd::Range<vaddr_t>::fromSize(
            out_base + code_set.code.getBegin(),
            align(code_set.code.getSize(), hw::tegra_x1::cpu::GUEST_PAGE_SIZE)),
        MemoryPermission::ReadExecute);
    // mmu->Protect(
    //     ztd::Range<vaddr_t>::fromSize(out_base + code_set.ro_data.getBegin(),
    //                              align(code_set.ro_data.GetSize(),
    //                                    hw::tegra_x1::cpu::GUEST_PAGE_SIZE)),
    //     MemoryPermission::Read);
    mmu->Protect(
        ztd::Range<vaddr_t>::fromSize(
            out_base + code_set.data.getBegin(),
            align(code_set.data.getSize(), hw::tegra_x1::cpu::GUEST_PAGE_SIZE)),
        MemoryPermission::ReadWrite);

    // Debug
    DEBUGGER_MANAGER_INSTANCE.GetDebugger(this).GetModuleTable().RegisterSymbol(
        {.name = std::string(module_name),
         .guest_mem_range =
             ztd::Range<vaddr_t>(out_base, out_base + code_set.size)});

    return ptr;
}

hw::tegra_x1::cpu::IMemory* Process::CreateTlsMemory(vaddr_t& base) {
    auto mem = system.GetCpu().AllocateMemory(TLS_SIZE);
    base = tls_mem_base;
    mmu->Map(base, mem,
             {.type = MemoryType::ThreadLocal,
              .attr = MemoryAttribute::None,
              .perm = MemoryPermission::ReadWrite});
    tls_mem_base += TLS_SIZE;

    return mem;
}

void Process::CreateStackMemory(u64 stack_size) {
    // main_thread = new GuestThread(this, STACK_REGION.begin + stack_size -
    // 0x10, priority); auto handle_id = AddHandle(main_thread);

    main_thread_stack_mem.reset(system.GetCpu().AllocateMemory(stack_size));
    mmu->Map(STACK_REGION.getBegin(), main_thread_stack_mem.get(),
             {.type = MemoryType::Stack,
              .attr = MemoryAttribute::None,
              .perm = MemoryPermission::ReadWrite});
}

void Process::ResizeHeap(u64 size) {
    if (heap_mem == nullptr) {
        heap_mem.reset(system.GetCpu().AllocateMemory(size));
    } else {
        mmu->Unmap(ztd::Range<vaddr_t>::fromSize(HEAP_REGION.getBegin(),
                                                 heap_mem->GetSize()));
        heap_mem->Resize(size);
    }

    mmu->Map(HEAP_REGION.getBegin(), heap_mem.get(),
             {.type = MemoryType::Normal_1_0_0,
              .attr = MemoryAttribute::None,
              .perm = MemoryPermission::ReadWrite});
}

void Process::Start() {
    // Main thread
    main_thread->Start();

    // Signal
    SignalStateChange(ProcessState::Started);
}

void Process::Stop() {
    std::scoped_lock lock(thread_mutex);
    for (auto thread : threads)
        thread->Stop();

    // Signal
    SignalStateChange(ProcessState::Exiting);
}

void Process::SupervisorPause() {
    std::scoped_lock lock(thread_mutex);
    for (auto thread : threads)
        thread->SupervisorPause();

    // Signal
    SignalStateChange(ProcessState::DebugSuspended);
}

void Process::SupervisorResume() {
    std::scoped_lock lock(thread_mutex);
    for (auto thread : threads)
        thread->SupervisorResume();

    // Signal
    SignalStateChange(ProcessState::Started);
}

void Process::CleanUp() {
    executable_mems.clear();
    main_thread_stack_mem = nullptr;
    heap_mem = nullptr;

    // Main thread
    if (main_thread != nullptr) {
        main_thread->Release();
        main_thread = nullptr;
    }

    for (const auto& obj : handle_pool) {
        obj->Release();
    }

    // Signal
    SignalStateChange(ProcessState::Exited);
}

void Process::SignalStateChange(ProcessState new_state) {
    state = new_state;
    Signal();
}

} // namespace hydra::horizon::kernel
