#include "core/horizon/kernel/kernel.hpp"

#include "core/debugger/debugger.hpp"
#include "core/horizon/kernel/code_memory.hpp"
#include "core/horizon/kernel/hipc/client_session.hpp"
#include "core/horizon/kernel/hipc/server_port.hpp"
#include "core/horizon/kernel/hipc/server_session.hpp"
#include "core/horizon/kernel/hipc/session.hpp"
#include "core/horizon/kernel/process.hpp"
#include "core/horizon/kernel/synchronization_object.hpp"
#include "core/horizon/kernel/thread.hpp"
#include "core/hw/tegra_x1/cpu/cpu.hpp"
#include "core/hw/tegra_x1/cpu/mmu.hpp"
#include "core/hw/tegra_x1/cpu/thread.hpp"

namespace hydra::horizon::kernel {

SINGLETON_DEFINE_GET_INSTANCE(Kernel, Kernel)

Kernel::Kernel() { SINGLETON_SET_INSTANCE(Kernel, Kernel); }

Kernel::~Kernel() { SINGLETON_UNSET_INSTANCE(); }

void Kernel::SupervisorCall(Process* crnt_process, IThread* crnt_thread,
                            hw::tegra_x1::cpu::IThread* guest_thread, u64 id) {
    result_t res;
    i32 tmp_i32;
    u32 tmp_u32;
    u64 tmp_u64;
    uptr tmp_uptr;
    switch (id) {
    case 0x1:
        res = SetHeapSize(crnt_process, guest_thread->GetRegX(1), tmp_uptr);
        guest_thread->SetRegW(0, res);
        guest_thread->SetRegX(1, tmp_uptr);
        break;
    case 0x2:
        res = SetMemoryPermission(
            guest_thread->GetRegX(0), guest_thread->GetRegX(1),
            static_cast<MemoryPermission>(guest_thread->GetRegX(2)));
        guest_thread->SetRegW(0, res);
        break;
    case 0x3:
        res = SetMemoryAttribute(
            guest_thread->GetRegX(0), guest_thread->GetRegX(1),
            guest_thread->GetRegX(2), guest_thread->GetRegX(3));
        guest_thread->SetRegW(0, res);
        break;
    case 0x4:
        res = MapMemory(crnt_process, guest_thread->GetRegX(0),
                        guest_thread->GetRegX(1), guest_thread->GetRegX(2));
        guest_thread->SetRegW(0, res);
        break;
    case 0x5:
        res = UnmapMemory(crnt_process, guest_thread->GetRegX(0),
                          guest_thread->GetRegX(1), guest_thread->GetRegX(2));
        guest_thread->SetRegW(0, res);
        break;
    case 0x6:
        res = QueryMemory(
            crnt_process, guest_thread->GetRegX(2),
            *reinterpret_cast<MemoryInfo*>(
                crnt_process->GetMmu()->UnmapAddr(guest_thread->GetRegX(0))),
            tmp_u32);
        guest_thread->SetRegW(0, res);
        guest_thread->SetRegX(1, tmp_u32);
        break;
    case 0x7:
        ExitProcess(crnt_process);
        break;
    case 0x8: {
        IThread* thread = nullptr;
        res =
            CreateThread(crnt_process, guest_thread->GetRegX(1),
                         guest_thread->GetRegX(2), guest_thread->GetRegX(3),
                         std::bit_cast<i32>(guest_thread->GetRegW(4)),
                         std::bit_cast<i32>(guest_thread->GetRegW(5)), thread);
        guest_thread->SetRegW(0, res);
        guest_thread->SetRegX(1, crnt_process->AddHandleNoRetain(thread));
        break;
    }
    case 0x9:
        res = StartThread(
            crnt_process->GetHandle<IThread>(guest_thread->GetRegW(0)));
        guest_thread->SetRegW(0, res);
        break;
    case 0xa:
        ExitThread(crnt_thread);
        break;
    case 0xb:
        SleepThread(std::bit_cast<i64>(guest_thread->GetRegX(0)));
        break;
    case 0xc:
        res = GetThreadPriority(
            crnt_process->GetHandle<IThread>(guest_thread->GetRegX(1)),
            tmp_i32);
        guest_thread->SetRegW(0, res);
        guest_thread->SetRegW(1, std::bit_cast<u32>(tmp_i32));
        break;
    case 0xd:
        res = SetThreadPriority(
            crnt_process->GetHandle<IThread>(guest_thread->GetRegW(0)),
            guest_thread->GetRegX(1));
        guest_thread->SetRegW(0, res);
        break;
    case 0xe:
        res = GetThreadCoreMask(
            crnt_process->GetHandle<IThread>(guest_thread->GetRegW(0)), tmp_i32,
            tmp_u64);
        guest_thread->SetRegW(0, res);
        guest_thread->SetRegW(1, tmp_i32);
        guest_thread->SetRegW(2, tmp_u64);
        break;
    case 0xf:
        res = SetThreadCoreMask(
            crnt_process->GetHandle<IThread>(guest_thread->GetRegW(0)),
            std::bit_cast<i32>(guest_thread->GetRegW(1)),
            guest_thread->GetRegX(2));
        guest_thread->SetRegW(0, res);
        break;
    case 0x10:
        GetCurrentProcessorNumber(tmp_u32);
        guest_thread->SetRegW(0, tmp_u32);
        break;
    case 0x11:
        res = SignalEvent(
            crnt_process->GetHandle<Event>(guest_thread->GetRegW(0)));
        guest_thread->SetRegW(0, res);
        break;
    case 0x12:
        res = ClearEvent(
            crnt_process->GetHandle<Event>(guest_thread->GetRegW(0)));
        guest_thread->SetRegW(0, res);
        break;
    case 0x13:
        res = MapSharedMemory(
            crnt_process,
            crnt_process->GetHandle<SharedMemory>(guest_thread->GetRegX(0)),
            guest_thread->GetRegX(1), guest_thread->GetRegX(2),
            static_cast<MemoryPermission>(guest_thread->GetRegX(3)));
        guest_thread->SetRegW(0, res);
        break;
    case 0x14:
        res = UnmapSharedMemory(
            crnt_process,
            crnt_process->GetHandle<SharedMemory>(guest_thread->GetRegX(0)),
            guest_thread->GetRegX(1), guest_thread->GetRegX(2));
        guest_thread->SetRegW(0, res);
        break;
    case 0x15: {
        TransferMemory* tmem;
        res = CreateTransferMemory(
            crnt_process, guest_thread->GetRegX(1), guest_thread->GetRegX(2),
            static_cast<MemoryPermission>(guest_thread->GetRegX(3)), tmem);
        guest_thread->SetRegW(0, res);
        guest_thread->SetRegX(1, crnt_process->AddHandleNoRetain(tmem));
        break;
    }
    case 0x16:
        res = CloseHandle(crnt_process, guest_thread->GetRegX(0));
        guest_thread->SetRegW(0, res);
        break;
    case 0x17:
        res = ResetSignal(crnt_process->GetHandle<SynchronizationObject>(
            guest_thread->GetRegX(0)));
        guest_thread->SetRegW(0, res);
        break;
    case 0x18: {
        const auto handle_ids = reinterpret_cast<handle_id_t*>(
            crnt_process->GetMmu()->UnmapAddr(guest_thread->GetRegX(1)));
        const auto num_handles = std::bit_cast<i64>(guest_thread->GetRegX(2));
        SynchronizationObject* sync_objs[num_handles];
        for (auto i = 0; i < num_handles; i++)
            sync_objs[i] =
                crnt_process->GetHandle<SynchronizationObject>(handle_ids[i]);

        res = WaitSynchronization(
            crnt_thread, std::span(sync_objs, num_handles),
            std::bit_cast<i64>(guest_thread->GetRegX(3)), tmp_i32);
        guest_thread->SetRegW(0, res);
        guest_thread->SetRegW(1, tmp_i32);
        break;
    }
    case 0x19:
        res = CancelSynchronization(
            crnt_process->GetHandle<IThread>(guest_thread->GetRegW(0)));
        guest_thread->SetRegW(0, res);
        break;
    case 0x1a:
        res = ArbitrateLock(crnt_process, guest_thread->GetRegX(0),
                            guest_thread->GetRegX(1), guest_thread->GetRegX(2));
        guest_thread->SetRegW(0, res);
        break;
    case 0x1b:
        res = ArbitrateUnlock(crnt_process, guest_thread->GetRegX(0));
        guest_thread->SetRegW(0, res);
        break;
    case 0x1c:
        res = WaitProcessWideKeyAtomic(
            crnt_process, guest_thread->GetRegX(0), guest_thread->GetRegX(1),
            guest_thread->GetRegX(2),
            std::bit_cast<i64>(guest_thread->GetRegX(3)));
        guest_thread->SetRegW(0, res);
        break;
    case 0x1d:
        res = SignalProcessWideKey(guest_thread->GetRegX(0),
                                   guest_thread->GetRegX(1));
        guest_thread->SetRegW(0, res);
        break;
    case 0x1e:
        GetSystemTick(tmp_u64);
        guest_thread->SetRegX(0, tmp_u64);
        break;
    case 0x1f: {
        hipc::ClientSession* client_session;
        res = ConnectToNamedPort(
            reinterpret_cast<const char*>(
                crnt_process->GetMmu()->UnmapAddr(guest_thread->GetRegX(1))),
            client_session);
        guest_thread->SetRegW(0, res);
        guest_thread->SetRegX(1,
                              crnt_process->AddHandleNoRetain(client_session));
        break;
    }
    case 0x21:
        res = SendSyncRequest(crnt_process, crnt_thread,
                              guest_thread->GetTlsMemory(),
                              crnt_process->GetHandle<hipc::ClientSession>(
                                  guest_thread->GetRegX(0)));
        guest_thread->SetRegW(0, res);
        break;
    case 0x25:
        res = GetThreadId(
            crnt_process->GetHandle<IThread>(guest_thread->GetRegX(1)),
            tmp_u64);
        guest_thread->SetRegW(0, res);
        guest_thread->SetRegX(1, tmp_u64);
        break;
    case 0x26:
        res = Break(BreakReason(guest_thread->GetRegX(0)),
                    crnt_process->GetMmu()->UnmapAddr(guest_thread->GetRegX(1)),
                    guest_thread->GetRegX(2));
        guest_thread->SetRegW(0, res);
        break;
    case 0x27:
        res = OutputDebugString(
            reinterpret_cast<const char*>(
                crnt_process->GetMmu()->UnmapAddr(guest_thread->GetRegX(0))),
            guest_thread->GetRegX(1));
        guest_thread->SetRegW(0, res);
        break;
    case 0x29:
        res = GetInfo(
            crnt_process, static_cast<InfoType>(guest_thread->GetRegX(1)),
            crnt_process->GetHandle<AutoObject>(guest_thread->GetRegX(2)),
            guest_thread->GetRegX(3), tmp_u64);
        guest_thread->SetRegW(0, res);
        guest_thread->SetRegX(1, tmp_u64);
        break;
    case 0x2c:
        res = MapPhysicalMemory(crnt_process, guest_thread->GetRegX(0),
                                guest_thread->GetRegX(1));
        guest_thread->SetRegW(0, res);
        break;
    case 0x32:
        res = SetThreadActivity(
            crnt_process->GetHandle<IThread>(guest_thread->GetRegW(0)),
            static_cast<ThreadActivity>(guest_thread->GetRegW(1)));
        guest_thread->SetRegW(0, res);
        break;
    case 0x33:
        res = GetThreadContext3(
            crnt_process->GetHandle<IThread>(guest_thread->GetRegW(1)),
            *reinterpret_cast<ThreadContext*>(
                crnt_process->GetMmu()->UnmapAddr(guest_thread->GetRegX(0))));
        guest_thread->SetRegW(0, res);
        break;
    case 0x34:
        res = WaitForAddress(
            crnt_process,
            crnt_process->GetMmu()->UnmapAddr(guest_thread->GetRegX(0)),
            static_cast<ArbitrationType>(guest_thread->GetRegW(1)),
            guest_thread->GetRegW(2), guest_thread->GetRegX(3));
        guest_thread->SetRegW(0, res);
        break;
    case 0x40: {
        hipc::ServerSession* server_session = nullptr;
        hipc::ClientSession* client_session = nullptr;
        res = CreateSession(guest_thread->GetRegW(2) != 0,
                            guest_thread->GetRegX(3), server_session,
                            client_session);
        guest_thread->SetRegW(0, res);
        guest_thread->SetRegW(1,
                              crnt_process->AddHandleNoRetain(server_session));
        guest_thread->SetRegW(2,
                              crnt_process->AddHandleNoRetain(client_session));
        break;
    }
    case 0x41: {
        hipc::ServerSession* server_session = nullptr;
        res = AcceptSession(
            crnt_process->GetHandle<hipc::ServerPort>(guest_thread->GetRegW(1)),
            server_session);
        guest_thread->SetRegW(0, res);
        guest_thread->SetRegW(1,
                              crnt_process->AddHandleNoRetain(server_session));
        break;
    }
    case 0x43: {
        const auto handle_ids = reinterpret_cast<handle_id_t*>(
            crnt_process->GetMmu()->UnmapAddr(guest_thread->GetRegX(1)));
        const auto num_handles = std::bit_cast<i64>(guest_thread->GetRegX(2));
        SynchronizationObject* sync_objs[num_handles];
        for (auto i = 0; i < num_handles; i++)
            sync_objs[i] =
                crnt_process->GetHandle<SynchronizationObject>(handle_ids[i]);

        res = ReplyAndReceive(crnt_thread, std::span(sync_objs, num_handles),
                              crnt_process->GetHandle<hipc::ServerSession>(
                                  guest_thread->GetRegX(3)),
                              std::bit_cast<i64>(guest_thread->GetRegX(4)),
                              tmp_i32);
        guest_thread->SetRegW(0, res);
        guest_thread->SetRegW(1, tmp_i32);
        break;
    }
    case 0x4b: {
        CodeMemory* code_mem = nullptr;
        res = CreateCodeMemory(guest_thread->GetRegX(1),
                               guest_thread->GetRegX(2), code_mem);
        guest_thread->SetRegW(0, res);
        guest_thread->SetRegW(1, crnt_process->AddHandleNoRetain(code_mem));
        break;
    }
    case 0x4c:
        res = ControlCodeMemory(
            crnt_process->GetHandle<CodeMemory>(guest_thread->GetRegW(0)),
            CodeMemoryOperation(guest_thread->GetRegW(1)),
            guest_thread->GetRegX(2), guest_thread->GetRegX(3),
            MemoryPermission(guest_thread->GetRegW(4)));
        guest_thread->SetRegW(0, res);
        break;
    case 0x73:
        res = SetProcessMemoryPermission(
            crnt_process->GetHandle<Process>(guest_thread->GetRegW(0)),
            guest_thread->GetRegX(1), guest_thread->GetRegX(2),
            MemoryPermission(guest_thread->GetRegW(3)));
        guest_thread->SetRegW(0, res);
        break;
    case 0x77:
        res = MapProcessCodeMemory(
            crnt_process->GetHandle<Process>(guest_thread->GetRegW(0)),
            guest_thread->GetRegX(1), guest_thread->GetRegX(2),
            guest_thread->GetRegX(3));
        guest_thread->SetRegW(0, res);
        break;
    case 0x78:
        res = UnmapProcessCodeMemory(
            crnt_process->GetHandle<Process>(guest_thread->GetRegW(0)),
            guest_thread->GetRegX(1), guest_thread->GetRegX(2),
            guest_thread->GetRegX(3));
        guest_thread->SetRegW(0, res);
        break;
    default:
        LOG_NOT_IMPLEMENTED(Kernel, "SVC 0x{:x}", id);
        res = MAKE_RESULT(Svc, Error::NotImplemented);
        guest_thread->SetRegW(0, res);
        break;
    }
}

result_t Kernel::SetHeapSize(Process* crnt_process, usize size,
                             uptr& out_base) {
    LOG_DEBUG(Kernel, "SetHeapSize called (size: 0x{:08x})", size);

    if ((size % HEAP_MEM_ALIGNMENT) != 0)
        return MAKE_RESULT(Svc, Error::InvalidSize); // TODO: correct?

    // TODO: handle this more cleanly?
    auto& heap_mem = crnt_process->GetHeapMemory();
    if (!heap_mem) {
        heap_mem = CPU_INSTANCE.AllocateMemory(size);
        crnt_process->GetMmu()->Map(HEAP_REGION_BASE, heap_mem,
                                    {MemoryType::Normal_1_0_0,
                                     MemoryAttribute::None,
                                     MemoryPermission::ReadWriteExecute});
    } else {
        crnt_process->GetMmu()->ResizeHeap(heap_mem, HEAP_REGION_BASE, size);
    }

    out_base = HEAP_REGION_BASE;

    return RESULT_SUCCESS;
}

result_t Kernel::SetMemoryPermission(uptr addr, usize size,
                                     MemoryPermission perm) {
    LOG_DEBUG(
        Kernel,
        "SetMemoryPermission called (addr: 0x{:08x}, size: 0x{:08x}, perm: "
        "{})",
        addr, size, perm);

    // TODO: implement
    LOG_FUNC_STUBBED(Kernel);

    return RESULT_SUCCESS;
}

result_t Kernel::SetMemoryAttribute(uptr addr, usize size, u32 mask,
                                    u32 value) {
    LOG_DEBUG(
        Kernel,
        "SetMemoryAttribute called (addr: 0x{:08x}, size: 0x{:08x}, mask: "
        "0x{:08x}, value: 0x{:08x})",
        addr, size, mask, value);

    // TODO: implement
    LOG_FUNC_STUBBED(Kernel);

    return RESULT_SUCCESS;
}

result_t Kernel::MapMemory(Process* crnt_process, uptr dst_addr, uptr src_addr,
                           usize size) {
    LOG_DEBUG(Kernel,
              "MapMemory called (dst_addr: 0x{:08x}, src_addr: 0x{:08x}, size: "
              "0x{:08x})",
              dst_addr, src_addr, size);

    crnt_process->GetMmu()->Map(dst_addr, src_addr, size);

    return RESULT_SUCCESS;
}

result_t Kernel::UnmapMemory(Process* crnt_process, uptr dst_addr,
                             uptr src_addr, usize size) {
    LOG_DEBUG(
        Kernel,
        "UnmapMemory called (dst_addr: 0x{:08x}, src_addr: 0x{:08x}, size: "
        "0x{:08x})",
        dst_addr, src_addr, size);

    // TODO: check if src_addr is the same as the one used in MapMemory

    crnt_process->GetMmu()->Unmap(dst_addr, size);

    return RESULT_SUCCESS;
}

result_t Kernel::QueryMemory(Process* crnt_process, uptr addr,
                             MemoryInfo& out_mem_info, u32& out_page_info) {
    LOG_DEBUG(Kernel, "QueryMemory called (addr: 0x{:08x})", addr);

    out_mem_info = crnt_process->GetMmu()->QueryMemory(addr);

    // TODO: what is this?
    out_page_info = 0;

    return RESULT_SUCCESS;
}

void Kernel::ExitProcess(Process* crnt_process) {
    LOG_DEBUG(Kernel, "ExitProcess called");

    crnt_process->Stop();
}

result_t Kernel::CreateThread(Process* crnt_process, vaddr_t entry_point,
                              vaddr_t args_addr, vaddr_t stack_top_addr,
                              i32 priority, i32 processor_id,
                              IThread*& out_thread) {
    LOG_DEBUG(Kernel,
              "CreateThread called (entry_point: 0x{:08x}, args_addr: "
              "0x{:08x}, stack_top_addr: 0x{:08x}, priority: {}, "
              "processor_id: {})",
              entry_point, args_addr, stack_top_addr, priority, processor_id);

    // Thread
    // TODO: processor ID
    auto thread = new GuestThread(crnt_process, stack_top_addr, priority);
    thread->SetEntryPoint(entry_point);
    thread->SetArg(0, args_addr);

    out_thread = thread;

    return RESULT_SUCCESS;
}

result_t Kernel::StartThread(IThread* thread) {
    LOG_DEBUG(Kernel, "StartThread called (thread: {})",
              thread->GetDebugName());

    thread->Start();

    return RESULT_SUCCESS;
}

void Kernel::ExitThread(IThread* crnt_thread) {
    LOG_DEBUG(Kernel, "ExitThread called");

    crnt_thread->Stop();
}

void Kernel::SleepThread(i64 nano) {
    LOG_DEBUG(Kernel, "SleepThread called (nano: {})", nano);

    if (nano == 0 || nano == -1 || nano == -2)
        std::this_thread::yield();
    else
        std::this_thread::sleep_for(std::chrono::nanoseconds(nano));
}

result_t Kernel::GetThreadPriority(IThread* thread, i32& out_priority) {
    LOG_DEBUG(Kernel, "GetThreadPriority called (thread: {})",
              thread->GetDebugName());

    // TODO: implement
    LOG_FUNC_STUBBED(Kernel);

    // HACK
    out_priority = 0x20; // 0x0 - 0x3f, lower is higher priority

    return RESULT_SUCCESS;
}

result_t Kernel::SetThreadPriority(IThread* thread, i32 priority) {
    LOG_DEBUG(Kernel, "SetThreadPriority called (thread: {}, priority: 0x{:x})",
              thread->GetDebugName(), priority);

    // TODO: implement
    LOG_FUNC_STUBBED(Kernel);

    return RESULT_SUCCESS;
}

result_t Kernel::GetThreadCoreMask(IThread* thread, i32& out_core_mask0,
                                   u64& out_core_mask1) {
    LOG_DEBUG(Kernel, "GetThreadCoreMask called (thread: {})",
              thread->GetDebugName());

    // TODO: implement
    LOG_FUNC_STUBBED(Kernel);

    // HACK
    out_core_mask0 = 0x1;
    out_core_mask1 = 0x2;

    return RESULT_SUCCESS;
}

result_t Kernel::SetThreadCoreMask(IThread* thread, i32 core_mask0,
                                   u64 core_mask1) {
    LOG_DEBUG(Kernel,
              "SetThreadCoreMask called (thread: {}, core_mask0: "
              "0x{:08x}, core_mask1: 0x{:08x})",
              thread->GetDebugName(), core_mask0, core_mask1);

    // TODO: implement
    LOG_FUNC_STUBBED(Kernel);

    return RESULT_SUCCESS;
}

void Kernel::GetCurrentProcessorNumber(u32& out_number) {
    LOG_DEBUG(Kernel, "GetCurrentProcessorNumber called");

    // TODO: implement
    LOG_FUNC_STUBBED(Kernel);

    // HACK
    out_number = 0;
}

result_t Kernel::SignalEvent(Event* event) {
    LOG_DEBUG(Kernel, "SignalEvent called (event: {})", event->GetDebugName());

    event->Signal();

    return RESULT_SUCCESS;
}

result_t Kernel::ClearEvent(Event* event) {
    LOG_DEBUG(Kernel, "ClearEvent called (event: {})", event->GetDebugName());

    if (!event->Clear())
        return MAKE_RESULT(Svc, Error::InvalidState); // TODO: correct?

    return RESULT_SUCCESS;
}

result_t Kernel::MapSharedMemory(Process* crnt_process, SharedMemory* shmem,
                                 uptr addr, usize size, MemoryPermission perm) {
    LOG_DEBUG(Kernel,
              "MapSharedMemory called (handle: {}, addr: 0x{:08x}, size: "
              "0x{:08x}, perm: {})",
              shmem->GetDebugName(), addr, size, perm);

    shmem->MapToRange(crnt_process->GetMmu(), range(addr, uptr(addr + size)),
                      perm);

    return RESULT_SUCCESS;
}

result_t Kernel::UnmapSharedMemory(Process* crnt_process, SharedMemory* shmem,
                                   uptr addr, usize size) {
    LOG_DEBUG(Kernel,
              "UnmapSharedMemory called (handle: {}, addr: 0x{:08x}, size: "
              "0x{:08x})",
              shmem->GetDebugName(), addr, size);

    // TODO
    LOG_FUNC_STUBBED(Kernel);

    return RESULT_SUCCESS;
}

result_t Kernel::CreateTransferMemory(Process* crnt_process, uptr addr,
                                      u64 size, MemoryPermission perm,
                                      TransferMemory*& out_tmem) {
    LOG_DEBUG(Kernel,
              "CreateTransferMemory called (address: 0x{:08x}, size: 0x{:08x}, "
              "perm: {})",
              addr, size, perm);

    out_tmem = new TransferMemory(addr, size, perm);

    return RESULT_SUCCESS;
}

result_t Kernel::CloseHandle(Process* crnt_process, handle_id_t handle_id) {
    auto obj = crnt_process->GetHandle<AutoObject>(handle_id);
    if (!obj) {
        LOG_WARN(Kernel, "CloseHandle called (INVALID_HANDLE)");
        return MAKE_RESULT(Svc, Error::InvalidHandle);
    }

    LOG_DEBUG(Kernel, "CloseHandle called (handle: {})", obj->GetDebugName());

    crnt_process->FreeHandle(handle_id);
    return RESULT_SUCCESS;
}

// TODO: can only be ReadableEvent or Process?
result_t Kernel::ResetSignal(SynchronizationObject* sync_obj) {
    LOG_DEBUG(Kernel, "ResetSignal called (sync_obj: {})",
              sync_obj->GetDebugName());

    if (!sync_obj->Clear())
        return MAKE_RESULT(Svc, Error::InvalidState);

    return RESULT_SUCCESS;
}

result_t
Kernel::WaitSynchronization(IThread* crnt_thread,
                            std::span<SynchronizationObject*> sync_objs,
                            i64 timeout, i32& out_signalled_index) {
    LOG_DEBUG(Kernel,
              "WaitSynchronization called (count: {}, timeout: "
              "{})",
              sync_objs.size(), timeout);

    if (sync_objs.empty()) {
        // TODO: allow waiting forever
        ASSERT_DEBUG(timeout != INFINITE_TIMEOUT, Kernel,
                     "Infinite timeout not implemented");
        std::this_thread::sleep_for(std::chrono::nanoseconds(timeout));
        return MAKE_RESULT(Svc, Error::TimedOut);
    } else {
        crnt_thread->Pause();

        for (u32 i = 0; i < sync_objs.size(); i++) {
            if (!sync_objs[i]) {
                LOG_WARN(Kernel, "Invalid sync object");
                return MAKE_RESULT(Svc, Error::InvalidHandle);
            }

            // LOG_DEBUG(Kernel, "Synchronizing with {}",
            //           sync_objs[i]->GetDebugName());

            sync_objs[i]->AddWaitingThread(crnt_thread);
        }

        const auto action = crnt_thread->ProcessMessages(timeout);
        switch (action.type) {
        case ThreadActionType::Stop:
            return RESULT_SUCCESS;
        case ThreadActionType::Resume: {
            switch (action.payload.resume.reason) {
            case ThreadResumeReason::Signalled: {
                const auto signalled_obj = action.payload.resume.signalled_obj;

                // Find the handle index
                out_signalled_index = -1;
                for (u32 i = 0; i < sync_objs.size(); i++) {
                    if (sync_objs[i] == signalled_obj) {
                        out_signalled_index = i;
                        break;
                    }
                }

                return RESULT_SUCCESS;
            }
            case ThreadResumeReason::TimedOut:
                return MAKE_RESULT(Svc, Error::TimedOut);
            case ThreadResumeReason::Cancelled:
                return MAKE_RESULT(Svc, Error::Cancelled);
            }
        }
        default:
            LOG_FATAL(Kernel, "Thread not resumed properly");
            unreachable();
        }
    }
}

result_t Kernel::CancelSynchronization(IThread* thread) {
    LOG_DEBUG(Kernel, "CancelSynchronization called (thread: {})",
              thread->GetDebugName());

    thread->Resume();

    return RESULT_SUCCESS;
}

result_t Kernel::ArbitrateLock(Process* crnt_process, u32 wait_tag,
                               uptr mutex_addr, u32 self_tag) {
    LOG_DEBUG(Kernel,
              "ArbitrateLock called (wait: 0x{:08x}, mutex: 0x{:08x}, self: "
              "0x{:08x})",
              wait_tag, mutex_addr, self_tag);

    sync_mutex.lock();
    auto& mutex = mutex_map[mutex_addr];
    sync_mutex.unlock();
    mutex.Lock(
        *reinterpret_cast<u32*>(crnt_process->GetMmu()->UnmapAddr(mutex_addr)),
        self_tag);

    return RESULT_SUCCESS;
}

result_t Kernel::ArbitrateUnlock(Process* crnt_process, uptr mutex_addr) {
    LOG_DEBUG(Kernel, "ArbitrateUnlock called (mutex: 0x{:08x})", mutex_addr);

    sync_mutex.lock();
    auto& mutex = mutex_map[mutex_addr];
    sync_mutex.unlock();
    mutex.Unlock(
        *reinterpret_cast<u32*>(crnt_process->GetMmu()->UnmapAddr(mutex_addr)));

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

result_t Kernel::WaitProcessWideKeyAtomic(Process* crnt_process,
                                          uptr mutex_addr, uptr var_addr,
                                          u32 self_tag, i64 timeout) {
    LOG_DEBUG(
        Kernel,
        "WaitProcessWideKeyAtomic called (mutex: 0x{:08x}, var: 0x{:08x}, "
        "self: 0x{:08x}, timeout: {})",
        mutex_addr, var_addr, self_tag, timeout);

    sync_mutex.lock();
    auto& mutex = mutex_map[mutex_addr];
    auto& cond_var = cond_var_map[var_addr];
    sync_mutex.unlock();

    // TODO: correct?
    auto& value =
        *reinterpret_cast<u32*>(crnt_process->GetMmu()->UnmapAddr(mutex_addr));
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

result_t Kernel::SignalProcessWideKey(uptr addr, i32 count) {
    LOG_DEBUG(Kernel, "SignalProcessWideKey called (addr: 0x{:08x}, count: {})",
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

void Kernel::GetSystemTick(u64& out_tick) {
    LOG_DEBUG(Kernel, "GetSystemTick called");

    out_tick = get_absolute_time();
}

result_t Kernel::ConnectToNamedPort(const std::string_view name,
                                    hipc::ClientSession*& out_client_session) {
    LOG_DEBUG(Kernel, "ConnectToNamedPort called (name: {})", name);

    auto port = service_manager.GetPort(std::string(name));
    if (!port) {
        LOG_ERROR(Kernel, "Failed to connect to port \"{}\"", name);
        return MAKE_RESULT(Svc, Error::NotFound);
    }

    out_client_session = port->Connect();

    return RESULT_SUCCESS;
}

result_t Kernel::SendSyncRequest(Process* crnt_process, IThread* crnt_thread,
                                 hw::tegra_x1::cpu::IMemory* tls_mem,
                                 hipc::ClientSession* client_session) {
    LOG_DEBUG(Kernel, "SendSyncRequest called (session: {})",
              client_session->GetDebugName());

    // Pause the thread
    crnt_thread->Pause();

    // Send request
    client_session->GetParent()->GetServerSide()->EnqueueRequest(
        crnt_process, crnt_thread, crnt_thread->GetTlsPtr());

    // Wait for response
    crnt_thread->ProcessMessages();

    return RESULT_SUCCESS;
}

result_t Kernel::GetThreadId(IThread* thread, u64& out_thread_id) {
    LOG_DEBUG(Kernel, "GetThreadId called (thread: {})",
              thread->GetDebugName());

    // TODO: implement
    LOG_FUNC_STUBBED(Services);

    // HACK
    out_thread_id = u64(thread);

    return RESULT_SUCCESS;
}

result_t Kernel::Break(BreakReason reason, uptr buffer_ptr, usize buffer_size) {
    LOG_DEBUG(Kernel,
              "Break called (reason: {}, buffer ptr: 0x{:08x}, buffer "
              "size: 0x{:08x})",
              reason.type, buffer_ptr, buffer_size);

    // TODO: this should be sent to the debugger instead of being logged
    if (buffer_ptr) {
        if (buffer_size == sizeof(u32)) {
            const u32 result = *reinterpret_cast<u32*>(buffer_ptr);
            const auto module = GET_RESULT_MODULE(result);
            const auto description = GET_RESULT_DESCRIPTION(result);
            LOG_INFO(Kernel, "Module: {}, description: {}", module,
                     description);
        } else {
            constexpr u32 MAX_DATA_COUNT = 16;
            const u32 data_count = buffer_size / sizeof(u32);
            for (u32 i = 0; i < std::min(data_count, MAX_DATA_COUNT); i++) {
                const u32 value = reinterpret_cast<u32*>(buffer_ptr)[i];
                LOG_INFO(Kernel, "0x{:08x}", value);
            }
            if (data_count > MAX_DATA_COUNT)
                LOG_INFO(Kernel, "...");
        }
    }

    if (!reason.notification_only)
        DEBUGGER_INSTANCE.BreakOnThisThread("Break");

    return RESULT_SUCCESS;
}

result_t Kernel::OutputDebugString(const std::string_view str, usize len) {
    LOG_DEBUG(Kernel, "OutputDebugString called");
    if (len != 0) {
        // TODO: handle differently
        LOG_INFO(Kernel, "{}", str);
    }

    return RESULT_SUCCESS;
}

result_t Kernel::GetInfo(Process* crnt_process, InfoType info_type,
                         AutoObject* obj, u64 info_sub_type, u64& out_info) {
    LOG_DEBUG(Kernel, "GetInfo called (type: {}, object: {}, subtype: {})",
              info_type, (obj != nullptr ? obj->GetDebugName() : "null"),
              info_sub_type);

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
    case InfoType::TotalSystemResourceSize: {
        out_info = crnt_process->GetSystemResourceSize();
        return RESULT_SUCCESS;
    }
    case InfoType::UsedSystemResourceSize:
        LOG_NOT_IMPLEMENTED(Kernel, "UsedSystemResourceSize");
        // HACK
        out_info = 64 * 1024;
        return RESULT_SUCCESS;
    case InfoType::UserExceptionContextAddress:
        LOG_NOT_IMPLEMENTED(Kernel, "UserExceptionContextAddress");
        // HACK
        out_info = 0;
        return RESULT_SUCCESS;
    case InfoType::TotalNonSystemMemorySize:
        LOG_NOT_IMPLEMENTED(Kernel, "TotalNonSystemMemorySize");
        // HACK
        out_info = 2u * 1024u * 1024u * 1024u;
        return RESULT_SUCCESS;
    case InfoType::UsedNonSystemMemorySize:
        LOG_NOT_IMPLEMENTED(Kernel, "UsedNonSystemMemorySize");
        // HACK
        out_info = 1;
        return RESULT_SUCCESS;
    case InfoType::IsApplication:
        // TODO: don't always return true
        out_info = true;
        return RESULT_SUCCESS;
    case InfoType::AliasRegionExtraSize:
        LOG_NOT_IMPLEMENTED(Kernel, "AliasRegionExtraSize");
        // HACK
        out_info = 0;
        return RESULT_SUCCESS;
    default:
        LOG_WARN(Kernel, "Unknown info type {}", info_type);
        return MAKE_RESULT(Svc, 0x78);
    }
}

result_t Kernel::MapPhysicalMemory(Process* crnt_process, vaddr_t addr,
                                   usize size) {
    LOG_DEBUG(Kernel,
              "MapPhysicalMemory called (addr: 0x{:08x}, size: 0x{:08x})", addr,
              size);

    if (!is_aligned(size, hw::tegra_x1::cpu::GUEST_PAGE_SIZE))
        return MAKE_RESULT(Svc, 102); // Invalid address

    if (!is_aligned(size, hw::tegra_x1::cpu::GUEST_PAGE_SIZE))
        return MAKE_RESULT(Svc, 101); // Invalid size

    if (!(addr >= ALIAS_REGION_BASE &&
          addr < ALIAS_REGION_BASE + ALIAS_REGION_SIZE))
        return MAKE_RESULT(Svc, 110); // Invalid memory region

    auto mem = CPU_INSTANCE.AllocateMemory(size);
    // TODO: keep track of the memory
    crnt_process->GetMmu()->Map(addr, mem,
                                {MemoryType::Alias, MemoryAttribute::None,
                                 MemoryPermission::ReadWrite});

    return RESULT_SUCCESS;
}

result_t Kernel::SetThreadActivity(IThread* thread, ThreadActivity activity) {
    LOG_DEBUG(Kernel, "SetThreadActivity called (thread: {}, activity: {})",
              thread->GetDebugName(), activity);

    // TODO: implement
    LOG_FUNC_STUBBED(Kernel);

    return RESULT_SUCCESS;
}

result_t Kernel::GetThreadContext3(IThread* thread,
                                   ThreadContext& out_thread_context) {
    LOG_DEBUG(Kernel, "SetThreadActivity called (thread: {})",
              thread->GetDebugName());

    // TODO: implement
    LOG_FUNC_STUBBED(Kernel);

    // HACK
    out_thread_context = {};
    return RESULT_SUCCESS;
}

result_t Kernel::WaitForAddress(Process* crnt_process, vaddr_t addr,
                                ArbitrationType arbitration_type, u32 value,
                                u64 timeout) {
    LOG_DEBUG(Kernel,
              "WaitForAddress called (addr: 0x{:08x}, type: {}, value: "
              "0x{:x}, timeout: 0x{:08x})",
              addr, arbitration_type, value, timeout);

    sync_mutex.lock();
    auto& mutex = mutex_map[addr];
    auto& cond_var = cond_var_map[addr];
    sync_mutex.unlock();

    {
        std::unique_lock lock(mutex.GetNativeHandle());

        const auto current_value = crnt_process->GetMmu()->Load<u32>(addr);
        bool wait{false};
        switch (arbitration_type) {
        case ArbitrationType::WaitIfLessThan:
            wait = (current_value < value);
            break;
        case ArbitrationType::DecrementAndWaitIfLessThan:
            wait = (current_value < value);
            crnt_process->GetMmu()->Store<u32>(
                addr, current_value - 1); // TODO: decrement after?
            break;
        case ArbitrationType::WaitIfEqual:
            wait = (current_value == value);
            break;
        }

        if (wait) {
            cond_var.wait_for(
                lock, std::chrono::nanoseconds(timeout), [=, this]() {
                    return crnt_process->GetMmu()->Load<u32>(addr) == value;
                }); // TODO: is the timeout correct?
        }
    }

    return RESULT_SUCCESS;
}

result_t Kernel::CreateSession(bool is_light, u64 name,
                               hipc::ServerSession*& out_server_session,
                               hipc::ClientSession*& out_client_session) {
    LOG_DEBUG(Kernel, "CreateSession called (is_light: {}, name: 0x{:08x})",
              is_light, name);

    // TODO: what are light sessions?
    // TODO: what's the purpose of the name?
    out_server_session = new hipc::ServerSession();
    out_client_session = new hipc::ClientSession();
    // TODO: is it fine to just instantiate it like this?
    new hipc::Session(out_server_session, out_client_session);

    return RESULT_SUCCESS;
}

result_t Kernel::AcceptSession(hipc::ServerPort* server_port,
                               hipc::ServerSession*& out_server_session) {
    LOG_DEBUG(Kernel, "AcceptSession called (port: {})",
              server_port->GetDebugName());

    out_server_session = server_port->AcceptSession();

    return RESULT_SUCCESS;
}

result_t Kernel::ReplyAndReceive(IThread* crnt_thread,
                                 std::span<SynchronizationObject*> sync_objs,
                                 hipc::ServerSession* reply_target_session,
                                 i64 timeout, i32& out_signalled_index) {
    LOG_DEBUG(Kernel, "ReplyAndReceive called (count: {}, timeout: {})",
              sync_objs.size(), timeout);

    if (reply_target_session) {
        // Reply
        reply_target_session->Reply(crnt_thread->GetTlsPtr());
    }

    // Wait
    const auto res = WaitSynchronization(crnt_thread, sync_objs, timeout,
                                         out_signalled_index);
    if (res != RESULT_SUCCESS)
        return res;

    // Receive
    auto server_session =
        dynamic_cast<hipc::ServerSession*>(sync_objs[out_signalled_index]);
    if (server_session)
        server_session->Receive(crnt_thread);

    return RESULT_SUCCESS;
}

result_t Kernel::CreateCodeMemory(vaddr_t addr, u64 size,
                                  CodeMemory*& out_code_memory) {
    LOG_DEBUG(Kernel, "CreateCodeMemory called (addr: 0x{:08x}, size: {})",
              addr, size);

    out_code_memory = new CodeMemory(addr, size);

    return RESULT_SUCCESS;
}

result_t Kernel::ControlCodeMemory(CodeMemory* code_memory,
                                   CodeMemoryOperation op, vaddr_t addr,
                                   u64 size, MemoryPermission perm) {
    LOG_DEBUG(Kernel,
              "ControlCodeMemory called (code memory: {}, op: {}, addr: "
              "0x{:08x}, size: {}, perm: {})",
              code_memory->GetDebugName(), op, addr, size, perm);

    // TODO: implement
    LOG_FUNC_NOT_IMPLEMENTED(Kernel);

    return RESULT_SUCCESS;
}

result_t Kernel::SetProcessMemoryPermission(Process* process, vaddr_t addr,
                                            u64 size, MemoryPermission perm) {
    LOG_DEBUG(Kernel,
              "SetProcessMemoryPermission called (process: {}, addr: 0x{:08x}, "
              "size: {}, perm: {})",
              process->GetDebugName(), addr, size, perm);

    // TODO: implement
    LOG_FUNC_NOT_IMPLEMENTED(Kernel);

    return RESULT_SUCCESS;
}

result_t Kernel::MapProcessCodeMemory(Process* process, vaddr_t dst_addr,
                                      vaddr_t src_addr, u64 size) {
    LOG_DEBUG(Kernel,
              "MapProcessCodeMemory called (process: {}, dst_addr: 0x{:08x}, "
              "src_addr: 0x{:08x}, size: {})",
              process->GetDebugName(), dst_addr, src_addr, size);

    process->GetMmu()->Map(dst_addr, src_addr, size);

    return RESULT_SUCCESS;
}

result_t Kernel::UnmapProcessCodeMemory(Process* process, vaddr_t dst_addr,
                                        vaddr_t src_addr, u64 size) {
    LOG_DEBUG(Kernel,
              "UnmapProcessCodeMemory called (process: {}, dst_addr: 0x{:08x}, "
              "src_addr: 0x{:08x}, size: {})",
              process->GetDebugName(), dst_addr, src_addr, size);

    // TODO: what's the purpose of src_addr?
    process->GetMmu()->Unmap(dst_addr, size);

    return RESULT_SUCCESS;
}

} // namespace hydra::horizon::kernel
