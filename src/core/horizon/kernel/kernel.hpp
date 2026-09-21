#pragma once

#include "core/horizon/kernel/applet_resource.hpp"
#include "core/horizon/kernel/event.hpp"
#include "core/horizon/kernel/hipc/service_manager.hpp"
#include "core/horizon/kernel/process_manager.hpp"
#include "core/horizon/kernel/shared_memory.hpp"
#include "core/horizon/kernel/transfer_memory.hpp"

namespace hydra::hw::tegra_x1::cpu {
class IMmu;
class IThread;
} // namespace hydra::hw::tegra_x1::cpu

// TODO: remove this
namespace hydra::horizon::services {
class IService;
}

namespace hydra::horizon::kernel::hipc {
class ServerPort;
class ServerSession;
class ClientSession;
class Session;
} // namespace hydra::horizon::kernel::hipc

namespace hydra::horizon::kernel {

class IThread;
class Process;
class CodeMemory;

class Kernel {
  public:
    explicit Kernel(System& system_);

    void supervisorCall(Process* crnt_process, IThread* crnt_thread,
                        hw::tegra_x1::cpu::IThread* guest_thread, u64 id);

    AppletResourceUserId allocateAppletResourceUserId() {
        for (u32 i = 0; i < MAX_APPLET_RESOURCES; i++) {
            auto& is_free = free_applet_resource_user_ids[i];
            if (is_free) {
                is_free = false;
                return toAruid(i);
            }
        }

        LOG_FATAL(Kernel, "Out of applet resource user IDs");
    }

    void releaseAppletResourceUserId(AppletResourceUserId aruid) {
        const auto index = toIndex(aruid);
        ASSERT(!free_applet_resource_user_ids[index], Kernel,
               "Invalid aruid {:#x}", aruid);
        free_applet_resource_user_ids[index] = true;
    }

