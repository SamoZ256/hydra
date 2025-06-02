#include "core/horizon/kernel/kernel.hpp"

#include "core/horizon/kernel/cmif.hpp"
#include "core/horizon/kernel/session.hpp"
#include "core/horizon/kernel/thread.hpp"
#include "core/hw/tegra_x1/cpu/cpu_base.hpp"
#include "core/hw/tegra_x1/cpu/mmu_base.hpp"
#include "core/hw/tegra_x1/cpu/thread_base.hpp"

namespace hydra::horizon::kernel {

SINGLETON_DEFINE_GET_INSTANCE(Kernel, Kernel)

Kernel::Kernel(hw::Bus& bus_, hw::tegra_x1::cpu::MMUBase* mmu_)
    : bus{bus_}, mmu{mmu_} {
    SINGLETON_SET_INSTANCE(Kernel, Kernel);
}

Kernel::~Kernel() {
    // TODO: also unmap
    mmu->FreeMemory(heap_mem);
    for (auto mem : executable_mems)
        mmu->FreeMemory(mem);

    SINGLETON_UNSET_INSTANCE();
}

uptr Kernel::CreateRomMemory(usize size, MemoryType type, MemoryPermission perm,
                             bool add_guard_page, vaddr_t& out_base) {
    size = align(size, hw::tegra_x1::cpu::GUEST_PAGE_SIZE);
    auto mem = mmu->AllocateMemory(size);
    mmu->Map(executable_mem_base, mem, {type, MemoryAttribute::None, perm});
    executable_mems.push_back(mem);

    out_base = executable_mem_base;
    if (add_guard_page)
        size += hw::tegra_x1::cpu::GUEST_PAGE_SIZE; // One guard page
    executable_mem_base += size;

    return mmu->GetMemoryPtr(mem);
}

uptr Kernel::CreateExecutableMemory(usize size, MemoryPermission perm,
                                    bool add_guard_page, vaddr_t& out_base) {
    // TODO: use MemoryType::Static
    return CreateRomMemory(size, static_cast<MemoryType>(3), perm,
                           add_guard_page, out_base);
}

bool Kernel::SupervisorCall(hw::tegra_x1::cpu::ThreadBase* thread, u64 id) {
    result_t res;
    i32 tmp_i32;
    u32 tmp_u32;
    u64 tmp_u64;
    uptr tmp_uptr;
    handle_id_t tmp_handle_id;
    switch (id) {
    case 0x1:
        res = svcSetHeapSize(thread->GetRegX(1), tmp_uptr);
        thread->SetRegW(0, res);
        thread->SetRegX(1, tmp_uptr);
        break;
    case 0x2:
        res = svcSetMemoryPermission(
            thread->GetRegX(0), thread->GetRegX(1),
            static_cast<MemoryPermission>(thread->GetRegX(2)));
        thread->SetRegW(0, res);
        break;
    case 0x3:
        res = svcSetMemoryAttribute(thread->GetRegX(0), thread->GetRegX(1),
                                    thread->GetRegX(2), thread->GetRegX(3));
        thread->SetRegW(0, res);
        break;
    case 0x4:
        res = svcMapMemory(thread->GetRegX(0), thread->GetRegX(1),
                           thread->GetRegX(2));
        thread->SetRegW(0, res);
        break;
    case 0x5:
        res = svcUnmapMemory(thread->GetRegX(0), thread->GetRegX(1),
                             thread->GetRegX(2));
        thread->SetRegW(0, res);
        break;
    case 0x6:
        res = svcQueryMemory(
            thread->GetRegX(2),
            *reinterpret_cast<MemoryInfo*>(mmu->UnmapAddr(thread->GetRegX(0))),
            tmp_u32);
        thread->SetRegW(0, res);
        thread->SetRegX(1, tmp_u32);
        break;
    case 0x7:
        svcExitProcess();
        // TODO: exit all threads?
        return false;
    case 0x8:
        res = svcCreateThread(
            thread->GetRegX(1), thread->GetRegX(2), thread->GetRegX(3),
            std::bit_cast<i32>(thread->GetRegW(4)),
            std::bit_cast<i32>(thread->GetRegW(5)), tmp_handle_id);
        thread->SetRegW(0, res);
        thread->SetRegX(1, tmp_handle_id);
        break;
    case 0x9:
        res = svcStartThread(thread->GetRegW(0));
        thread->SetRegW(0, res);
        break;
    case 0xa:
        svcExitThread();
        return false;
    case 0xb:
        svcSleepThread(std::bit_cast<i64>(thread->GetRegX(0)));
        break;
    case 0xc:
        res = svcGetThreadPriority(thread->GetRegX(1), tmp_i32);
        thread->SetRegW(0, res);
        thread->SetRegW(1, std::bit_cast<u32>(tmp_i32));
        break;
    case 0xd:
        res = svcSetThreadPriority(thread->GetRegX(0), thread->GetRegX(1));
        thread->SetRegW(0, res);
        break;
    case 0xe:
        res = svcGetThreadCoreMask(thread->GetRegW(2), tmp_i32, tmp_u64);
        thread->SetRegW(0, res);
        thread->SetRegW(1, tmp_i32);
        thread->SetRegW(2, tmp_u64);
        break;
    case 0xf:
        res = svcSetThreadCoreMask(thread->GetRegW(0),
                                   std::bit_cast<i32>(thread->GetRegW(1)),
                                   thread->GetRegX(2));
        thread->SetRegW(0, res);
        break;
    case 0x10:
        svcGetCurrentProcessorNumber(tmp_u32);
        thread->SetRegW(0, tmp_u32);
        break;
    case 0x11:
        res = svcSignalEvent(thread->GetRegW(0));
        thread->SetRegW(0, res);
        break;
    case 0x12:
        res = svcClearEvent(thread->GetRegW(0));
        thread->SetRegW(0, res);
        break;
    case 0x13:
        res = svcMapSharedMemory(
            thread->GetRegX(0), thread->GetRegX(1), thread->GetRegX(2),
            static_cast<MemoryPermission>(thread->GetRegX(3)));
        thread->SetRegW(0, res);
        break;
    case 0x14:
        res = svcUnmapSharedMemory(thread->GetRegX(0), thread->GetRegX(1),
                                   thread->GetRegX(2));
        thread->SetRegW(0, res);
        break;
    case 0x15:
        res = svcCreateTransferMemory(
            thread->GetRegX(1), thread->GetRegX(2),
            static_cast<MemoryPermission>(thread->GetRegX(3)), tmp_handle_id);
        thread->SetRegW(0, res);
        thread->SetRegX(1, tmp_handle_id);
        break;
    case 0x16:
        res = svcCloseHandle(thread->GetRegX(0));
        thread->SetRegW(0, res);
        break;
    case 0x17:
        res = svcResetSignal(thread->GetRegX(0));
        thread->SetRegW(0, res);
        break;
    case 0x18:
        res = svcWaitSynchronization(
            reinterpret_cast<handle_id_t*>(mmu->UnmapAddr(thread->GetRegX(1))),
            std::bit_cast<i64>(thread->GetRegX(2)),
            std::bit_cast<i64>(thread->GetRegX(3)), tmp_u64);
        thread->SetRegW(0, res);
        thread->SetRegX(1, tmp_u64);
        break;
    case 0x19:
        res = svcCancelSynchronization(thread->GetRegW(0));
        thread->SetRegW(0, res);
        break;
    case 0x1a:
        res = svcArbitrateLock(thread->GetRegX(0), thread->GetRegX(1),
                               thread->GetRegX(2));
        thread->SetRegW(0, res);
        break;
    case 0x1b:
        res = svcArbitrateUnlock(thread->GetRegX(0));
        thread->SetRegW(0, res);
        break;
    case 0x1c:
        res = svcWaitProcessWideKeyAtomic(
            thread->GetRegX(0), thread->GetRegX(1), thread->GetRegX(2),
            std::bit_cast<i64>(thread->GetRegX(3)));
        thread->SetRegW(0, res);
        break;
    case 0x1d:
        res = svcSignalProcessWideKey(thread->GetRegX(0), thread->GetRegX(1));
        thread->SetRegW(0, res);
        break;
    case 0x1e:
        svcGetSystemTick(tmp_u64);
        thread->SetRegX(0, tmp_u64);
        break;
    case 0x1f:
        res = svcConnectToNamedPort(
            reinterpret_cast<const char*>(mmu->UnmapAddr(thread->GetRegX(1))),
            tmp_handle_id);
        thread->SetRegW(0, res);
        thread->SetRegX(1, tmp_handle_id);
        break;
    case 0x21:
        res = svcSendSyncRequest(thread->GetTlsMemory(), thread->GetRegX(0));
        thread->SetRegW(0, res);
        break;
    case 0x25:
        res = svcGetThreadId(static_cast<handle_id_t>(thread->GetRegX(1)),
                             tmp_u64);
        thread->SetRegW(0, res);
        thread->SetRegX(1, tmp_u64);
        break;
    case 0x26:
        // Debug
        thread->LogRegisters();

        res = svcBreak(BreakReason(thread->GetRegX(0)),
                       mmu->UnmapAddr(thread->GetRegX(1)), thread->GetRegX(2));
        thread->SetRegW(0, res);
        break;
    case 0x27:
        res = svcOutputDebugString(
            reinterpret_cast<const char*>(mmu->UnmapAddr(thread->GetRegX(0))),
            thread->GetRegX(1));
        thread->SetRegW(0, res);
        break;
    case 0x29:
        res = svcGetInfo(static_cast<InfoType>(thread->GetRegX(1)),
                         thread->GetRegX(2), thread->GetRegX(3), tmp_u64);
        thread->SetRegW(0, res);
        thread->SetRegX(1, tmp_u64);
        break;
    case 0x2c:
        res = svcMapPhysicalMemory(thread->GetRegX(0), thread->GetRegX(1));
        thread->SetRegW(0, res);
        break;
    case 0x32:
        res = svcSetThreadActivity(
            thread->GetRegW(0),
            static_cast<ThreadActivity>(thread->GetRegW(1)));
        thread->SetRegW(0, res);
        break;
    case 0x33:
        res = svcGetThreadContext3(thread->GetRegW(1),
                                   *reinterpret_cast<ThreadContext*>(
                                       mmu->UnmapAddr(thread->GetRegX(0))));
        thread->SetRegW(0, res);
        break;
    default:
        LOG_NOT_IMPLEMENTED(Kernel, "SVC 0x{:x}", id);
        res = MAKE_RESULT(Svc, Error::NotImplemented);
        thread->SetRegW(0, res);
        break;
    }

    return true;
}

result_t Kernel::svcSetHeapSize(usize size, uptr& out_base) {
    LOG_DEBUG(Kernel, "svcSetHeapSize called (size: 0x{:08x})", size);

    if ((size % HEAP_MEM_ALIGNMENT) != 0)
        return MAKE_RESULT(Svc, Error::InvalidSize); // TODO: correct?

    // TODO: handle this more cleanly?
    if (!heap_mem) {
        heap_mem = mmu->AllocateMemory(size);
        mmu->Map(HEAP_REGION_BASE, heap_mem,
                 {MemoryType::Normal_1_0_0, MemoryAttribute::None,
                  MemoryPermission::ReadWriteExecute});
    } else {
        mmu->ResizeHeap(heap_mem, HEAP_REGION_BASE, size);
    }

    out_base = HEAP_REGION_BASE;

    return RESULT_SUCCESS;
}

result_t Kernel::svcSetMemoryPermission(uptr addr, usize size,
                                        MemoryPermission perm) {
    LOG_DEBUG(
        Kernel,
        "svcSetMemoryPermission called (addr: 0x{:08x}, size: 0x{:08x}, perm: "
        "{})",
        addr, size, perm);

    // TODO: implement
    LOG_FUNC_STUBBED(Kernel);

    return RESULT_SUCCESS;
}

result_t Kernel::svcSetMemoryAttribute(uptr addr, usize size, u32 mask,
                                       u32 value) {
    LOG_DEBUG(
        Kernel,
        "svcSetMemoryAttribute called (addr: 0x{:08x}, size: 0x{:08x}, mask: "
        "0x{:08x}, value: 0x{:08x})",
        addr, size, mask, value);

    // TODO: implement
    LOG_FUNC_STUBBED(Kernel);

    return RESULT_SUCCESS;
}

result_t Kernel::svcMapMemory(uptr dst_addr, uptr src_addr, usize size) {
    LOG_DEBUG(
        Kernel,
        "svcMapMemory called (dst_addr: 0x{:08x}, src_addr: 0x{:08x}, size: "
        "0x{:08x})",
        dst_addr, src_addr, size);

    mmu->Map(dst_addr, src_addr, size);

    return RESULT_SUCCESS;
}

result_t Kernel::svcUnmapMemory(uptr dst_addr, uptr src_addr, usize size) {
    LOG_DEBUG(
        Kernel,
        "svcUnmapMemory called (dst_addr: 0x{:08x}, src_addr: 0x{:08x}, size: "
        "0x{:08x})",
        dst_addr, src_addr, size);

    // TODO: check if src_addr is the same as the one used in svcMapMemory

    mmu->Unmap(dst_addr, size);

    return RESULT_SUCCESS;
}

result_t Kernel::svcQueryMemory(uptr addr, MemoryInfo& out_mem_info,
                                u32& out_page_info) {
    LOG_DEBUG(Kernel, "svcQueryMemory called (addr: 0x{:08x})", addr);

    out_mem_info = mmu->QueryMemory(addr);

    // TODO: what is this?
    out_page_info = 0;

    return RESULT_SUCCESS;
}

void Kernel::svcExitProcess() { LOG_DEBUG(Kernel, "svcExitProcess called"); }

result_t Kernel::svcCreateThread(vaddr_t entry_point, vaddr_t args_addr,
                                 vaddr_t stack_top_addr, i32 priority,
                                 i32 processor_id,
                                 handle_id_t& out_thread_handle_id) {
    LOG_DEBUG(Kernel,
              "svcCreateThread called (entry_point: 0x{:08x}, args_addr: "
              "0x{:08x}, stack_top_addr: 0x{:08x}, priority: {}, "
              "processor_id: {})",
              entry_point, args_addr, stack_top_addr, priority, processor_id);

    // Thread
    // TODO: processor ID
    auto thread = new Thread(stack_top_addr, priority);
    thread->SetEntryPoint(entry_point);
    thread->SetArg(0, args_addr);
    out_thread_handle_id = AddHandle(thread);

    return RESULT_SUCCESS;
}

result_t Kernel::svcStartThread(handle_id_t thread_handle_id) {
    LOG_DEBUG(Kernel, "svcStartThread called (thread: 0x{:08x})",
              thread_handle_id);

    // Start thread
    auto thread = dynamic_cast<Thread*>(GetHandle(thread_handle_id));
    ASSERT_DEBUG(thread, Kernel, "Handle 0x{:x} is not a Thread",
                 thread_handle_id);
    thread->Run();

    return RESULT_SUCCESS;
}

void Kernel::svcExitThread() { LOG_DEBUG(Kernel, "svcExitThread called"); }

void Kernel::svcSleepThread(i64 nano) {
    LOG_DEBUG(Kernel, "svcSleepThread called (nano: {})", nano);

    if (nano == 0 || nano == -1 || nano == -2)
        std::this_thread::yield();
    else
        std::this_thread::sleep_for(std::chrono::nanoseconds(nano));
}

result_t Kernel::svcGetThreadPriority(handle_id_t thread_handle_id,
                                      i32& out_priority) {
    LOG_DEBUG(Kernel, "svcGetThreadPriority called (thread: 0x{:08x})",
              thread_handle_id);

    // TODO: implement
    LOG_FUNC_STUBBED(Kernel);

    // HACK
    out_priority = 0x20; // 0x0 - 0x3f, lower is higher priority

    return RESULT_SUCCESS;
}

result_t Kernel::svcSetThreadPriority(handle_id_t thread_handle_id,
                                      i32 priority) {
    LOG_DEBUG(
        Kernel,
        "svcSetThreadPriority called (thread: 0x{:08x}, priority: 0x{:x})",
        thread_handle_id, priority);

    // TODO: implement
    LOG_FUNC_STUBBED(Kernel);

    return RESULT_SUCCESS;
}

result_t Kernel::svcGetThreadCoreMask(handle_id_t thread_handle_id,
                                      i32& out_core_mask0,
                                      u64& out_core_mask1) {
    LOG_DEBUG(Kernel, "svcGetThreadCoreMask called (thread: 0x{:08x})",
              thread_handle_id);

    // TODO: implement
    LOG_FUNC_STUBBED(Kernel);

    // HACK
    out_core_mask0 = 0x1;
    out_core_mask1 = 0x2;

    return RESULT_SUCCESS;
}

result_t Kernel::svcSetThreadCoreMask(handle_id_t thread_handle_id,
                                      i32 core_mask0, u64 core_mask1) {
    LOG_DEBUG(Kernel,
              "svcSetThreadCoreMask called (thread: 0x{:08x}, core_mask0: "
              "0x{:08x}, core_mask1: 0x{:08x})",
              thread_handle_id, core_mask0, core_mask1);

    // TODO: implement
    LOG_FUNC_STUBBED(Kernel);

    return RESULT_SUCCESS;
}

void Kernel::svcGetCurrentProcessorNumber(u32& out_number) {
    LOG_DEBUG(Kernel, "svcGetCurrentProcessorNumber called");

    // TODO: implement
    LOG_FUNC_STUBBED(Kernel);

    // HACK
    out_number = 0;
}

result_t Kernel::svcSignalEvent(handle_id_t event_handle_id) {
    LOG_DEBUG(Kernel, "svcSignalEvent called (event: 0x{:08x})",
              event_handle_id);

    auto handle = dynamic_cast<Event*>(GetHandle(event_handle_id));
    ASSERT_DEBUG(handle, Kernel, "Handle {} is not an event handle",
                 event_handle_id);

    handle->Signal();

    return RESULT_SUCCESS;
}

result_t Kernel::svcClearEvent(handle_id_t event_handle_id) {
    LOG_DEBUG(Kernel, "svcClearEvent called (event: 0x{:08x})",
              event_handle_id);

    auto event = dynamic_cast<Event*>(GetHandle(event_handle_id));
    ASSERT_DEBUG(event, Kernel, "Handle {} is not an event handle",
                 event_handle_id);

    if (!event->Clear())
        return MAKE_RESULT(Svc, Error::InvalidState); // TODO: correct?

    return RESULT_SUCCESS;
}

result_t Kernel::svcMapSharedMemory(handle_id_t shared_mem_handle_id, uptr addr,
                                    usize size, MemoryPermission perm) {
    LOG_DEBUG(
        Kernel,
        "svcMapSharedMemory called (handle: 0x{:08x}, addr: 0x{:08x}, size: "
        "0x{:08x}, perm: {})",
        shared_mem_handle_id, addr, size, perm);

    // Map
    auto shared_mem =
        dynamic_cast<SharedMemory*>(handle_pool.Get(shared_mem_handle_id));
    ASSERT_DEBUG(shared_mem, Kernel,
                 "Handle 0x{:x} is not a shared memory handle",
                 shared_mem_handle_id);
    shared_mem->MapToRange(range(addr, addr + size), perm);

    return RESULT_SUCCESS;
}

result_t Kernel::svcUnmapSharedMemory(handle_id_t shared_mem_handle_id,
                                      uptr addr, usize size) {
    LOG_DEBUG(
        Kernel,
        "svcUnmapSharedMemory called (handle: 0x{:08x}, addr: 0x{:08x}, size: "
        "0x{:08x})",
        shared_mem_handle_id, addr, size);

    // Map
    auto shared_mem =
        dynamic_cast<SharedMemory*>(handle_pool.Get(shared_mem_handle_id));
    ASSERT_DEBUG(shared_mem, Kernel,
                 "Handle 0x{:x} is not a shared memory handle",
                 shared_mem_handle_id);
    // TODO: unmap
    LOG_FUNC_STUBBED(Kernel);

    return RESULT_SUCCESS;
}

result_t
Kernel::svcCreateTransferMemory(uptr addr, u64 size, MemoryPermission perm,
                                handle_id_t& out_transfer_mem_handle_id) {
    LOG_DEBUG(
        Kernel,
        "svcCreateTransferMemory called (address: 0x{:08x}, size: 0x{:08x}, "
        "perm: {})",
        addr, size, perm);

    out_transfer_mem_handle_id =
        AddHandle(new TransferMemory(addr, size, perm));

    return RESULT_SUCCESS;
}

result_t Kernel::svcCloseHandle(handle_id_t handle_id) {
    LOG_DEBUG(Kernel, "svcCloseHandle called (handle: 0x{:x})", handle_id);

    // TODO: uncomment
    // FreeHandle(handle_id);

    return RESULT_SUCCESS;
}

result_t Kernel::svcResetSignal(handle_id_t handle_id) {
    LOG_DEBUG(Kernel, "svcResetSignal called (handle: 0x{:x})", handle_id);

    // TODO: correct?
    auto handle = dynamic_cast<Event*>(GetHandle(handle_id));
    ASSERT_DEBUG(handle, Kernel, "Handle {} is not an event handle", handle_id);

    if (!handle->Clear())
        return MAKE_RESULT(Svc, Error::InvalidState);

    return RESULT_SUCCESS;
}

result_t Kernel::svcWaitSynchronization(handle_id_t* handle_ids,
                                        i32 handle_count, i64 timeout,
                                        u64& out_handle_index) {
    LOG_DEBUG(
        Kernel,
        "svcWaitSynchronization called (handles: 0x{}, count: {}, timeout: "
        "{})",
        (void*)handle_ids, handle_count, timeout);

    // TODO: implement multiple handles
    ASSERT_DEBUG(handle_count <= 1, Kernel,
                 "Multiple handles not "
                 "implemented");

    if (handle_count == 0) {
        // TODO: allow waiting forever
        ASSERT(timeout != INFINITE_TIMEOUT, Kernel,
               "Infinite timeout not implemented");
        std::this_thread::sleep_for(std::chrono::nanoseconds(timeout));
        out_handle_index = 0;

        return MAKE_RESULT(Svc, Error::TimedOut);
    } else {
        handle_id_t handle_id = handle_ids[0];
        auto event = dynamic_cast<Event*>(GetHandle(handle_id));

        // HACK
        if (!event) {
            LOG_WARN(Kernel, "Handle 0x{:x} is not an event handle", handle_id);
            std::this_thread::sleep_for(std::chrono::milliseconds(33));
            return RESULT_SUCCESS;
        }

        ASSERT_DEBUG(event, Kernel, "Handle 0x{:x} is not an event handle",
                     handle_id);

        LOG_DEBUG(Kernel, "Synchronizing with handle 0x{:x}", handle_id);

        if (!event->Wait(timeout))
            return MAKE_RESULT(Svc, Error::TimedOut);
    }

    return RESULT_SUCCESS;
}

result_t Kernel::svcCancelSynchronization(handle_id_t thread_handle_id) {
    LOG_DEBUG(Kernel, "svcCancelSynchronization called (thread: 0x{:x})",
              thread_handle_id);

    // TODO: implement
    LOG_FUNC_NOT_IMPLEMENTED(Kernel);

    return RESULT_SUCCESS;
}

result_t Kernel::svcArbitrateLock(u32 wait_tag, uptr mutex_addr, u32 self_tag) {
    LOG_DEBUG(Kernel,
              "svcArbitrateLock called (wait: 0x{:08x}, mutex: 0x{:08x}, self: "
              "0x{:08x})",
              wait_tag, mutex_addr, self_tag);

    sync_mutex.lock();
    auto& mutex = mutex_map[mutex_addr];
    sync_mutex.unlock();
    mutex.Lock(*reinterpret_cast<u32*>(mmu->UnmapAddr(mutex_addr)), self_tag);

    return RESULT_SUCCESS;
}

result_t Kernel::svcArbitrateUnlock(uptr mutex_addr) {
    LOG_DEBUG(Kernel, "svcArbitrateUnlock called (mutex: 0x{:08x})",
              mutex_addr);

    sync_mutex.lock();
    auto& mutex = mutex_map[mutex_addr];
    sync_mutex.unlock();
    mutex.Unlock(*reinterpret_cast<u32*>(mmu->UnmapAddr(mutex_addr)));

    // HACK
    /*
    if (mutex_addr == 0x40bae248) {
        static bool hack = false;
        if (!hack) {
            hack = true;
            std::this_thread::sleep_for(std::chrono::seconds(20));
        }
    }
    */

    return RESULT_SUCCESS;
}

result_t Kernel::svcWaitProcessWideKeyAtomic(uptr mutex_addr, uptr var_addr,
                                             u32 self_tag, i64 timeout) {
    LOG_DEBUG(
        Kernel,
        "svcWaitProcessWideKeyAtomic called (mutex: 0x{:08x}, var: 0x{:08x}, "
        "self: 0x{:08x}, timeout: {})",
        mutex_addr, var_addr, self_tag, timeout);

    sync_mutex.lock();
    auto& mutex = mutex_map[mutex_addr];
    auto& cond_var = cond_var_map[var_addr];
    sync_mutex.unlock();

    // TODO: correct?
    auto& value = *reinterpret_cast<u32*>(mmu->UnmapAddr(mutex_addr));
    mutex.Unlock(value);

    {
        std::unique_lock lock(mutex.GetNativeHandle());
        if (timeout == INFINITE_TIMEOUT)
            cond_var.wait(lock);
        else
            cond_var.wait_for(lock, std::chrono::nanoseconds(timeout));
    }

    mutex.Lock(value, self_tag);

    return RESULT_SUCCESS;
}

result_t Kernel::svcSignalProcessWideKey(uptr addr, i32 count) {
    LOG_DEBUG(Kernel,
              "svcSignalProcessWideKey called (addr: 0x{:08x}, count: {})",
              addr, count);

    sync_mutex.lock();
    auto& cond_var = cond_var_map[addr];
    sync_mutex.unlock();

    if (count == -1) {
        cond_var.notify_all();
    } else {
        ASSERT_DEBUG(count > 0, Kernel, "Invalid signal count {}", count);

        // TODO: correct?
        for (u32 i = 0; i < count; i++)
            cond_var.notify_one();
    }

    return RESULT_SUCCESS;
}

void Kernel::svcGetSystemTick(u64& out_tick) {
    LOG_DEBUG(Kernel, "svcGetSystemTick called");

    out_tick = get_absolute_time();
}

result_t Kernel::svcConnectToNamedPort(const std::string& name,
                                       handle_id_t& out_session_handle_id) {
    LOG_DEBUG(Kernel, "svcConnectToNamedPort called (name: {})", name);

    // TODO: don't construct a new string?
    auto it = service_ports.find(std::string(name));
    if (it == service_ports.end()) {
        LOG_ERROR(Kernel, "Unknown service name \"{}\"", name);
        return MAKE_RESULT(Svc, Error::NotFound);
    }

    out_session_handle_id = AddHandle(new Session(it->second));

    return RESULT_SUCCESS;
}

result_t Kernel::svcSendSyncRequest(hw::tegra_x1::cpu::MemoryBase* tls_mem,
                                    handle_id_t session_handle_id) {
    LOG_DEBUG(Kernel, "svcSendSyncRequest called (handle: 0x{:x})",
              session_handle_id);

    auto session = dynamic_cast<Session*>(GetHandle(session_handle_id));

    // HACK
    if (!session) {
        LOG_WARN(Kernel, "Handle 0x{:x} is not a session handle",
                 session_handle_id);
        return RESULT_SUCCESS;
    }

    ASSERT_DEBUG(session, Kernel, "Handle 0x{:x} is not a session handle",
                 session_handle_id);
    auto tls_ptr = reinterpret_cast<void*>(mmu->GetMemoryPtr(tls_mem));

    // Request

    // HIPC header
    hipc::ParsedRequest hipc_in = hipc::parse_request(tls_ptr);
    u8* in_ptr = align_ptr((u8*)hipc_in.data.data_words, 0x10);

    // Scratch memory
    u8 scratch_buffer[0x200];
    u8 scratch_buffer_objects[0x100];
    u8 scratch_buffer_copy_handles[0x100];
    u8 scratch_buffer_move_handles[0x100];

    // Request context
    hipc::Readers readers(mmu, hipc_in);
    hipc::Writers writers(mmu, hipc_in, scratch_buffer, scratch_buffer_objects,
                          scratch_buffer_copy_handles,
                          scratch_buffer_move_handles);
    RequestContext context{
        readers,
        writers,
        [&](ServiceBase* service) {
            auto session = new Session(service);
            handle_id_t handle_id = AddHandle(session);
            session->SetHandleId(handle_id);
            writers.move_handles_writer.Write(handle_id);
        },
        [&](handle_id_t handle_id) {
            return static_cast<Session*>(GetHandle(handle_id))->GetService();
        },
    };

    // Dispatch
    auto command_type = static_cast<cmif::CommandType>(hipc_in.meta.type);
    switch (command_type) {
    case cmif::CommandType::Close:
    case cmif::CommandType::TipcClose:
        session->Close();
        break;
    case cmif::CommandType::Request:
    case cmif::CommandType::RequestWithContext: {
        // TODO: how is RequestWithContext different?

        session->Request(context);
        break;
    }
    case cmif::CommandType::Control:
    case cmif::CommandType::ControlWithContext:
        // TODO: how is ControlWithContext different?

        session->Control(readers, writers);
        break;
    default:
        if (command_type >= cmif::CommandType::TipcCommandRegion) {
            const auto tipc_command =
                (u32)command_type - (u32)cmif::CommandType::TipcCommandRegion;
            LOG_ERROR(Kernel, "TIPC command {}", tipc_command);
            session->Request(context);
            break;
        }

        LOG_WARN(Kernel, "Unknown command {}", command_type);
        break;
    }

    // Response

    // HIPC header
#define GET_ARRAY_SIZE(writer)                                                 \
    static_cast<u32>(align(writers.writer.GetWrittenSize(), (usize)4) /        \
                     sizeof(u32))

#define WRITE_ARRAY(writer, ptr)                                               \
    if (ptr) {                                                                 \
        memcpy(ptr, writers.writer.GetBase(),                                  \
               writers.writer.GetWrittenSize());                               \
    }

    hipc::Metadata meta{.num_data_words = GET_ARRAY_SIZE(writer) +
                                          GET_ARRAY_SIZE(objects_writer),
                        .num_copy_handles = GET_ARRAY_SIZE(copy_handles_writer),
                        .num_move_handles =
                            GET_ARRAY_SIZE(move_handles_writer)};
    auto response = hipc::make_request(tls_ptr, meta);

    u8* data_start =
        reinterpret_cast<u8*>(align_ptr(response.data_words, 0x10));
    WRITE_ARRAY(writer, data_start);
    if (writers.objects_writer.GetWrittenSize() != 0) {
        memcpy(data_start + GET_ARRAY_SIZE(writer) * sizeof(u32),
               writers.objects_writer.GetBase(),
               writers.objects_writer.GetWrittenSize());
    }
    WRITE_ARRAY(copy_handles_writer, response.copy_handles);
    WRITE_ARRAY(move_handles_writer, response.move_handles);

#undef GET_ARRAY_SIZE
#undef WRITE_ARRAY

    return RESULT_SUCCESS;
}

result_t Kernel::svcGetThreadId(handle_id_t thread_handle_id,
                                u64& out_thread_id) {
    LOG_DEBUG(Kernel, "svcGetThreadId called (thread: 0x{:08x})",
              thread_handle_id);

    // TODO: implement
    LOG_FUNC_STUBBED(Services);

    // HACK
    out_thread_id = thread_handle_id;

    return RESULT_SUCCESS;
}

result_t Kernel::svcBreak(BreakReason reason, uptr buffer_ptr,
                          usize buffer_size) {
    LOG_DEBUG(Kernel,
              "svcBreak called (reason: {}, buffer ptr: 0x{:08x}, buffer "
              "size: 0x{:08x})",
              reason.type, buffer_ptr, buffer_size);

    if (buffer_ptr) {
        if (buffer_size == sizeof(u32)) {
            const u32 result = *reinterpret_cast<u32*>(buffer_ptr);
            const auto module = GET_RESULT_MODULE(result);
            const auto description = GET_RESULT_DESCRIPTION(result);
            LOG_INFO(Kernel, "Module: {}, description: {}", module,
                     description);
        } else {
            for (u32 i = 0; i < buffer_size / sizeof(u32); i++) {
                const u32 value = reinterpret_cast<u32*>(buffer_ptr)[i];
                LOG_INFO(Kernel, "0x{:08x}", value);
            }
        }
    }

    if (!reason.notification_only)
        throw;

    return RESULT_SUCCESS;
}

result_t Kernel::svcOutputDebugString(const char* str, usize len) {
    LOG_DEBUG(Kernel, "svcOutputDebugString called");
    if (len != 0) {
        // TODO: handle differently
        LOG_INFO(Kernel, "{}", std::string(str, len));
    }

    return RESULT_SUCCESS;
}

result_t Kernel::svcGetInfo(InfoType info_type, handle_id_t handle_id,
                            u64 info_sub_type, u64& out_info) {
    LOG_DEBUG(Kernel,
              "svcGetInfo called (type: {}, handle: 0x{:08x}, subtype: {})",
              info_type, handle_id, info_sub_type);

    switch (info_type) {
    case InfoType::CoreMask:
        LOG_NOT_IMPLEMENTED(Kernel, "CoreMask");
        // HACK
        out_info = 0xf;
        return RESULT_SUCCESS;
    case InfoType::AliasRegionAddress:
        out_info = ALIAS_REGION_BASE;
        return RESULT_SUCCESS;
    case InfoType::AliasRegionSize:
        out_info = ALIAS_REGION_SIZE;
        return RESULT_SUCCESS;
    case InfoType::HeapRegionAddress:
        out_info = HEAP_REGION_BASE;
        return RESULT_SUCCESS;
    case InfoType::HeapRegionSize:
        out_info = HEAP_REGION_SIZE;
        return RESULT_SUCCESS;
    case InfoType::TotalMemorySize:
        // TODO: what should this be?
        out_info = 3u * 1024u * 1024u * 1024u;
        return RESULT_SUCCESS;
    case InfoType::UsedMemorySize: {
        // TODO: correct?
        /*
        usize size = stack_mem->GetSize() + kernel_mem->GetSize() +
                     tls_mem->GetSize() + heap_mem->GetSize();
        for (auto executable_mem : executable_memories)
            size += executable_mem->GetSize();
        out_info = size;
        */
        out_info = 4u * 1024u * 1024u;
        return RESULT_SUCCESS;
    }
    case InfoType::DebuggerAttached:
        // TODO: make this configurable
        out_info = true;
        return RESULT_SUCCESS;
    case InfoType::RandomEntropy:
        // TODO: correct?
        // TODO: subtype 0-3
        out_info = rand();
        return RESULT_SUCCESS;
    case InfoType::AslrRegionAddress:
        out_info = ADDRESS_SPACE_START;
        return RESULT_SUCCESS;
    case InfoType::AslrRegionSize:
        out_info = ADDRESS_SPACE_SIZE;
        return RESULT_SUCCESS;
    case InfoType::StackRegionAddress:
        out_info = STACK_REGION_BASE;
        return RESULT_SUCCESS;
    case InfoType::StackRegionSize:
        out_info = STACK_REGION_SIZE;
        return RESULT_SUCCESS;
    case InfoType::TotalSystemResourceSize:
        LOG_NOT_IMPLEMENTED(Kernel, "TotalSystemResourceSize");
        // HACK
        out_info = 2u * 1024u * 1024u;
        return RESULT_SUCCESS;
    case InfoType::UsedSystemResourceSize:
        LOG_NOT_IMPLEMENTED(Kernel, "UsedSystemResourceSize");
        // HACK
        out_info = 64 * 1024;
        return RESULT_SUCCESS;
    case InfoType::TotalNonSystemMemorySize:
        LOG_NOT_IMPLEMENTED(Kernel, "TotalNonSystemMemorySize");
        // HACK
        out_info = 2u * 1024u * 1024u * 1024u;
        return RESULT_SUCCESS;
    case InfoType::UsedNonSystemMemorySize:
        LOG_NOT_IMPLEMENTED(Kernel, "UsedNonSystemMemorySize");
        // HACK
        out_info = 16u * 1024u * 1024u;
        return RESULT_SUCCESS;
    case InfoType::AliasRegionExtraSize:
        LOG_NOT_IMPLEMENTED(Kernel, "AliasRegionExtraSize");
        // HACK
        out_info = 0;
        return RESULT_SUCCESS;
    default:
        LOG_WARN(Kernel, "Unimplemented info type {}", info_type);
        return MAKE_RESULT(Svc, 0x78);
    }
}

result_t Kernel::svcMapPhysicalMemory(vaddr_t addr, usize size) {
    LOG_DEBUG(Kernel,
              "svcMapPhysicalMemory called (addr: 0x{:08x}, size: 0x{:08x})",
              addr, size);

    if (!is_aligned(size, hw::tegra_x1::cpu::GUEST_PAGE_SIZE))
        return MAKE_RESULT(Svc, 102); // Invalid address

    if (!is_aligned(size, hw::tegra_x1::cpu::GUEST_PAGE_SIZE))
        return MAKE_RESULT(Svc, 101); // Invalid size

    if (!(addr >= ALIAS_REGION_BASE &&
          addr < ALIAS_REGION_BASE + ALIAS_REGION_SIZE))
        return MAKE_RESULT(Svc, 110); // Invalid memory region

    auto mem = mmu->AllocateMemory(size);
    // TODO: keep track of the memory
    mmu->Map(addr, mem,
             {MemoryType::Alias, MemoryAttribute::None,
              MemoryPermission::ReadWrite});

    return RESULT_SUCCESS;
}

result_t Kernel::svcSetThreadActivity(handle_id_t thread_handle_id,
                                      ThreadActivity activity) {
    LOG_DEBUG(Kernel,
              "svcSetThreadActivity called (thread: 0x{:x}, activity: {})",
              thread_handle_id, activity);

    // TODO: implement
    LOG_FUNC_STUBBED(Kernel);

    return RESULT_SUCCESS;
}

result_t Kernel::svcGetThreadContext3(handle_id_t thread_handle_id,
                                      ThreadContext& out_thread_context) {
    LOG_DEBUG(Kernel, "svcSetThreadActivity called (thread: 0x{:x})",
              thread_handle_id);

    // TODO: implement
    LOG_FUNC_STUBBED(Kernel);

    // HACK
    out_thread_context = {};
    return RESULT_SUCCESS;
}

hw::tegra_x1::cpu::MemoryBase* Kernel::CreateTlsMemory(vaddr_t& base) {
    constexpr usize TLS_MEM_SIZE = 0x20000;

    auto mem = mmu->AllocateMemory(TLS_MEM_SIZE);
    base = tls_mem_base;
    mmu->Map(base, mem,
             {MemoryType::ThreadLocal, MemoryAttribute::None,
              MemoryPermission::ReadWrite});
    tls_mem_base += TLS_MEM_SIZE;

    return mem;
}

} // namespace hydra::horizon::kernel
