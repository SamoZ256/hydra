#include "core/horizon/kernel/kernel.hpp"

#include "core/debugger/debugger_manager.hpp"
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
    auto& state = guest_thread->GetState();

    i32 tmp_i32;
    u32 tmp_u32;
    u64 tmp_u64;
    uptr tmp_uptr;
    switch (id) {
    case 0x1:
        state.r[0] = SetHeapSize(crnt_process, state.r[1], tmp_uptr);
        state.r[1] = tmp_uptr;
        break;
    case 0x2:
        state.r[0] = SetMemoryPermission(
            state.r[0], state.r[1], static_cast<MemoryPermission>(state.r[2]));
        break;
    case 0x3:
        state.r[0] = SetMemoryAttribute(state.r[0], state.r[1],
                                        static_cast<u32>(state.r[2]),
                                        static_cast<u32>(state.r[3]));
        break;
    case 0x4:
        state.r[0] =
            MapMemory(crnt_process, state.r[0], state.r[1], state.r[2]);
        break;
    case 0x5:
        state.r[0] =
            UnmapMemory(crnt_process, state.r[0], state.r[1], state.r[2]);
        break;
    case 0x6:
        state.r[0] =
            QueryMemory(crnt_process, state.r[2],
                        *reinterpret_cast<MemoryInfo*>(
                            crnt_process->GetMmu()->UnmapAddr(state.r[0])),
                        tmp_u32);
        state.r[1] = tmp_u32;
        break;
    case 0x7:
        ExitProcess(crnt_process);
        break;
    case 0x8: {
        IThread* thread = nullptr;
        state.r[0] =
            CreateThread(crnt_process, state.r[1], state.r[2], state.r[3],
                         std::bit_cast<i32>(u32(state.r[4])),
                         std::bit_cast<i32>(u32(state.r[5])), thread);
        state.r[1] = crnt_process->AddHandleNoRetain(thread);
        break;
    }
    case 0x9:
        state.r[0] = StartThread(crnt_process->GetHandle<IThread>(
            static_cast<handle_id_t>(state.r[0])));
        break;
    case 0xa:
        ExitThread(crnt_thread);
        break;
    case 0xb:
        SleepThread(std::bit_cast<i64>(state.r[0]));
        break;
    case 0xc:
        state.r[0] =
            GetThreadPriority(crnt_process->GetHandle<IThread>(
                                  static_cast<handle_id_t>(state.r[1])),
                              tmp_i32);
        state.r[1] = std::bit_cast<u32>(tmp_i32);
        break;
    case 0xd:
        state.r[0] =
            SetThreadPriority(crnt_process->GetHandle<IThread>(
                                  static_cast<handle_id_t>(state.r[0])),
                              std::bit_cast<i32>(static_cast<u32>(state.r[1])));
        break;
    case 0xe:
        state.r[0] =
            GetThreadCoreMask(crnt_process->GetHandle<IThread>(
                                  static_cast<handle_id_t>(state.r[0])),
                              tmp_i32, tmp_u64);
        state.r[1] = std::bit_cast<u32>(tmp_i32);
        state.r[2] = tmp_u64;
        break;
    case 0xf:
        state.r[0] = SetThreadCoreMask(
            crnt_process->GetHandle<IThread>(
                static_cast<handle_id_t>(state.r[0])),
            std::bit_cast<i32>(static_cast<u32>(state.r[1])), state.r[2]);
        break;
    case 0x10:
        GetCurrentProcessorNumber(tmp_u32);
        state.r[0] = tmp_u32;
        break;
    case 0x11:
        state.r[0] = SignalEvent(crnt_process->GetHandle<Event>(
            static_cast<handle_id_t>(state.r[0])));
        break;
    case 0x12:
        state.r[0] = ClearEvent(crnt_process->GetHandle<Event>(
            static_cast<handle_id_t>(state.r[0])));
        break;
    case 0x13:
        state.r[0] = MapSharedMemory(crnt_process,
                                     crnt_process->GetHandle<SharedMemory>(
                                         static_cast<handle_id_t>(state.r[0])),
                                     state.r[1], state.r[2],
                                     static_cast<MemoryPermission>(state.r[3]));
        break;
    case 0x14:
        state.r[0] =
            UnmapSharedMemory(crnt_process,
                              crnt_process->GetHandle<SharedMemory>(
                                  static_cast<handle_id_t>(state.r[0])),
                              state.r[1], state.r[2]);
        break;
    case 0x15: {
        TransferMemory* tmem = nullptr;
        state.r[0] = CreateTransferMemory(
            crnt_process, state.r[1], state.r[2],
            static_cast<MemoryPermission>(state.r[3]), tmem);
        state.r[1] = crnt_process->AddHandleNoRetain(tmem);
        break;
    }
    case 0x16:
        state.r[0] =
            CloseHandle(crnt_process, static_cast<handle_id_t>(state.r[0]));
        break;
    case 0x17:
        state.r[0] = ResetSignal(crnt_process->GetHandle<SynchronizationObject>(
            static_cast<handle_id_t>(state.r[0])));
        break;
    case 0x18: {
        const auto handle_ids = reinterpret_cast<handle_id_t*>(
            crnt_process->GetMmu()->UnmapAddr(state.r[1]));
        const auto num_handles = std::bit_cast<i64>(state.r[2]);
        SynchronizationObject* sync_objs[num_handles];
        for (auto i = 0; i < num_handles; i++)
            sync_objs[i] =
                crnt_process->GetHandle<SynchronizationObject>(handle_ids[i]);

        state.r[0] =
            WaitSynchronization(crnt_thread, std::span(sync_objs, num_handles),
                                std::bit_cast<i64>(state.r[3]), tmp_i32);
        state.r[1] = std::bit_cast<u32>(tmp_i32);
        break;
    }
    case 0x19:
        state.r[0] = CancelSynchronization(crnt_process->GetHandle<IThread>(
            static_cast<handle_id_t>(state.r[0])));
        break;
    case 0x1a:
        state.r[0] =
            ArbitrateLock(crnt_thread,
                          crnt_process->GetHandle<IThread>(
                              static_cast<handle_id_t>(state.r[0])),
                          crnt_process->GetMmu()->UnmapAddr(state.r[1]),
                          static_cast<handle_id_t>(state.r[2]),
                          static_cast<handle_id_t>(state.r[0]));
        break;
    case 0x1b:
        state.r[0] = ArbitrateUnlock(
            crnt_thread, crnt_process->GetMmu()->UnmapAddr(state.r[0]));
        break;
    case 0x1c:
        state.r[0] = WaitProcessWideKeyAtomic(
            crnt_process, crnt_thread,
            crnt_process->GetMmu()->UnmapAddr(state.r[0]),
            crnt_process->GetMmu()->UnmapAddr(state.r[1]),
            static_cast<handle_id_t>(state.r[2]),
            std::bit_cast<i64>(state.r[3]));
        break;
    case 0x1d:
        state.r[0] = SignalProcessWideKey(
            crnt_process, crnt_process->GetMmu()->UnmapAddr(state.r[0]),
            std::bit_cast<i32>(static_cast<u32>(state.r[1])));
        break;
    case 0x1e:
        GetSystemTick(tmp_u64);
        state.r[0] = tmp_u64;
        break;
    case 0x1f: {
        hipc::ClientSession* client_session = nullptr;
        state.r[0] = ConnectToNamedPort(
            reinterpret_cast<const char*>(
                crnt_process->GetMmu()->UnmapAddr(state.r[1])),
            client_session);
        state.r[1] = crnt_process->AddHandleNoRetain(client_session);
        break;
    }
    case 0x21:
        state.r[0] = SendSyncRequest(
            crnt_process, crnt_thread, guest_thread->GetTlsMemory(),
            crnt_process->GetHandle<hipc::ClientSession>(
                static_cast<handle_id_t>(state.r[0])));
        break;
    case 0x25:
        state.r[0] = GetThreadId(crnt_process->GetHandle<IThread>(
                                     static_cast<handle_id_t>(state.r[1])),
                                 tmp_u64);
        state.r[1] = tmp_u64;
        break;
    case 0x26: {
        const vaddr_t addr = state.r[1];
        state.r[0] =
            Break(BreakReason(state.r[0]),
                  (addr != 0x0 ? crnt_process->GetMmu()->UnmapAddr(addr) : 0x0),
                  state.r[2]);
        break;
    }
    case 0x27:
        state.r[0] = OutputDebugString(
            reinterpret_cast<const char*>(
                crnt_process->GetMmu()->UnmapAddr(state.r[0])),
            state.r[1]);
        break;
    case 0x29:
        state.r[0] = GetInfo(crnt_process, static_cast<InfoType>(state.r[1]),
                             crnt_process->GetHandle<AutoObject>(
                                 static_cast<handle_id_t>(state.r[2])),
                             state.r[3], tmp_u64);
        state.r[1] = tmp_u64;
        break;
    case 0x2c:
        state.r[0] = MapPhysicalMemory(crnt_process, state.r[0], state.r[1]);
        break;
    case 0x32:
        state.r[0] =
            SetThreadActivity(crnt_process->GetHandle<IThread>(
                                  static_cast<handle_id_t>(state.r[0])),
                              static_cast<ThreadActivity>(state.r[1]));
        break;
    case 0x33:
        state.r[0] = GetThreadContext3(
            crnt_process->GetHandle<IThread>(
                static_cast<handle_id_t>(state.r[1])),
            *reinterpret_cast<ThreadContext*>(
                crnt_process->GetMmu()->UnmapAddr(state.r[0])));
        break;
    case 0x34:
        state.r[0] = WaitForAddress(
            crnt_thread, crnt_process->GetMmu()->UnmapAddr(state.r[0]),
            static_cast<ArbitrationType>(state.r[1]),
            static_cast<u32>(state.r[2]), state.r[3]);
        break;
    case 0x40: {
        hipc::ServerSession* server_session = nullptr;
        hipc::ClientSession* client_session = nullptr;
        state.r[0] = CreateSession(state.r[2] != 0, state.r[3], server_session,
                                   client_session);
        state.r[1] = crnt_process->AddHandleNoRetain(server_session);
        state.r[2] = crnt_process->AddHandleNoRetain(client_session);
        break;
    }
    case 0x41: {
        hipc::ServerSession* server_session = nullptr;
        state.r[0] = AcceptSession(crnt_process->GetHandle<hipc::ServerPort>(
                                       static_cast<handle_id_t>(state.r[1])),
                                   server_session);
        state.r[1] = crnt_process->AddHandleNoRetain(server_session);
        break;
    }
    case 0x43: {
        const auto handle_ids = reinterpret_cast<handle_id_t*>(
            crnt_process->GetMmu()->UnmapAddr(state.r[1]));
        const auto num_handles = std::bit_cast<i64>(state.r[2]);
        SynchronizationObject* sync_objs[num_handles];
        for (auto i = 0; i < num_handles; i++)
            sync_objs[i] =
                crnt_process->GetHandle<SynchronizationObject>(handle_ids[i]);

        state.r[0] =
            ReplyAndReceive(crnt_thread, std::span(sync_objs, num_handles),
                            crnt_process->GetHandle<hipc::ServerSession>(
                                static_cast<handle_id_t>(state.r[3])),
                            std::bit_cast<i64>(state.r[4]), tmp_i32);
        state.r[1] = std::bit_cast<u32>(tmp_i32);
        break;
    }
    case 0x4b: {
        CodeMemory* code_mem = nullptr;
        state.r[0] = CreateCodeMemory(state.r[1], state.r[2], code_mem);
        state.r[1] = crnt_process->AddHandleNoRetain(code_mem);
        break;
    }
    case 0x4c:
        state.r[0] =
            ControlCodeMemory(crnt_process->GetHandle<CodeMemory>(
                                  static_cast<handle_id_t>(state.r[0])),
                              CodeMemoryOperation(state.r[1]), state.r[2],
                              state.r[3], MemoryPermission(state.r[4]));
        break;
    case 0x65:
        state.r[0] =
            GetProcessList(reinterpret_cast<u64*>(
                               crnt_process->GetMmu()->UnmapAddr(state.r[1])),
                           static_cast<u32>(state.r[2]), tmp_u32);
        state.r[1] = tmp_u32;
        break;
    case 0x73:
        state.r[0] = SetProcessMemoryPermission(
            crnt_process->GetHandle<Process>(
                static_cast<handle_id_t>(state.r[0])),
            state.r[1], state.r[2], MemoryPermission(state.r[3]));
        break;
    case 0x74:
        state.r[0] = MapProcessMemory(crnt_process, state.r[0],
                                      crnt_process->GetHandle<Process>(
                                          static_cast<handle_id_t>(state.r[1])),
                                      state.r[2], state.r[3]);
        break;
    case 0x77:
        state.r[0] =
            MapProcessCodeMemory(crnt_process->GetHandle<Process>(
                                     static_cast<handle_id_t>(state.r[0])),
                                 state.r[1], state.r[2], state.r[3]);
        break;
    case 0x78:
        state.r[0] =
            UnmapProcessCodeMemory(crnt_process->GetHandle<Process>(
                                       static_cast<handle_id_t>(state.r[0])),
                                   state.r[1], state.r[2], state.r[3]);
        break;
    default:
        LOG_NOT_IMPLEMENTED(Kernel, "SVC 0x{:x}", id);
        state.r[0] = MAKE_RESULT(Svc, Error::NotImplemented);
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
        crnt_process->GetMmu()->Map(HEAP_REGION.begin, heap_mem,
                                    {MemoryType::Normal_1_0_0,
                                     MemoryAttribute::None,
                                     MemoryPermission::ReadWriteExecute});
    } else {
        crnt_process->GetMmu()->ResizeHeap(heap_mem, HEAP_REGION.begin, size);
    }

    out_base = HEAP_REGION.begin;

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
    if (!sync_obj) {
        LOG_WARN(Kernel, "ResetSignal called (INVALID_HANDLE)");
        // HACK
        return RESULT_SUCCESS; // MAKE_RESULT(Svc, Error::InvalidHandle);
    }

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

    for (u32 i = 0; i < sync_objs.size(); i++) {
        if (!sync_objs[i]) {
            LOG_WARN(Kernel, "Invalid sync object");
            // HACK: Celeste gets stuck in an infinite WaitSynchronization
            // loop if an error is returned
            return RESULT_SUCCESS; // MAKE_RESULT(Svc,
                                   // Error::InvalidHandle);
        }
    }

    crnt_thread->Pause();

    // Add waiting thread
    for (auto sync_obj : sync_objs) {
        // LOG_DEBUG(Kernel, "Synchronizing with {}",
        //           sync_obj->GetDebugName());

        sync_obj->AddWaitingThread(crnt_thread);
    }

    SynchronizationObject* signalled_obj = nullptr;
    result_t res = RESULT_SUCCESS;
    if (crnt_thread->ProcessMessages(timeout)) {
        if (crnt_thread->GetState() == ThreadState::Stopping) {
            // TODO: is this necessary?
            res = MAKE_RESULT(Svc, Error::Cancelled);
        } else {
            if (!crnt_thread->ConsumeSignalledObject(signalled_obj))
                res = MAKE_RESULT(Svc, Error::Cancelled);
        }
    } else {
        res = MAKE_RESULT(Svc, Error::TimedOut);
    }

    // Remove the thread from the waiting list
    for (auto sync_obj : sync_objs) {
        if (sync_obj != signalled_obj)
            sync_obj->RemoveWaitingThread(crnt_thread);
    }

    // Find the handle index
    out_signalled_index = -1;
    if (signalled_obj) {
        for (u32 i = 0; i < sync_objs.size(); i++) {
            if (sync_objs[i] == signalled_obj) {
                out_signalled_index = i;
                break;
            }
        }
    }

    return res;
}

