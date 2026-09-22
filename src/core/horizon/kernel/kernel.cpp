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
#include "core/system.hpp"

namespace hydra::horizon::kernel {

Kernel::Kernel(System& system_) : system{system_}, process_manager(system) {}

void Kernel::supervisorCall(Process* crnt_process, IThread* crnt_thread,
                            hw::tegra_x1::cpu::IThread* guest_thread, u64 id) {
    auto& state = guest_thread->getState();

    i32 tmp_i32;
    u32 tmp_u32;
    u64 tmp_u64;
    uptr tmp_uptr;
    switch (id) {
    case 0x1:
        state.r[0] = setHeapSize(crnt_process, state.r[1], tmp_uptr);
        state.r[1] = tmp_uptr;
        break;
    case 0x2:
        state.r[0] = setMemoryPermission(
            state.r[0], state.r[1], static_cast<MemoryPermission>(state.r[2]));
        break;
    case 0x3:
        state.r[0] =
            setMemoryAttribute(crnt_process, state.r[0], state.r[1],
                               static_cast<MemoryAttribute>(state.r[2]),
                               static_cast<MemoryAttribute>(state.r[3]));
        break;
    case 0x4:
        state.r[0] =
            mapMemory(crnt_process, state.r[0], state.r[1], state.r[2]);
        break;
    case 0x5:
        state.r[0] =
            unmapMemory(crnt_process, state.r[0], state.r[1], state.r[2]);
        break;
    case 0x6:
        state.r[0] =
            queryMemory(crnt_process, state.r[2],
                        *reinterpret_cast<MemoryInfo*>(
                            crnt_process->getMmu()->unmapAddr(state.r[0])),
                        tmp_u32);
        state.r[1] = tmp_u32;
        break;
    case 0x7:
        exitProcess(crnt_process);
        break;
    case 0x8: {
        IThread* thread = nullptr;
        state.r[0] = createThread(
            crnt_process, state.r[1], state.r[2], state.r[3],
            std::bit_cast<i32>(static_cast<u32>(state.r[4])),
            std::bit_cast<i32>(static_cast<u32>(state.r[5])), thread);
        state.r[1] = crnt_process->addHandleNoRetain(thread).getRaw();
        break;
    }
    case 0x9:
        state.r[0] = startThread(crnt_process->getHandle<IThread>(
            Handle(static_cast<u32>(state.r[0]))));
        break;
    case 0xa:
        exitThread(crnt_thread);
        break;
    case 0xb:
        sleepThread(std::bit_cast<i64>(state.r[0]));
        break;
    case 0xc:
        state.r[0] = getThreadPriority(
            crnt_process->getHandle<IThread>(static_cast<u32>(state.r[1])),
            tmp_i32);
        state.r[1] = std::bit_cast<u32>(tmp_i32);
        break;
    case 0xd:
        state.r[0] = setThreadPriority(
            crnt_process->getHandle<IThread>(static_cast<u32>(state.r[0])),
            std::bit_cast<i32>(static_cast<u32>(state.r[1])));
        break;
    case 0xe:
        state.r[0] = getThreadCoreMask(
            crnt_process->getHandle<IThread>(static_cast<u32>(state.r[0])),
            tmp_i32, tmp_u64);
        state.r[1] = std::bit_cast<u32>(tmp_i32);
        state.r[2] = tmp_u64;
        break;
    case 0xf:
        state.r[0] = setThreadCoreMask(
            crnt_process->getHandle<IThread>(static_cast<u32>(state.r[0])),
            std::bit_cast<i32>(static_cast<u32>(state.r[1])), state.r[2]);
        break;
    case 0x10:
        getCurrentProcessorNumber(tmp_u32);
        state.r[0] = tmp_u32;
        break;
    case 0x11:
        state.r[0] = signalEvent(
            crnt_process->getHandle<Event>(static_cast<u32>(state.r[0])));
        break;
    case 0x12:
        state.r[0] = clearEvent(
            crnt_process->getHandle<Event>(static_cast<u32>(state.r[0])));
        break;
    case 0x13:
        state.r[0] = mapSharedMemory(
            crnt_process,
            crnt_process->getHandle<SharedMemory>(static_cast<u32>(state.r[0])),
            state.r[1], state.r[2], static_cast<MemoryPermission>(state.r[3]));
        break;
    case 0x14:
        state.r[0] = unmapSharedMemory(
            crnt_process,
            crnt_process->getHandle<SharedMemory>(static_cast<u32>(state.r[0])),
            state.r[1], state.r[2]);
        break;
    case 0x15: {
        TransferMemory* tmem = nullptr;
        state.r[0] = createTransferMemory(
            state.r[1], state.r[2], static_cast<MemoryPermission>(state.r[3]),
            tmem);
        state.r[1] = crnt_process->addHandleNoRetain(tmem).getRaw();
        break;
    }
    case 0x16:
        state.r[0] = closeHandle(crnt_process, static_cast<u32>(state.r[0]));
        break;
    case 0x17:
        state.r[0] = resetSignal(crnt_process->getHandle<SynchronizationObject>(
            static_cast<u32>(state.r[0])));
        break;
    case 0x18: {
        const auto handles = reinterpret_cast<Handle*>(
            crnt_process->getMmu()->unmapAddr(state.r[1]));
        const auto num_handles = std::bit_cast<i64>(state.r[2]);
        SynchronizationObject* sync_objs[num_handles];
        for (auto i = 0; i < num_handles; i++)
            sync_objs[i] =
                crnt_process->getHandle<SynchronizationObject>(handles[i]);

        state.r[0] = waitSynchronization(
            crnt_thread, std::span(sync_objs, static_cast<usize>(num_handles)),
            std::bit_cast<i64>(state.r[3]), tmp_u32);
        state.r[1] = tmp_u32;
        break;
    }
    case 0x19:
        state.r[0] = cancelSynchronization(
            crnt_process->getHandle<IThread>(static_cast<u32>(state.r[0])));
        break;
    case 0x1a:
        state.r[0] = arbitrateLock(
            crnt_thread,
            crnt_process->getHandle<IThread>(static_cast<u32>(state.r[0])),
            crnt_process->getMmu()->unmapAddr(state.r[1]),
            static_cast<u32>(state.r[2]), static_cast<u32>(state.r[0]));
        break;
    case 0x1b:
        state.r[0] = arbitrateUnlock(
            crnt_thread, crnt_process->getMmu()->unmapAddr(state.r[0]));
        break;
    case 0x1c:
        state.r[0] = waitProcessWideKeyAtomic(
            crnt_process, crnt_thread,
            crnt_process->getMmu()->unmapAddr(state.r[0]),
            crnt_process->getMmu()->unmapAddr(state.r[1]),
            static_cast<u32>(state.r[2]), std::bit_cast<i64>(state.r[3]));
        break;
    case 0x1d:
        state.r[0] = signalProcessWideKey(
            crnt_process, crnt_process->getMmu()->unmapAddr(state.r[0]),
            std::bit_cast<i32>(static_cast<u32>(state.r[1])));
        break;
    case 0x1e:
        getSystemTick(tmp_u64);
        state.r[0] = tmp_u64;
        break;
    case 0x1f: {
        hipc::ClientSession* client_session = nullptr;
        state.r[0] = connectToNamedPort(
            reinterpret_cast<const char*>(
                crnt_process->getMmu()->unmapAddr(state.r[1])),
            client_session);
        state.r[1] = crnt_process->addHandleNoRetain(client_session).getRaw();
        break;
    }
    case 0x21:
        state.r[0] =
            sendSyncRequest(crnt_process, crnt_thread,
                            crnt_process->getHandle<hipc::ClientSession>(
                                static_cast<u32>(state.r[0])));
        break;
    case 0x25:
        state.r[0] = getThreadId(
            crnt_process->getHandle<IThread>(static_cast<u32>(state.r[1])),
            tmp_u64);
        state.r[1] = tmp_u64;
        break;
    case 0x26: {
        const vaddr_t addr = state.r[1];
        state.r[0] = break_(
            BreakReason(state.r[0]),
            (addr != 0x0 ? crnt_process->getMmu()->unmapAddr(addr) : 0x0),
            state.r[2]);
        break;
    }
    case 0x27:
        state.r[0] = outputDebugString(
            reinterpret_cast<const char*>(
                crnt_process->getMmu()->unmapAddr(state.r[0])),
            state.r[1]);
        break;
    case 0x29:
        state.r[0] = getInfo(
            crnt_process, static_cast<InfoType>(state.r[1]),
            crnt_process->getHandle<AutoObject>(static_cast<u32>(state.r[2])),
            state.r[3], tmp_u64);
        state.r[1] = tmp_u64;
        break;
    case 0x2c:
        state.r[0] = mapPhysicalMemory(crnt_process, state.r[0], state.r[1]);
        break;
    case 0x32:
        state.r[0] = setThreadActivity(
            crnt_process->getHandle<IThread>(static_cast<u32>(state.r[0])),
            static_cast<ThreadActivity>(state.r[1]));
        break;
    case 0x33:
        state.r[0] = getThreadContext3(
            crnt_process->getHandle<IThread>(static_cast<u32>(state.r[1])),
            *reinterpret_cast<ThreadContext*>(
                crnt_process->getMmu()->unmapAddr(state.r[0])));
        break;
    case 0x34:
        state.r[0] = waitForAddress(
            crnt_thread, crnt_process->getMmu()->unmapAddr(state.r[0]),
            static_cast<ArbitrationType>(state.r[1]),
            static_cast<u32>(state.r[2]), std::bit_cast<i64>(state.r[3]));
        break;
    case 0x35:
        state.r[0] = signalToAddress(
            crnt_process->getMmu()->unmapAddr(state.r[0]),
            static_cast<SignalType>(state.r[1]), static_cast<u32>(state.r[2]),
            static_cast<u32>(state.r[3]));
        break;
    case 0x36:
        synchronizePreemptionState(crnt_thread);
        break;
    case 0x40: {
        hipc::ServerSession* server_session = nullptr;
        hipc::ClientSession* client_session = nullptr;
        state.r[0] = createSession(state.r[2] != 0, state.r[3], server_session,
                                   client_session);
        state.r[1] = crnt_process->addHandleNoRetain(server_session).getRaw();
        state.r[2] = crnt_process->addHandleNoRetain(client_session).getRaw();
        break;
    }
    case 0x41: {
        hipc::ServerSession* server_session = nullptr;
        state.r[0] = acceptSession(crnt_process->getHandle<hipc::ServerPort>(
                                       static_cast<u32>(state.r[1])),
                                   server_session);
        state.r[1] = crnt_process->addHandleNoRetain(server_session).getRaw();
        break;
    }
    case 0x43: {
        const auto handles = reinterpret_cast<Handle*>(
            crnt_process->getMmu()->unmapAddr(state.r[1]));
        const auto num_handles = std::bit_cast<i64>(state.r[2]);
        SynchronizationObject* sync_objs[num_handles];
        for (auto i = 0; i < num_handles; i++)
            sync_objs[i] =
                crnt_process->getHandle<SynchronizationObject>(handles[i]);

        state.r[0] = replyAndReceive(
            crnt_thread, std::span(sync_objs, static_cast<usize>(num_handles)),
            crnt_process->getHandle<hipc::ServerSession>(
                static_cast<u32>(state.r[3])),
            std::bit_cast<i64>(state.r[4]), tmp_u32);
        state.r[1] = tmp_u32;
        break;
    }
    case 0x4b: {
        CodeMemory* code_mem = nullptr;
        state.r[0] = createCodeMemory(state.r[1], state.r[2], code_mem);
        state.r[1] = crnt_process->addHandleNoRetain(code_mem).getRaw();
        break;
    }
    case 0x4c:
        state.r[0] = controlCodeMemory(
            crnt_process->getHandle<CodeMemory>(static_cast<u32>(state.r[0])),
            CodeMemoryOperation(state.r[1]), state.r[2], state.r[3],
            MemoryPermission(state.r[4]));
        break;
    case 0x65:
        state.r[0] =
            getProcessList(reinterpret_cast<u64*>(
                               crnt_process->getMmu()->unmapAddr(state.r[1])),
                           static_cast<u32>(state.r[2]), tmp_u32);
        state.r[1] = tmp_u32;
        break;
    case 0x73:
        state.r[0] = setProcessMemoryPermission(
            crnt_process->getHandle<Process>(static_cast<u32>(state.r[0])),
            state.r[1], state.r[2], MemoryPermission(state.r[3]));
        break;
    case 0x74:
        state.r[0] = mapProcessMemory(
            crnt_process, state.r[0],
            crnt_process->getHandle<Process>(static_cast<u32>(state.r[1])),
            state.r[2], state.r[3]);
        break;
    case 0x77:
        state.r[0] = mapProcessCodeMemory(
            crnt_process->getHandle<Process>(static_cast<u32>(state.r[0])),
            state.r[1], state.r[2], state.r[3]);
        break;
    case 0x78:
        state.r[0] = unmapProcessCodeMemory(
            crnt_process->getHandle<Process>(static_cast<u32>(state.r[0])),
            state.r[1], state.r[2], state.r[3]);
        break;
    default:
        LOG_NOT_IMPLEMENTED(Kernel, "SVC {:#x}", id);
        state.r[0] = MAKE_RESULT(Svc, Error::NotImplemented);
        break;
    }
}

result_t Kernel::setHeapSize(Process* crnt_process, u64 size, uptr& out_base) {
    LOG_DEBUG(Kernel, "SetHeapSize called (size: 0x{:08x})", size);

    if ((size % HEAP_MEM_ALIGNMENT) != 0)
        return MAKE_RESULT(Svc, Error::InvalidSize); // TODO: correct?

    crnt_process->resizeHeap(size);

    out_base = HEAP_REGION.getBegin();
    return RESULT_SUCCESS;
}

result_t Kernel::setMemoryPermission(uptr addr, u64 size,
                                     MemoryPermission perm) {
    LOG_DEBUG(
        Kernel,
        "SetMemoryPermission called (addr: 0x{:08x}, size: 0x{:08x}, perm: "
        "{})",
        addr, size, perm);

    // TODO: implement
    LOG_FUNC_WITH_ARGS_STUBBED(
        Kernel, "addr: 0x{:08x}, size: 0x{:08x}, perm: {}", addr, size, perm);

    return RESULT_SUCCESS;
}

result_t Kernel::setMemoryAttribute(Process* crnt_process, vaddr_t addr,
                                    u64 size, MemoryAttribute mask,
                                    MemoryAttribute value) {
    LOG_DEBUG(
        Kernel,
        "SetMemoryAttribute called (addr: 0x{:08x}, size: 0x{:08x}, mask: "
        "{}, value: {})",
        addr, size, mask, value);

    crnt_process->getMmu()->setMemoryAttribute(
        ztd::Range<vaddr_t>::fromSize(addr, size), mask, value);

    return RESULT_SUCCESS;
}

result_t Kernel::mapMemory(Process* crnt_process, uptr dst_addr, uptr src_addr,
                           u64 size) {
    LOG_DEBUG(Kernel,
              "MapMemory called (dst_addr: 0x{:08x}, src_addr: 0x{:08x}, size: "
              "0x{:08x})",
              dst_addr, src_addr, size);

    crnt_process->getMmu()->map(dst_addr,
                                ztd::Range<vaddr_t>::fromSize(src_addr, size));

    return RESULT_SUCCESS;
}

result_t Kernel::unmapMemory(Process* crnt_process, uptr dst_addr,
                             uptr src_addr, u64 size) {
    LOG_DEBUG(
        Kernel,
        "UnmapMemory called (dst_addr: 0x{:08x}, src_addr: 0x{:08x}, size: "
        "0x{:08x})",
        dst_addr, src_addr, size);

    // TODO: verify that src_addr is the same as the one used in MapMemory?
    (void)src_addr;

    crnt_process->getMmu()->unmap(
        ztd::Range<vaddr_t>::fromSize(dst_addr, size));

    return RESULT_SUCCESS;
}

result_t Kernel::queryMemory(Process* crnt_process, uptr addr,
                             MemoryInfo& out_mem_info, u32& out_page_info) {
    LOG_DEBUG(Kernel, "QueryMemory called (addr: 0x{:08x})", addr);

    out_mem_info = crnt_process->getMmu()->queryMemory(addr);

    // TODO: what is this?
    out_page_info = 0;

    return RESULT_SUCCESS;
}

void Kernel::exitProcess(Process* crnt_process) {
    LOG_DEBUG(Kernel, "ExitProcess called");

    crnt_process->stop();
}

result_t Kernel::createThread(Process* crnt_process, vaddr_t entry_point,
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
    (void)processor_id;
    auto thread =
        new GuestThread(system, crnt_process, stack_top_addr, priority);
    thread->setEntryPoint(entry_point);
    thread->setArg(0, args_addr);

    out_thread = thread;

    return RESULT_SUCCESS;
}

result_t Kernel::startThread(IThread* thread) {
    LOG_DEBUG(Kernel, "StartThread called (thread: {})",
              thread->getDebugName());

    thread->start();

    return RESULT_SUCCESS;
}

void Kernel::exitThread(IThread* crnt_thread) {
    LOG_DEBUG(Kernel, "ExitThread called");

    crnt_thread->stop();
}

void Kernel::sleepThread(i64 nano) {
    LOG_DEBUG(Kernel, "SleepThread called (nano: {})", nano);

    if (nano == 0 || nano == -1 || nano == -2)
        std::this_thread::yield();
    else
        std::this_thread::sleep_for(std::chrono::nanoseconds(nano));
}

result_t Kernel::getThreadPriority(IThread* thread, i32& out_priority) {
    LOG_DEBUG(Kernel, "GetThreadPriority called (thread: {})",
              thread->getDebugName());

    // TODO: implement
    LOG_FUNC_WITH_ARGS_STUBBED(Kernel, "thread: {}", thread->getDebugName());

    // HACK
    out_priority = 0x20; // 0x0 - 0x3f, lower is higher priority
    return RESULT_SUCCESS;
}

result_t Kernel::setThreadPriority(IThread* thread, i32 priority) {
    LOG_DEBUG(Kernel, "SetThreadPriority called (thread: {}, priority: 0x{:x})",
              thread->getDebugName(), priority);

    // TODO: implement
    LOG_FUNC_WITH_ARGS_STUBBED(Kernel, "thread: {}, priority: 0x{:x}",
                               thread->getDebugName(), priority);

    return RESULT_SUCCESS;
}

result_t Kernel::getThreadCoreMask(IThread* thread, i32& out_core_mask0,
                                   u64& out_core_mask1) {
    // HACK: for botw
    LOG_DEBUG(Kernel, "GetThreadCoreMask called (thread: {})",
              thread ? thread->getDebugName() : "null");

    // TODO: implement
    LOG_FUNC_WITH_ARGS_STUBBED(Kernel, "thread: {}",
                               thread ? thread->getDebugName() : "null");

    // HACK
    out_core_mask0 = 0x1;
    out_core_mask1 = 0x2;
    return RESULT_SUCCESS;
}

result_t Kernel::setThreadCoreMask(IThread* thread, i32 core_mask0,
                                   u64 core_mask1) {
    LOG_DEBUG(Kernel,
              "SetThreadCoreMask called (thread: {}, core mask 0: "
              "{:#x}, core mask 1: {:#x})",
              thread->getDebugName(), core_mask0, core_mask1);

    // TODO: implement
    LOG_FUNC_WITH_ARGS_STUBBED(
        Kernel, "thread: {}, core mask 0: {:#x}, core mask 1: {:#x}",
        thread->getDebugName(), core_mask0, core_mask1);

    return RESULT_SUCCESS;
}

void Kernel::getCurrentProcessorNumber(u32& out_number) {
    LOG_DEBUG(Kernel, "GetCurrentProcessorNumber called");

    // TODO: implement
    LOG_FUNC_STUBBED(Kernel);

    // HACK
    out_number = 0;
}

result_t Kernel::signalEvent(Event* event) {
    LOG_DEBUG(Kernel, "SignalEvent called (event: {})", event->getDebugName());

    event->signal();
    return RESULT_SUCCESS;
}

result_t Kernel::clearEvent(Event* event) {
    LOG_DEBUG(Kernel, "ClearEvent called (event: {})", event->getDebugName());

    event->clear();
    return RESULT_SUCCESS;
}

result_t Kernel::mapSharedMemory(Process* crnt_process, SharedMemory* shmem,
                                 uptr addr, u64 size, MemoryPermission perm) {
    LOG_DEBUG(Kernel,
              "MapSharedMemory called (handle: {}, addr: 0x{:08x}, size: "
              "0x{:08x}, perm: {})",
              shmem->getDebugName(), addr, size, perm);

    shmem->mapToRange(crnt_process->getMmu(),
                      ztd::Range(addr, static_cast<uptr>(addr + size)), perm);

    return RESULT_SUCCESS;
}

result_t Kernel::unmapSharedMemory(Process* crnt_process, SharedMemory* shmem,
                                   uptr addr, u64 size) {
    (void)shmem;

    LOG_DEBUG(Kernel,
              "UnmapSharedMemory called (shmem: {}, addr: 0x{:08x}, size: "
              "0x{:08x})",
              shmem->getDebugName(), addr, size);

    crnt_process->getMmu()->unmap(ztd::Range<vaddr_t>::fromSize(addr, size));
    return RESULT_SUCCESS;
}

result_t Kernel::createTransferMemory(uptr addr, u64 size,
                                      MemoryPermission perm,
                                      TransferMemory*& out_tmem) {
    LOG_DEBUG(Kernel,
              "CreateTransferMemory called (address: 0x{:08x}, size: 0x{:08x}, "
              "perm: {})",
              addr, size, perm);

    out_tmem = new TransferMemory(addr, size, perm);

    return RESULT_SUCCESS;
}

result_t Kernel::closeHandle(Process* crnt_process, Handle handle) {
    LOG_DEBUG(Kernel, "CloseHandle called (handle: {})", handle);

    if (crnt_process->freeHandle(handle)) {
        return RESULT_SUCCESS;
    } else {
        return MAKE_RESULT(Svc, Error::InvalidHandle);
    }
}

// TODO: can only be ReadableEvent or Process?
result_t Kernel::resetSignal(SynchronizationObject* sync_obj) {
    if (sync_obj == nullptr) {
        LOG_WARN(Kernel, "ResetSignal called (INVALID_HANDLE)");
        // HACK
        return RESULT_SUCCESS; // MAKE_RESULT(Svc, Error::InvalidHandle);
    }

    LOG_DEBUG(Kernel, "ResetSignal called (sync_obj: {})",
              sync_obj->getDebugName());

    if (!sync_obj->clear())
        return MAKE_RESULT(Svc, Error::InvalidState);

    return RESULT_SUCCESS;
}

result_t
Kernel::waitSynchronization(IThread* crnt_thread,
                            std::span<SynchronizationObject*> sync_objs,
                            i64 timeout, u32& out_signalled_index) {
    LOG_DEBUG(Kernel,
              "WaitSynchronization called (count: {}, timeout: "
              "{})",
              sync_objs.size(), timeout);

    for (auto& sync_obj : sync_objs) {
        if (sync_obj == nullptr) {
            LOG_WARN(Kernel, "Invalid sync object");
            // HACK: Celeste gets stuck in an infinite WaitSynchronization
            // loop if an error is returned
            return RESULT_SUCCESS; // MAKE_RESULT(Svc,
                                   // Error::InvalidHandle);
        }
    }

    crnt_thread->pause();

    // Add waiting thread
    for (auto sync_obj : sync_objs) {
        // LOG_DEBUG(Kernel, "Synchronizing with {}",
        //           sync_obj->GetDebugName());

        sync_obj->addWaitingThread(crnt_thread);
    }

    SynchronizationObject* signalled_obj = nullptr;
    result_t res = RESULT_SUCCESS;
    if (crnt_thread->processMessages(timeout)) {
        if (crnt_thread->getState() == ThreadState::Stopping) {
            // TODO: is this necessary?
            res = MAKE_RESULT(Svc, Error::Cancelled);
        } else {
            if (!crnt_thread->consumeSignalledObject(signalled_obj))
                res = MAKE_RESULT(Svc, Error::Cancelled);
        }
    } else {
        res = MAKE_RESULT(Svc, Error::TimedOut);
    }

    // Remove the thread from the waiting list
    for (auto sync_obj : sync_objs) {
        if (sync_obj != signalled_obj)
            sync_obj->removeWaitingThread(crnt_thread);
    }

    // Find the handle index
    out_signalled_index = 0;
    if (signalled_obj != nullptr) {
        for (u32 i = 0; i < sync_objs.size(); i++) {
            if (sync_objs[i] == signalled_obj) {
                out_signalled_index = i;
                break;
            }
        }
    }

    return res;
}

result_t Kernel::cancelSynchronization(IThread* thread) {
    LOG_DEBUG(Kernel, "CancelSynchronization called (thread: {})",
              thread->getDebugName());

    thread->cancelSync();

    return RESULT_SUCCESS;
}

result_t Kernel::arbitrateLock(IThread* crnt_thread, IThread* owner_thread,
                               uptr mutex_addr, Handle self_handle,
                               Handle owner_handle) {
    LOG_DEBUG(Kernel,
              "ArbitrateLock called (owner: {}, mutex: {:#x}, self: "
              "{})",
              owner_thread->getDebugName(), mutex_addr, self_handle);

    crnt_thread->self_handle_for_mutex = self_handle;
    owner_thread->self_handle_for_mutex = owner_handle;

    {
        CriticalSectionLock cs_lock(*this);

        if (atomicLoad(reinterpret_cast<u32*>(mutex_addr)) !=
            (owner_thread->self_handle_for_mutex.getRaw() | MUTEX_WAIT_MASK))
            return RESULT_SUCCESS;

        crnt_thread->mutex_wait_addr = mutex_addr;

        crnt_thread->pause();
        owner_thread->addMutexWaiter(crnt_thread);
    }

    ASSERT_DEBUG(crnt_thread->processMessages(), Kernel,
                 "ArbitrateLock timed out");

    result_t res = RESULT_SUCCESS;
    if (crnt_thread->getState() == ThreadState::Stopping) {
        // TODO: is this necessary?
        res = MAKE_RESULT(Svc, Error::Cancelled);
    } else {
        if (!crnt_thread->wasSignalled())
            res = MAKE_RESULT(Svc, Error::Cancelled);
    }

    return res;
}

result_t Kernel::arbitrateUnlock(IThread* crnt_thread, uptr mutex_addr) {
    LOG_DEBUG(Kernel, "ArbitrateUnlock called (mutex: 0x{:08x})", mutex_addr);

    {
        CriticalSectionLock cs_lock(*this);
        unlockMutex(crnt_thread, mutex_addr);
    }

    return RESULT_SUCCESS;
}

result_t Kernel::waitProcessWideKeyAtomic(Process* crnt_process,
                                          IThread* crnt_thread, uptr mutex_addr,
                                          uptr var_addr, Handle self_handle,
                                          i64 timeout) {
    LOG_DEBUG(Kernel,
              "WaitProcessWideKeyAtomic called (mutex: {:#x}, var: {:#x}, "
              "self: {}, timeout: {})",
              mutex_addr, var_addr, self_handle, timeout);

    crnt_thread->self_handle_for_mutex = self_handle;
    crnt_thread->mutex_wait_addr = mutex_addr;
    crnt_thread->cond_var_wait_addr = var_addr;

    crnt_thread->pause();

    {
        CriticalSectionLock cs_lock(*this);
        ASSERT_DEBUG(cond_var_waiters.addLast(crnt_thread).has_value(), Kernel,
                     "Failed to add cond var waiter");
        unlockMutex(crnt_thread, mutex_addr);
    }

    result_t res = RESULT_SUCCESS;
    if (crnt_thread->processMessages(timeout)) {
        if (crnt_thread->getState() == ThreadState::Stopping) {
            // TODO: is this necessary?
            res = MAKE_RESULT(Svc, Error::Cancelled);
        } else {
            if (!crnt_thread->wasSignalled())
                res = MAKE_RESULT(Svc, Error::Cancelled);
        }
    } else {
        res = MAKE_RESULT(Svc, Error::TimedOut);
    }

    // Remove this thread from the wait list
    {
        CriticalSectionLock cs_lock(*this);

        // Cond var
        cond_var_waiters.remove(crnt_thread);

        // Mutex
        auto owner = getMutexOwner(
            crnt_process, reinterpret_cast<u32*>(crnt_thread->mutex_wait_addr));
        if (owner.has_value())
            owner.value()->removeMutexWaiter(crnt_thread);
    }

    return res;
}

result_t Kernel::signalProcessWideKey(Process* crnt_process, uptr addr,
                                      i32 count) {
    LOG_DEBUG(Kernel, "SignalProcessWideKey called (addr: 0x{:08x}, count: {})",
              addr, count);

    CriticalSectionLock cs_lock(*this);

    if (count == -1)
        count = static_cast<i32>(cond_var_waiters.getSize());

    // TODO: sort by priority
    for (auto thread_node = cond_var_waiters.getHead();
         thread_node.has_value() && count > 0;) {
        const auto thread_node_ = thread_node.value();
        const auto thread = thread_node_->get();
        if (thread->cond_var_wait_addr == addr) {
            thread->cond_var_wait_addr = 0x0;
            tryAcquireMutex(crnt_process, thread);
            thread_node = cond_var_waiters.remove(thread_node_);
            count--;
        } else {
            thread_node = thread_node_->getNext();
        }
    }

    return RESULT_SUCCESS;
}

void Kernel::getSystemTick(u64& out_tick) {
    LOG_DEBUG(Kernel, "GetSystemTick called");

    out_tick = system.getWallClock().getCntpct(); // TODO: correct?
}

result_t Kernel::connectToNamedPort(const std::string_view name,
                                    hipc::ClientSession*& out_client_session) {
    LOG_DEBUG(Kernel, "ConnectToNamedPort called (name: {})", name);

    auto port = service_manager.getPort(std::string(name));
    if (port == nullptr) {
        LOG_ERROR(Kernel, "Failed to connect to port \"{}\"", name);
        return MAKE_RESULT(Svc, Error::NotFound);
    }

    out_client_session = port->connect();

    return RESULT_SUCCESS;
}

result_t Kernel::sendSyncRequest(Process* crnt_process, IThread* crnt_thread,
                                 hipc::ClientSession* client_session) {
    if (client_session == nullptr) {
        LOG_WARN(Kernel, "SendSyncRequest called (INVALID_HANDLE)");
        return MAKE_RESULT(Svc, Error::InvalidHandle);
    }

    LOG_DEBUG(Kernel, "SendSyncRequest called (session: {})",
              client_session->getDebugName());

    // Pause the thread
    crnt_thread->pause();

    // Send request
    client_session->getParent()->getServerSide()->enqueueRequest(
        crnt_process, crnt_thread, crnt_thread->getTlsPtr());

    // Wait for response
    crnt_thread->processMessages();

    ASSERT_DEBUG(crnt_thread->processMessages(), Kernel,
                 "SendSyncRequest timed out");

    result_t res = RESULT_SUCCESS;
    if (crnt_thread->getState() == ThreadState::Stopping) {
        // TODO: is this necessary?
        res = MAKE_RESULT(Svc, Error::Cancelled);
    } else {
        if (!crnt_thread->wasSignalled())
            res = MAKE_RESULT(Svc, Error::Cancelled);
    }

    return res;
}

result_t Kernel::getThreadId(IThread* thread, u64& out_thread_id) {
    LOG_DEBUG(Kernel, "GetThreadId called (thread: {})",
              thread->getDebugName());

    // TODO: implement
    LOG_FUNC_STUBBED(Services);

    // HACK
    out_thread_id = std::bit_cast<u64>(thread);

    return RESULT_SUCCESS;
}

result_t Kernel::break_(BreakReason reason, uptr buffer_ptr, u64 buffer_size) {
    LOG_DEBUG(Kernel,
              "Break called (reason: {}, buffer ptr: 0x{:08x}, buffer "
              "size: 0x{:08x})",
              reason.type, buffer_ptr, buffer_size);

    // TODO: this should be sent to the debugger instead of being logged
    if (buffer_ptr != 0u) {
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
        GET_CURRENT_PROCESS_DEBUGGER().breakOnThisThread("Break");

    return RESULT_SUCCESS;
}

result_t Kernel::outputDebugString(const std::string_view str, u64 len) {
    LOG_DEBUG(Kernel, "OutputDebugString called");
    if (len != 0) {
        // TODO: handle differently
        LOG_INFO(Kernel, "{}", str);
    }

    return RESULT_SUCCESS;
}

// TODO: object
result_t Kernel::getInfo(Process* crnt_process, InfoType info_type,
                         AutoObject* obj, u64 info_sub_type, u64& out_info) {
    (void)obj;

    LOG_DEBUG(Kernel, "GetInfo called (type: {}, object: {}, subtype: {})",
              info_type, (obj != nullptr ? obj->getDebugName() : "null"),
              info_sub_type);

    switch (info_type) {
    case InfoType::CoreMask:
        LOG_NOT_IMPLEMENTED(Kernel, "CoreMask");
        // HACK
        out_info = 0xf;
        return RESULT_SUCCESS;
    case InfoType::AliasRegionAddress:
        out_info = ALIAS_REGION.getBegin();
        return RESULT_SUCCESS;
    case InfoType::AliasRegionSize:
        out_info = ALIAS_REGION.getSize();
        return RESULT_SUCCESS;
    case InfoType::HeapRegionAddress:
        out_info = HEAP_REGION.getBegin();
        return RESULT_SUCCESS;
    case InfoType::HeapRegionSize:
        out_info = HEAP_REGION.getSize();
        return RESULT_SUCCESS;
    case InfoType::TotalMemorySize:
        // TODO: what should this be?
        out_info = 3ull * 1024ull * 1024ull * 1024ull;
        return RESULT_SUCCESS;
    case InfoType::UsedMemorySize: {
        // TODO: correct?
        /*
        u64 size = stack_mem->GetSize() + kernel_mem->GetSize() +
                     tls_mem->GetSize() + heap_mem->GetSize();
        for (auto executable_mem : executable_memories)
            size += executable_mem->GetSize();
        out_info = size;
        */
        out_info = 4ull * 1024ull * 1024ull;
        return RESULT_SUCCESS;
    }
    case InfoType::DebuggerAttached:
        // TODO: make this configurable
        out_info = static_cast<u64>(true);
        return RESULT_SUCCESS;
    case InfoType::RandomEntropy:
        ASSERT_DEBUG(info_sub_type < crnt_process->getRandomEntropy().size(),
                     Kernel, "Invalid random entropy index {}", info_sub_type);
        out_info = crnt_process->getRandomEntropy()[info_sub_type];
        return RESULT_SUCCESS;
    case InfoType::AslrRegionAddress:
        out_info = ADDRESS_SPACE.getBegin();
        return RESULT_SUCCESS;
    case InfoType::AslrRegionSize:
        out_info = ADDRESS_SPACE.getSize();
        return RESULT_SUCCESS;
    case InfoType::StackRegionAddress:
        out_info = STACK_REGION.getBegin();
        return RESULT_SUCCESS;
    case InfoType::StackRegionSize:
        out_info = STACK_REGION.getSize();
        return RESULT_SUCCESS;
    case InfoType::TotalSystemResourceSize: {
        out_info = crnt_process->getSystemResourceSize();
        return RESULT_SUCCESS;
    }
    case InfoType::UsedSystemResourceSize:
        LOG_NOT_IMPLEMENTED(Kernel, "UsedSystemResourceSize");
        // HACK
        out_info = 64ull * 1024ull;
        return RESULT_SUCCESS;
    case InfoType::ProgramId:
        out_info = crnt_process->getTitleId();
        return RESULT_SUCCESS;
    case InfoType::UserExceptionContextAddress:
        LOG_NOT_IMPLEMENTED(Kernel, "UserExceptionContextAddress");
        // HACK
        out_info = 0;
        return RESULT_SUCCESS;
    case InfoType::TotalNonSystemMemorySize:
        LOG_NOT_IMPLEMENTED(Kernel, "TotalNonSystemMemorySize");
        // HACK
        out_info = 2ull * 1024ull * 1024ull * 1024ull;
        return RESULT_SUCCESS;
    case InfoType::UsedNonSystemMemorySize:
        LOG_NOT_IMPLEMENTED(Kernel, "UsedNonSystemMemorySize");
        // HACK
        out_info = 1;
        return RESULT_SUCCESS;
    case InfoType::IsApplication:
        // TODO: don't always return true
        out_info = static_cast<u64>(true);
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

result_t Kernel::mapPhysicalMemory(Process* crnt_process, vaddr_t addr,
                                   u64 size) {
    LOG_DEBUG(Kernel,
              "MapPhysicalMemory called (addr: 0x{:08x}, size: 0x{:08x})", addr,
              size);

    if (!isAligned(size, hw::tegra_x1::cpu::GUEST_PAGE_SIZE))
        return MAKE_RESULT(Svc, 102); // Invalid address

    if (!isAligned(size, hw::tegra_x1::cpu::GUEST_PAGE_SIZE))
        return MAKE_RESULT(Svc, 101); // Invalid size

    if (!ALIAS_REGION.contains(ztd::Range<vaddr_t>::fromSize(addr, size)))
        return MAKE_RESULT(Svc, 110); // Invalid memory region

    auto mem = system.getCpu().allocateMemory(size);
    // TODO: keep track of the memory
    crnt_process->getMmu()->map(addr, mem,
                                {.type = MemoryType::Alias,
                                 .attr = MemoryAttribute::None,
                                 .perm = MemoryPermission::ReadWrite});

    return RESULT_SUCCESS;
}

result_t Kernel::setThreadActivity(IThread* thread, ThreadActivity activity) {
    LOG_DEBUG(Kernel, "SetThreadActivity called (thread: {}, activity: {})",
              thread->getDebugName(), activity);

    // TODO: implement
    LOG_FUNC_WITH_ARGS_STUBBED(Kernel, "thread: {}, activity: {}",
                               thread->getDebugName(), activity);

    return RESULT_SUCCESS;
}

result_t Kernel::getThreadContext3(IThread* thread,
                                   ThreadContext& out_thread_context) {
    LOG_DEBUG(Kernel, "SetThreadActivity called (thread: {})",
              thread->getDebugName());

    // TODO: implement
    LOG_FUNC_WITH_ARGS_STUBBED(Kernel, "thread: {}", thread->getDebugName());

    // HACK
    out_thread_context = {};
    return RESULT_SUCCESS;
}

result_t Kernel::waitForAddress(IThread* crnt_thread, uptr addr,
                                ArbitrationType arbitration_type, u32 value,
                                i64 timeout) {
    LOG_DEBUG(Kernel,
              "WaitForAddress called (addr: 0x{:08x}, type: {}, value: "
              "0x{:x}, timeout: 0x{:08x})",
              addr, arbitration_type, value, timeout);

    bool wait;
    {
        CriticalSectionLock cs_lock(*this);

        auto value_ptr = reinterpret_cast<u32*>(addr);
        u32 current_value;
        switch (arbitration_type) {
        case ArbitrationType::WaitIfLessThan:
            current_value = atomicLoad(value_ptr);
            wait = (current_value < value);
            break;
        case ArbitrationType::DecrementAndWaitIfLessThan:
            current_value = atomicFetchSub(value_ptr, 1u);
            wait = (current_value < value);
            break;
        case ArbitrationType::WaitIfEqual:
            current_value = atomicLoad(value_ptr);
            wait = (current_value == value);
            break;
        }

        if (wait) {
            crnt_thread->pause();

            crnt_thread->mutex_wait_addr = addr;
            ASSERT_DEBUG(arbiters.addLast(crnt_thread).has_value(), Kernel,
                         "Failed to add arbiter");
        }
    }

    if (wait) {
        crnt_thread->processMessages(timeout);

        result_t res = RESULT_SUCCESS;
        if (crnt_thread->processMessages(timeout)) {
            if (crnt_thread->getState() == ThreadState::Stopping) {
                // TODO: is this necessary?
                res = MAKE_RESULT(Svc, Error::Cancelled);
            } else {
                if (!crnt_thread->wasSignalled())
                    res = MAKE_RESULT(Svc, Error::Cancelled);
            }
        } else {
            res = MAKE_RESULT(Svc, Error::TimedOut);
        }

        // Removal is done by the signalling thread
        /*
        {
            CriticalSectionLock cs_lock(*this);
            arbiters.remove(crnt_thread);
        }
        */

        return res;
    }

    return RESULT_SUCCESS;
}

result_t Kernel::signalToAddress(uptr addr, SignalType signal_type, u32 value,
                                 u32 count) {
    LOG_DEBUG(Kernel,
              "SignalToAddress called (addr: 0x{:08x}, "
              "signal_type: {}, value: 0x{:08x}, count: {})",
              addr, signal_type, value, count);

    // TODO: handle other signal types as well
    if (signal_type != SignalType::Signal)
        LOG_WARN(Kernel, "Unimplemented signal type {}", signal_type);
    (void)value;
    (void)count;

    CriticalSectionLock cs_lock(*this);
    for (auto waiter_node = arbiters.getHead(); waiter_node.has_value();) {
        const auto waiter_node_ = waiter_node.value();
        auto waiter = waiter_node_->get();
        if (waiter->mutex_wait_addr != addr) {
            waiter_node = waiter_node_->getNext();
            continue;
        }

        waiter->resume();
        waiter_node = arbiters.remove(waiter_node_);
    }

    return RESULT_SUCCESS;
}

void Kernel::synchronizePreemptionState(IThread* crnt_thread) {
    (void)crnt_thread;

    LOG_FUNC_STUBBED(Kernel);
}

result_t Kernel::createSession(bool is_light, u64 name,
                               hipc::ServerSession*& out_server_session,
                               hipc::ClientSession*& out_client_session) {
    LOG_DEBUG(Kernel, "CreateSession called (is_light: {}, name: 0x{:08x})",
              is_light, name);

    // TODO: what are light sessions?
    (void)is_light;

    // TODO: what's the purpose of the name?
    (void)name;

    out_server_session = new hipc::ServerSession();
    out_client_session = new hipc::ClientSession();
    // TODO: is it fine to just instantiate it like this?
    new hipc::Session(out_server_session, out_client_session);

    return RESULT_SUCCESS;
}

result_t Kernel::acceptSession(hipc::ServerPort* server_port,
                               hipc::ServerSession*& out_server_session) {
    LOG_DEBUG(Kernel, "AcceptSession called (port: {})",
              server_port->getDebugName());

    out_server_session = server_port->acceptSession();

    return RESULT_SUCCESS;
}

result_t Kernel::replyAndReceive(IThread* crnt_thread,
                                 std::span<SynchronizationObject*> sync_objs,
                                 hipc::ServerSession* reply_target_session,
                                 i64 timeout, u32& out_signalled_index) {
    LOG_DEBUG(Kernel, "ReplyAndReceive called (count: {}, timeout: {})",
              sync_objs.size(), timeout);

    if (reply_target_session != nullptr) {
        // Reply
        reply_target_session->reply(crnt_thread->getTlsPtr());
    }

    // Wait
    const auto res = waitSynchronization(crnt_thread, sync_objs, timeout,
                                         out_signalled_index);
    if (res != RESULT_SUCCESS)
        return res;

    auto sync_obj = sync_objs[static_cast<usize>(out_signalled_index)];
    if (sync_obj->isOfType<hipc::ServerSession>()) {
        auto server_session = static_cast<hipc::ServerSession*>(sync_obj);
        if (server_session->isClientOpen()) {
            // Receive
            server_session->receive(crnt_thread);
            return RESULT_SUCCESS;
        } else {
            return MAKE_RESULT(Svc, 123); // SessionClosed
        }
    } else {
        return RESULT_SUCCESS;
    }
}

result_t Kernel::createCodeMemory(vaddr_t addr, u64 size,
                                  CodeMemory*& out_code_memory) {
    LOG_DEBUG(Kernel, "CreateCodeMemory called (addr: 0x{:08x}, size: {})",
              addr, size);

    out_code_memory = new CodeMemory(addr, size);

    return RESULT_SUCCESS;
}

result_t Kernel::controlCodeMemory(CodeMemory* code_memory,
                                   CodeMemoryOperation op, vaddr_t addr,
                                   u64 size, MemoryPermission perm) {
    LOG_DEBUG(Kernel,
              "ControlCodeMemory called (code memory: {}, op: {}, addr: "
              "0x{:08x}, size: {}, perm: {})",
              code_memory->getDebugName(), op, addr, size, perm);

    // TODO: implement
    LOG_FUNC_WITH_ARGS_STUBBED(
        Kernel,
        "code memory: {}, op: {}, addr: 0x{:08x}, size: 0x{:08x}, perm: {}",
        code_memory->getDebugName(), op, addr, size, perm);

    return RESULT_SUCCESS;
}

result_t Kernel::getProcessList(u64* process_id_buffer,
                                u32 process_id_buffer_size, u32& out_count) {
    LOG_DEBUG(Kernel,
              "GetProcessList called (process ID buffer: {}, "
              "process ID buffer size: {})",
              reinterpret_cast<void*>(process_id_buffer),
              process_id_buffer_size);

    // TODO: is process_id_buffer_size in bytes or number of process IDs?
    for (auto it = process_manager.begin();
         it != process_manager.end() && process_id_buffer_size >= sizeof(u64);
         it++) {
        // TODO: what is a process ID?
        *process_id_buffer++ = reinterpret_cast<u64>(*it); // HACK
        process_id_buffer_size -= sizeof(u64);
        out_count++;
    }

    return RESULT_SUCCESS;
}

result_t Kernel::setProcessMemoryPermission(Process* process, vaddr_t addr,
                                            u64 size, MemoryPermission perm) {
    LOG_DEBUG(Kernel,
              "SetProcessMemoryPermission called (process: {}, addr: 0x{:08x}, "
              "size: {}, perm: {})",
              process->getDebugName(), addr, size, perm);

    // TODO: implement
    (void)process;
    LOG_FUNC_WITH_ARGS_STUBBED(
        Kernel, "addr: 0x{:08x}, size: 0x{:08x}, perm: {}", addr, size, perm);

    return RESULT_SUCCESS;
}

result_t Kernel::mapProcessMemory(Process* crnt_process, vaddr_t dst_addr,
                                  Process* process, vaddr_t src_addr,
                                  u64 size) {
    LOG_DEBUG(Kernel,
              "MapProcessMemory called (crnt_process: {}, dst_addr: 0x{:08x}, "
              "process: {}, src_addr: 0x{:08x}, size: {})",
              crnt_process->getDebugName(), dst_addr, process->getDebugName(),
              src_addr, size);

    // TODO: correct?
    const auto ptr = process->getMmu()->unmapAddr(src_addr);
    crnt_process->getMmu()->map(dst_addr, ztd::Range<uptr>::fromSize(ptr, size),
                                {}); // TODO: state

    return RESULT_SUCCESS;
}

result_t Kernel::mapProcessCodeMemory(Process* process, vaddr_t dst_addr,
                                      vaddr_t src_addr, u64 size) {
    LOG_DEBUG(Kernel,
              "MapProcessCodeMemory called (process: {}, dst_addr: 0x{:08x}, "
              "src_addr: 0x{:08x}, size: {})",
              process->getDebugName(), dst_addr, src_addr, size);

    process->getMmu()->map(dst_addr,
                           ztd::Range<vaddr_t>::fromSize(src_addr, size));

    return RESULT_SUCCESS;
}

result_t Kernel::unmapProcessCodeMemory(Process* process, vaddr_t dst_addr,
                                        vaddr_t src_addr, u64 size) {
    LOG_DEBUG(Kernel,
              "UnmapProcessCodeMemory called (process: {}, dst_addr: 0x{:08x}, "
              "src_addr: 0x{:08x}, size: {})",
              process->getDebugName(), dst_addr, src_addr, size);

    // TODO: verify that src_addr is the same as the one used in MapMemory?
    (void)src_addr;

    process->getMmu()->unmap(ztd::Range<vaddr_t>::fromSize(dst_addr, size));

    return RESULT_SUCCESS;
}

void Kernel::tryAcquireMutex(Process* crnt_process, IThread* thread) {
    auto mutex = reinterpret_cast<u32*>(thread->mutex_wait_addr);

    u32 value = *mutex;
    u32 new_value;
    do {
        if (value == 0) {
            // Register this thread as the owner
            new_value = thread->self_handle_for_mutex.getRaw();
        } else {
            // Register this thread as a waiter
            new_value = value | MUTEX_WAIT_MASK;
        }
    } while (!atomicCompareExchangeWeak(mutex, value, new_value));

    if (value == 0) {
        // Mutex acquired
        thread->mutex_wait_addr = 0x0;
        thread->resume();
        return;
    }

    // Register this thread as a waiter by the owner
    auto owner = getMutexOwner(crnt_process, value).value();
    owner->addMutexWaiter(thread);
}

void Kernel::unlockMutex(IThread* thread, uptr mutex_addr) {
    auto mutex = reinterpret_cast<u32*>(mutex_addr);

    u32 waiter_count;
    auto new_owner = thread->relinquishMutex(mutex_addr, waiter_count);
    if (new_owner == nullptr) {
        atomicStore(mutex, 0u);
        return;
    }

    u32 value = new_owner->self_handle_for_mutex.getRaw();
    if (waiter_count > 0)
        value |= MUTEX_WAIT_MASK;

    atomicStore(mutex, value);

    // Resume the owner
    new_owner->resume();
}

} // namespace hydra::horizon::kernel
