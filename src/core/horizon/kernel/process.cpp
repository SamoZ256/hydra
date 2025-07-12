#include "core/horizon/kernel/process.hpp"

#include "core/horizon/kernel/thread.hpp"
#include "core/hw/tegra_x1/cpu/cpu.hpp"

namespace hydra::horizon::kernel {

Process::Process(const std::string_view debug_name)
    : SynchronizationObject(false, debug_name), mmu{CPU_INSTANCE.CreateMmu()} {}

Process::~Process() { CleanUp(); }

uptr Process::CreateMemory(usize size, MemoryType type, MemoryPermission perm,
                           bool add_guard_page, vaddr_t& out_base) {
    size = align(size, hw::tegra_x1::cpu::GUEST_PAGE_SIZE);
    auto mem = CPU_INSTANCE.AllocateMemory(size);
    mmu->Map(mem_base, mem, {type, MemoryAttribute::None, perm});
    executable_mems.push_back(mem);

    out_base = mem_base;
    if (add_guard_page)
        size += hw::tegra_x1::cpu::GUEST_PAGE_SIZE; // One guard page
    mem_base += size;

    return mem->GetPtr();
}

uptr Process::CreateExecutableMemory(const std::string_view module_name,
                                     usize size, MemoryPermission perm,
                                     bool add_guard_page, vaddr_t& out_base) {
    // TODO: use MemoryType::Static
    auto ptr = CreateMemory(size, static_cast<MemoryType>(3), perm,
                            add_guard_page, out_base);
    DEBUGGER_INSTANCE.GetModuleTable().RegisterSymbol(
        {std::string(module_name), range<vaddr_t>(out_base, out_base + size)});

    return ptr;
}

hw::tegra_x1::cpu::IMemory* Process::CreateTlsMemory(vaddr_t& base) {
    constexpr usize TLS_MEM_SIZE = 0x20000;

    auto mem = CPU_INSTANCE.AllocateMemory(TLS_MEM_SIZE);
    base = tls_mem_base;
    mmu->Map(base, mem,
             {MemoryType::ThreadLocal, MemoryAttribute::None,
              MemoryPermission::ReadWrite});
    tls_mem_base += TLS_MEM_SIZE;

    return mem;
}

std::pair<Thread*, handle_id_t>
Process::CreateMainThread(u8 priority, u8 core_number, u32 stack_size) {
    // Thread
    main_thread =
        new Thread(this, STACK_REGION_BASE + stack_size - 0x10, priority);
    auto handle_id = AddHandle(main_thread);

    // Stack memory
    main_thread_stack_mem = CPU_INSTANCE.AllocateMemory(stack_size);
    mmu->Map(STACK_REGION_BASE, main_thread_stack_mem,
             {MemoryType::Stack, MemoryAttribute::None,
              MemoryPermission::ReadWrite});

    return {main_thread, handle_id};
}

void Process::Start() {
    main_thread->Start();

    // Signal
    SignalStateChange(ProcessState::Started);
}

void Process::RequestStop() {
    std::lock_guard lock(thread_mutex);
    for (auto thread : threads)
        thread->RequestStop();

    // Signal
    SignalStateChange(ProcessState::Exiting);
}

void Process::CleanUp() {
    if (heap_mem)
        delete heap_mem;
    for (auto mem : executable_mems)
        delete mem;
    if (main_thread_stack_mem)
        delete main_thread_stack_mem;
    handle_pool.CleanUp();

    // Signal
    SignalStateChange(ProcessState::Exited);
}

void Process::SignalStateChange(ProcessState new_state) {
    state = new_state;
    Signal();
}

} // namespace hydra::horizon::kernel