result_t Kernel::CancelSynchronization(IThread* thread) {
    LOG_DEBUG(Kernel, "CancelSynchronization called (thread: {})",
              thread->GetDebugName());

    thread->CancelSync();

    return RESULT_SUCCESS;
}

result_t Kernel::ArbitrateLock(IThread* crnt_thread, IThread* owner_thread,
                               uptr mutex_addr, handle_id_t self_handle,
                               handle_id_t owner_handle) {
    LOG_DEBUG(Kernel,
              "ArbitrateLock called (owner: {}, mutex: 0x{:08x}, self: "
              "0x{:x})",
              owner_thread->GetDebugName(), mutex_addr, self_handle);

    crnt_thread->self_handle_for_mutex = self_handle;
    owner_thread->self_handle_for_mutex = owner_handle;

    {
        CriticalSectionLock cs_lock;

        if (atomic_load(reinterpret_cast<u32*>(mutex_addr)) !=
            (owner_thread->self_handle_for_mutex | MUTEX_WAIT_MASK))
            return RESULT_SUCCESS;

        crnt_thread->mutex_wait_addr = mutex_addr;

        crnt_thread->Pause();
        owner_thread->AddMutexWaiter(crnt_thread);
    }

    ASSERT_DEBUG(crnt_thread->ProcessMessages(), Kernel,
                 "ArbitrateLock timed out");

    result_t res = RESULT_SUCCESS;
    if (crnt_thread->GetState() == ThreadState::Stopping) {
        // TODO: is this necessary?
        res = MAKE_RESULT(Svc, Error::Cancelled);
    } else {
        if (!crnt_thread->WasSignalled())
            res = MAKE_RESULT(Svc, Error::Cancelled);
    }

    return res;
}