    // SVCs
    result_t setHeapSize(Process* crnt_process, u64 size, uptr& out_base);
    result_t setMemoryPermission(uptr addr, u64 size, MemoryPermission perm);
    result_t setMemoryAttribute(Process* crnt_process, vaddr_t addr, u64 size,
                                MemoryAttribute mask, MemoryAttribute value);
    result_t mapMemory(Process* crnt_process, uptr dst_addr, uptr src_addr,
                       u64 size);
    result_t unmapMemory(Process* crnt_process, uptr dst_addr, uptr src_addr,
                         u64 size);
    result_t queryMemory(Process* crnt_process, uptr addr,
                         MemoryInfo& out_mem_info, u32& out_page_info);
    void exitProcess(Process* crnt_process);
    result_t createThread(Process* crnt_process, vaddr_t entry_point,
                          vaddr_t args_addr, vaddr_t stack_top_addr,
                          i32 priority, i32 processor_id, IThread*& out_thread);
    result_t startThread(IThread* thread);
    void exitThread(IThread* crnt_thread);
    void sleepThread(i64 nano);
    result_t getThreadPriority(IThread* thread, i32& out_priority);
    result_t setThreadPriority(IThread* thread, i32 priority);
    result_t getThreadCoreMask(IThread* thread, i32& out_core_mask0,
                               u64& out_core_mask1);
    result_t setThreadCoreMask(IThread* thread, i32 core_mask0, u64 core_mask1);
    void getCurrentProcessorNumber(u32& out_number);
    result_t signalEvent(Event* event);
    result_t clearEvent(Event* event);
    result_t mapSharedMemory(Process* crnt_process, SharedMemory* shmem,
                             uptr addr, u64 size, MemoryPermission perm);
    result_t unmapSharedMemory(Process* crnt_process, SharedMemory* shmem,
                               uptr addr, u64 size);
    result_t createTransferMemory(uptr addr, u64 size, MemoryPermission perm,
                                  TransferMemory*& out_tmem);
    result_t closeHandle(Process* crnt_process, Handle handle);
    result_t resetSignal(SynchronizationObject* sync_object);
    result_t waitSynchronization(IThread* crnt_thread,
                                 std::span<SynchronizationObject*> sync_objs,
                                 i64 timeout, u32& out_signalled_index);
    result_t cancelSynchronization(IThread* thread);
    result_t arbitrateLock(IThread* crnt_thread, IThread* owner_thread,
                           uptr mutex_addr, Handle self_handle,
                           Handle owner_handle);
    result_t arbitrateUnlock(IThread* crnt_thread, uptr mutex_addr);
    result_t waitProcessWideKeyAtomic(Process* crnt_process,
                                      IThread* crnt_thread, uptr mutex_addr,
                                      uptr var_addr, Handle self_handle,
                                      i64 timeout);
    result_t signalProcessWideKey(Process* crnt_process, uptr addr, i32 count);
    void getSystemTick(u64& out_tick);
    result_t connectToNamedPort(const std::string_view name,
                                hipc::ClientSession*& out_client_session);
    result_t sendSyncRequest(Process* crnt_process, IThread* crnt_thread,
                             hipc::ClientSession* client_session);
    result_t getThreadId(IThread* thread, u64& out_thread_id);
    result_t break_(BreakReason reason, uptr buffer_ptr, u64 buffer_size);
    result_t outputDebugString(const std::string_view str, u64 len);
    result_t getInfo(Process* crnt_process, InfoType info_type, AutoObject* obj,
                     u64 info_sub_type, u64& out_info);
    result_t mapPhysicalMemory(Process* crnt_process, vaddr_t addr, u64 size);
    result_t setThreadActivity(IThread* thread, ThreadActivity activity);
    result_t getThreadContext3(IThread* thread,
                               ThreadContext& out_thread_context);
    result_t waitForAddress(IThread* crnt_thread, uptr addr,
                            ArbitrationType arbitration_type, u32 value,
                            i64 timeout);
    result_t signalToAddress(uptr addr, SignalType signal_type, u32 value,
                             u32 count);
    void synchronizePreemptionState(IThread* crnt_thread);
    result_t createSession(bool is_light, u64 name,
                           hipc::ServerSession*& out_server_session,
                           hipc::ClientSession*& out_client_session);
    result_t acceptSession(hipc::ServerPort* server_port,
                           hipc::ServerSession*& out_server_session);
    // TODO: handles can only be Port or ServerSession
    result_t replyAndReceive(IThread* crnt_thread,
                             std::span<SynchronizationObject*> sync_objs,
                             hipc::ServerSession* reply_target_session,
                             i64 timeout, u32& out_signalled_index);
    result_t createCodeMemory(vaddr_t addr, u64 size,
                              CodeMemory*& out_code_memory);
    result_t controlCodeMemory(CodeMemory* code_memory, CodeMemoryOperation op,
                               vaddr_t addr, u64 size, MemoryPermission perm);
    result_t getProcessList(u64* process_id_buffer, u32 process_id_buffer_size,
                            u32& out_count);
    result_t setProcessMemoryPermission(Process* process, vaddr_t addr,
                                        u64 size, MemoryPermission perm);
    result_t mapProcessMemory(Process* crnt_process, vaddr_t dst_addr,
                              Process* process, vaddr_t src_addr, u64 size);
    result_t mapProcessCodeMemory(Process* process, vaddr_t dst_addr,
                                  vaddr_t src_addr, u64 size);
    result_t unmapProcessCodeMemory(Process* process, vaddr_t dst_addr,
                                    vaddr_t src_addr, u64 size);

  private:
    System& system;

    ProcessManager process_manager;
    hipc::ServiceManager<std::string> service_manager;

    // Critical section
    std::mutex critical_section_mutex;

    // Sync
    ztd::DoublyLinkedList<IThread*> cond_var_waiters;
    ztd::DoublyLinkedList<IThread*> arbiters;

    // Applet resource
    std::array<bool, MAX_APPLET_RESOURCES> free_applet_resource_user_ids = {
        true};

    // Helpers
    static void tryAcquireMutex(Process* crnt_process, IThread* thread);
    static void unlockMutex(IThread* thread, uptr mutex_addr);

  public:
    REF_GETTER(process_manager, getProcessManager);
    REF_GETTER(service_manager, getServiceManager);
    REF_GETTER(critical_section_mutex, getCriticalSectionMutex);
};

class CriticalSectionLock {
  public:
    explicit CriticalSectionLock(Kernel& kernel_) : kernel{kernel_} {
        kernel.getCriticalSectionMutex().lock();
    }

    ~CriticalSectionLock() { kernel.getCriticalSectionMutex().unlock(); }

  private:
    Kernel& kernel;
};

} // namespace hydra::horizon::kernel
