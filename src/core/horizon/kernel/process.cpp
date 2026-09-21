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
      mmu{system.getCpu().createMmu(system)}, gmmu(mmu.get()),
      applet_state(system.getOs().getKernel()) {
    // TODO: use title ID and name as debugger name?
    DEBUGGER_MANAGER_INSTANCE.attachDebugger(
        this,
        /*fmt::format("{:016x}", title_id)*/ getDebugName());

    // Random entropy
    std::random_device rd;
    std::mt19937_64 gen(rd());
    for (auto& random_e : random_entropy)
        random_e = gen();
}

Process::~Process() {
    cleanUp();
    DEBUGGER_MANAGER_INSTANCE.detachDebugger(this);
}

uptr Process::createMemory(ztd::Range<vaddr_t> region, u64 size,
                           MemoryType type, MemoryPermission perm,
                           vaddr_t& out_base) {
    out_base = mmu->findFreeMemory(region, size);
    ASSERT(out_base != 0x0, Kernel, "Failed to find free memory");

    auto mem = system.getCpu().allocateMemory(size);
    mmu->map(out_base, mem,
             {.type = type, .attr = MemoryAttribute::None, .perm = perm});
    executable_mems.emplace_back(mem);

    return mem->getPtr();
}

uptr Process::createExecutableMemory(const std::string_view module_name,
                                     CodeSet code_set, vaddr_t& out_base) {
    // TODO: use MemoryType::Static?
    auto ptr = createMemory(EXECUTABLE_REGION, code_set.size,
                            static_cast<MemoryType>(3), MemoryPermission::Read,
                            out_base);

    // Protect
    mmu->protect(
        ztd::Range<vaddr_t>::fromSize(
            out_base + code_set.code.getBegin(),
            align(code_set.code.getSize(), hw::tegra_x1::cpu::GUEST_PAGE_SIZE)),
        MemoryPermission::ReadExecute);
    // mmu->Protect(
    //     ztd::Range<vaddr_t>::fromSize(out_base + code_set.ro_data.getBegin(),
    //                              align(code_set.ro_data.GetSize(),
    //                                    hw::tegra_x1::cpu::GUEST_PAGE_SIZE)),
    //     MemoryPermission::Read);
    mmu->protect(
        ztd::Range<vaddr_t>::fromSize(
            out_base + code_set.data.getBegin(),
            align(code_set.data.getSize(), hw::tegra_x1::cpu::GUEST_PAGE_SIZE)),
        MemoryPermission::ReadWrite);

    // Debug
    DEBUGGER_MANAGER_INSTANCE.getDebugger(this).getModuleTable().registerSymbol(
        {.name = std::string(module_name),
         .guest_mem_range =
             ztd::Range<vaddr_t>(out_base, out_base + code_set.size)});

    return ptr;
}

hw::tegra_x1::cpu::IMemory* Process::createTlsMemory(vaddr_t& base) {
    auto mem = system.getCpu().allocateMemory(TLS_SIZE);
    base = tls_mem_base;
    mmu->map(base, mem,
             {.type = MemoryType::ThreadLocal,
              .attr = MemoryAttribute::None,
              .perm = MemoryPermission::ReadWrite});
    tls_mem_base += TLS_SIZE;

    return mem;
}

void Process::createStackMemory(u64 stack_size) {
    // main_thread = new GuestThread(this, STACK_REGION.begin + stack_size -
    // 0x10, priority); auto handle = AddHandle(main_thread);

    main_thread_stack_mem.reset(system.getCpu().allocateMemory(stack_size));
    mmu->map(STACK_REGION.getBegin(), main_thread_stack_mem.get(),
             {.type = MemoryType::Stack,
              .attr = MemoryAttribute::None,
              .perm = MemoryPermission::ReadWrite});
}

void Process::resizeHeap(u64 size) {
    if (heap_mem == nullptr) {
        heap_mem.reset(system.getCpu().allocateMemory(size));
    } else {
        mmu->unmap(ztd::Range<vaddr_t>::fromSize(HEAP_REGION.getBegin(),
                                                 heap_mem->getSize()));
        heap_mem->resize(size);
    }

    mmu->map(HEAP_REGION.getBegin(), heap_mem.get(),
             {.type = MemoryType::Normal_1_0_0,
              .attr = MemoryAttribute::None,
              .perm = MemoryPermission::ReadWrite});
}

void Process::start() {
    // Main thread
    main_thread->start();

    // Signal
    signalStateChange(ProcessState::Started);
}

void Process::stop() {
    std::scoped_lock lock(thread_mutex);
    for (auto thread : threads)
        thread->stop();

    // Signal
    signalStateChange(ProcessState::Exiting);
}

void Process::supervisorPause() {
    std::scoped_lock lock(thread_mutex);
    for (auto thread : threads)
        thread->supervisorPause();

    // Signal
    signalStateChange(ProcessState::DebugSuspended);
}

void Process::supervisorResume() {
    std::scoped_lock lock(thread_mutex);
    for (auto thread : threads)
        thread->supervisorResume();

    // Signal
    signalStateChange(ProcessState::Started);
}

void Process::cleanUp() {
    executable_mems.clear();
    main_thread_stack_mem = nullptr;
    heap_mem = nullptr;

    // Main thread
    if (main_thread != nullptr) {
        main_thread->release();
        main_thread = nullptr;
    }

    for (const auto& obj : handle_pool) {
        obj->release();
    }

    // Signal
    signalStateChange(ProcessState::Exited);
}

void Process::signalStateChange(ProcessState new_state) {
    state = new_state;
    signal();
}

} // namespace hydra::horizon::kernel