result_t Kernel::ArbitrateUnlock(IThread* crnt_thread, uptr mutex_addr) {
    LOG_DEBUG(Kernel, "ArbitrateUnlock called (mutex: 0x{:08x})", mutex_addr);

    {
        CriticalSectionLock cs_lock;
        UnlockMutex(crnt_thread, mutex_addr);
    }

    return RESULT_SUCCESS;
}

result_t Kernel::WaitProcessWideKeyAtomic(Process* crnt_process,
                                          IThread* crnt_thread, uptr mutex_addr,
                                          uptr var_addr,
                                          handle_id_t self_handle,
                                          i64 timeout) {
    LOG_DEBUG(
        Kernel,
        "WaitProcessWideKeyAtomic called (mutex: 0x{:08x}, var: 0x{:08x}, "
        "self: 0x{:x}, timeout: {})",
        mutex_addr, var_addr, self_handle, timeout);

    crnt_thread->self_handle_for_mutex = self_handle;
    crnt_thread->mutex_wait_addr = mutex_addr;
    crnt_thread->cond_var_wait_addr = var_addr;

    crnt_thread->Pause();

    {
        CriticalSectionLock cs_lock;
        cond_var_waiters.AddLast(crnt_thread);
        UnlockMutex(crnt_thread, mutex_addr);
    }

    result_t res = RESULT_SUCCESS;
    if (crnt_thread->ProcessMessages(timeout)) {
        if (crnt_thread->GetState() == ThreadState::Stopping) {
            // TODO: is this necessary?
            res = MAKE_RESULT(Svc, Error::Cancelled);
        } else {
            if (!crnt_thread->WasSignalled())
                res = MAKE_RESULT(Svc, Error::Cancelled);
        }
    } else {
        res = MAKE_RESULT(Svc, Error::TimedOut);
    }

    // Remove this thread from the wait list
    {
        CriticalSectionLock cs_lock;

        // Cond var
        cond_var_waiters.Remove(crnt_thread);

        // Mutex
        auto owner = GetMutexOwner(
            crnt_process, static_cast<u32>(crnt_thread->mutex_wait_addr));
        if (owner)
            owner->RemoveMutexWaiter(crnt_thread);
    }

    return res;
}

result_t Kernel::SignalProcessWideKey(Process* crnt_process, uptr addr,
                                      i32 count) {
    LOG_DEBUG(Kernel, "SignalProcessWideKey called (addr: 0x{:08x}, count: {})",
              addr, count);

    CriticalSectionLock cs_lock;

    if (count == -1)
        count = static_cast<i32>(cond_var_waiters.GetSize());

    // TODO: sort by priority
    for (auto thread_node = cond_var_waiters.GetHead();
         thread_node && count > 0;) {
        const auto thread = thread_node->Get();
        if (thread->cond_var_wait_addr == addr) {
            thread->cond_var_wait_addr = 0x0;
            TryAcquireMutex(crnt_process, thread);
            thread_node = cond_var_waiters.Remove(thread_node);
            count--;
        } else {
            thread_node = thread_node->GetNext();
        }
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
    if (!client_session) {
        LOG_WARN(Kernel, "SendSyncRequest called (INVALID_HANDLE)");
        return MAKE_RESULT(Svc, Error::InvalidHandle);
    }

    LOG_DEBUG(Kernel, "SendSyncRequest called (session: {})",
              client_session->GetDebugName());

    // Pause the thread
    crnt_thread->Pause();

    // Send request
    client_session->GetParent()->GetServerSide()->EnqueueRequest(
        crnt_process, crnt_thread, crnt_thread->GetTlsPtr());

    // Wait for response
    crnt_thread->ProcessMessages();

    ASSERT_DEBUG(crnt_thread->ProcessMessages(), Kernel,
                 "SendSyncRequest timed out");

    result_t res = RESULT_SUCCESS;
    if (crnt_thread->GetState() == ThreadState::Stopping) {
        // TODO: is this necessary?
        res = MAKE_RESULT(Svc, Error::Cancelled);
    } else {
        if (!crnt_thread->WasSignalled())
            res = MAKE_RESULT(Svc, Error::Cancelled);
    }

    return res;
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
            const auto data_count = static_cast<u32>(buffer_size / sizeof(u32));
            for (u32 i = 0; i < std::min(data_count, MAX_DATA_COUNT); i++) {
                const u32 value = reinterpret_cast<u32*>(buffer_ptr)[i];
                LOG_INFO(Kernel, "0x{:08x}", value);
            }
            if (data_count > MAX_DATA_COUNT)
                LOG_INFO(Kernel, "...");
        }
    }

    if (!reason.notification_only)
        GET_CURRENT_PROCESS_DEBUGGER().BreakOnThisThread("Break");

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
        out_info = ALIAS_REGION.begin;
        return RESULT_SUCCESS;
    case InfoType::AliasRegionSize:
        out_info = ALIAS_REGION.GetSize();
        return RESULT_SUCCESS;
    case InfoType::HeapRegionAddress:
        out_info = HEAP_REGION.begin;
        return RESULT_SUCCESS;
    case InfoType::HeapRegionSize:
        out_info = HEAP_REGION.GetSize();
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
        out_info = ADDRESS_SPACE.begin;
        return RESULT_SUCCESS;
    case InfoType::AslrRegionSize:
        out_info = ADDRESS_SPACE.GetSize();
        return RESULT_SUCCESS;
    case InfoType::StackRegionAddress:
        out_info = STACK_REGION.begin;
        return RESULT_SUCCESS;
    case InfoType::StackRegionSize:
        out_info = STACK_REGION.GetSize();
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
    case InfoType::ProgramId:
        out_info = crnt_process->GetTitleID();
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

    if (!ALIAS_REGION.Contains(range<vaddr_t>::FromSize(addr, size)))
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

result_t Kernel::WaitForAddress(IThread* crnt_thread, uptr addr,
                                ArbitrationType arbitration_type, u32 value,
                                u64 timeout) {
    LOG_DEBUG(Kernel,
              "WaitForAddress called (addr: 0x{:08x}, type: {}, value: "
              "0x{:x}, timeout: 0x{:08x})",
              addr, arbitration_type, value, timeout);

    bool wait;
    {
        CriticalSectionLock cs_lock;

        auto value_ptr = reinterpret_cast<u32*>(addr);
        u32 current_value;
        switch (arbitration_type) {
        case ArbitrationType::WaitIfLessThan:
            current_value = atomic_load(value_ptr);
            wait = (current_value < value);
            break;
        case ArbitrationType::DecrementAndWaitIfLessThan:
            current_value = atomic_fetch_sub(value_ptr, 1u);
            wait = (current_value < value);
            break;
        case ArbitrationType::WaitIfEqual:
            wait = (current_value == value);
            break;
        }

        if (wait) {
            crnt_thread->Pause();

            crnt_thread->mutex_wait_addr = addr;
            arbiters.AddLast(crnt_thread);
        }
    }

    if (wait) {
        crnt_thread->ProcessMessages(timeout);

        result_t res = RESULT_SUCCESS;
        if (crnt_thread->ProcessMessages(timeout)) {
            if (crnt_thread->GetState() == ThreadState::Stopping) {
                // TODO: is this necessary?
                res = MAKE_RESULT(Svc, Error::Cancelled);
            } else {
                if (!crnt_thread->WasSignalled())
                    res = MAKE_RESULT(Svc, Error::Cancelled);
            }
        } else {
            res = MAKE_RESULT(Svc, Error::TimedOut);
        }

        // Remove the thread from the arbiter list
        {
            CriticalSectionLock cs_lock;
            arbiters.Remove(crnt_thread);
        }

        return res;
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

    auto sync_obj = sync_objs[out_signalled_index];
    if (auto server_session = dynamic_cast<hipc::ServerSession*>(sync_obj)) {
        if (server_session->IsClientOpen()) {
            // Receive
            server_session->Receive(crnt_thread);
            return RESULT_SUCCESS;
        } else {
            return MAKE_RESULT(Svc, 123); // SessionClosed
        }
    } else {
        return RESULT_SUCCESS;
    }
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

result_t Kernel::GetProcessList(u64* process_id_buffer,
                                u32 process_id_buffer_size, u32& out_count) {
    LOG_DEBUG(Kernel,
              "GetProcessList called (process_id_buffer: {}, "
              "process_id_buffer_size: {})",
              (void*)process_id_buffer, process_id_buffer_size);

    // TODO: is process_id_buffer_size in bytes or number of process IDs?
    for (auto it = process_manager.Begin();
         it != process_manager.End() && process_id_buffer_size >= sizeof(u64);
         it++) {
        // TODO: what is a process ID?
        *process_id_buffer++ = reinterpret_cast<u64>(*it); // HACK
        process_id_buffer_size -= sizeof(u64);
    }

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

result_t Kernel::MapProcessMemory(Process* crnt_process, vaddr_t dst_addr,
                                  Process* process, vaddr_t src_addr,
                                  u64 size) {
    LOG_DEBUG(Kernel,
              "MapProcessMemory called (crnt_process: {}, dst_addr: 0x{:08x}, "
              "process: {}, src_addr: 0x{:08x}, size: {})",
              crnt_process->GetDebugName(), dst_addr, process->GetDebugName(),
              src_addr, size);

    // TODO: correct?
    const auto ptr = process->GetMmu()->UnmapAddr(src_addr);
    crnt_process->GetMmu()->Map(dst_addr, ptr, size, {}); // TODO: state

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

void Kernel::TryAcquireMutex(Process* crnt_process, IThread* thread) {
    auto mutex = reinterpret_cast<u32*>(thread->mutex_wait_addr);

    u32 value = *mutex;
    u32 new_value;
    do {
        if (value == 0) {
            // Register this thread as the owner
            new_value = thread->self_handle_for_mutex;
        } else {
            // Register this thread as a waiter
            new_value = value | MUTEX_WAIT_MASK;
        }
    } while (!atomic_compare_exchange_weak(mutex, value, new_value));

    if (value == 0) {
        // Mutex acquired
        thread->mutex_wait_addr = 0x0;
        thread->Resume();
        return;
    }

    // Register this thread as a waiter by the owner
    auto owner = GetMutexOwner(crnt_process, value);
    owner->AddMutexWaiter(thread);
}

void Kernel::UnlockMutex(IThread* thread, uptr mutex_addr) {
    auto mutex = reinterpret_cast<u32*>(mutex_addr);

    u32 waiter_count;
    auto new_owner = thread->RelinquishMutex(mutex_addr, waiter_count);
    if (!new_owner) {
        atomic_store(mutex, 0u);
        return;
    }

    u32 value = new_owner->self_handle_for_mutex;
    if (waiter_count > 0)
        value |= MUTEX_WAIT_MASK;

    atomic_store(mutex, value);

    // Resume the owner
    new_owner->Resume();
}

} // namespace hydra::horizon::kernel
