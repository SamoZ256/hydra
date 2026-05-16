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
    Kernel(System& system_);

    void SupervisorCall(Process* crnt_process, IThread* crnt_thread,
                        hw::tegra_x1::cpu::IThread* guest_thread, u64 id);

    enum class AllocateAppletResourceUserIdError {
        OutOfIds,
    };
    AppletResourceUserId AllocateAppletResourceUserId() {
        for (u32 i = 0; i < MAX_APPLET_RESOURCES; i++) {
            auto& is_free = free_applet_resource_user_ids[i];
            if (is_free) {
                is_free = false;
                return ToAruid(i);
            }
        }

        throw AllocateAppletResourceUserIdError::OutOfIds;
    }

    enum class ReleaseAppletResourceUserIdError {
        InvalidAruid,
    };
    void ReleaseAppletResourceUserId(AppletResourceUserId aruid) {
        const auto index = ToIndex(aruid);
        ASSERT_THROWING(!free_applet_resource_user_ids[index], Kernel,
                        ReleaseAppletResourceUserIdError::InvalidAruid,
                        "Invalid aruid {:#x}", aruid);
        free_applet_resource_user_ids[index] = true;
    }

    // SVCs
    result_t SetHeapSize(Process* crnt_process, usize size, uptr& out_base);
    result_t SetMemoryPermission(uptr addr, usize size, MemoryPermission perm);
    result_t SetMemoryAttribute(Process* crnt_process, vaddr_t addr, usize size,
                                MemoryAttribute mask, MemoryAttribute value);
    result_t MapMemory(Process* crnt_process, uptr dst_addr, uptr src_addr,
                       usize size);
    result_t UnmapMemory(Process* crnt_process, uptr dst_addr, uptr src_addr,
                         usize size);
    result_t QueryMemory(Process* crnt_process, uptr addr,
                         MemoryInfo& out_mem_info, u32& out_page_info);
    void ExitProcess(Process* crnt_process);
    result_t CreateThread(Process* crnt_process, vaddr_t entry_point,
                          vaddr_t args_addr, vaddr_t stack_top_addr,
                          i32 priority, i32 processor_id, IThread*& out_thread);
    result_t StartThread(IThread* thread);
    void ExitThread(IThread* crnt_thread);
    void SleepThread(i64 nano);
    result_t GetThreadPriority(IThread* thread, i32& out_priority);
    result_t SetThreadPriority(IThread* thread, i32 priority);
    result_t GetThreadCoreMask(IThread* thread, i32& out_core_mask0,
                               u64& out_core_mask1);
    result_t SetThreadCoreMask(IThread* thread, i32 core_mask0, u64 core_mask1);
    void GetCurrentProcessorNumber(u32& out_number);
    result_t SignalEvent(Event* event);
    result_t ClearEvent(Event* event);
    result_t MapSharedMemory(Process* crnt_process, SharedMemory* shmem,
                             uptr addr, usize size, MemoryPermission perm);
    result_t UnmapSharedMemory(Process* crnt_process, SharedMemory* shmem,
                               uptr addr, usize size);
    result_t CreateTransferMemory(uptr addr, u64 size, MemoryPermission perm,
                                  TransferMemory*& out_tmem);
    result_t CloseHandle(Process* crnt_process, handle_id_t handle_id);
    result_t ResetSignal(SynchronizationObject* sync_object);
    result_t WaitSynchronization(IThread* crnt_thread,
                                 std::span<SynchronizationObject*> sync_objects,
                                 i64 timeout, u32& out_signalled_index);
    result_t CancelSynchronization(IThread* thread);
    result_t ArbitrateLock(IThread* crnt_thread, IThread* owner_thread,
                           uptr mutex_addr, handle_id_t self_handle,
                           handle_id_t owner_handle);
    result_t ArbitrateUnlock(IThread* crnt_thread, uptr mutex_addr);
    result_t WaitProcessWideKeyAtomic(Process* crnt_process,
                                      IThread* crnt_thread, uptr mutex_addr,
                                      uptr var_addr, handle_id_t self_handle,
                                      i64 timeout);
    result_t SignalProcessWideKey(Process* crnt_process, uptr addr, i32 count);
    void GetSystemTick(u64& out_tick);
    result_t ConnectToNamedPort(const std::string_view name,
                                hipc::ClientSession*& out_client_session);
    result_t SendSyncRequest(Process* crnt_process, IThread* crnt_thread,
                             hipc::ClientSession* client_session);
    result_t GetThreadId(IThread* thread, u64& out_thread_id);
    result_t Break(BreakReason reason, uptr buffer_ptr, usize buffer_size);
    result_t OutputDebugString(const std::string_view str, usize len);
    result_t GetInfo(Process* crnt_process, InfoType info_type, AutoObject* obj,
                     u64 info_sub_type, u64& out_info);
    result_t MapPhysicalMemory(Process* crnt_process, vaddr_t addr, usize size);
    result_t SetThreadActivity(IThread* thread, ThreadActivity activity);
    result_t GetThreadContext3(IThread* thread,
                               ThreadContext& out_thread_context);
    result_t WaitForAddress(IThread* crnt_thread, uptr addr,
                            ArbitrationType arbitration_type, u32 value,
                            i64 timeout);
    result_t SignalToAddress(uptr addr, SignalType signal_type, u32 value,
                             u32 count);
    void SynchronizePreemptionState(IThread* crnt_thread);
    result_t CreateSession(bool is_light, u64 name,
                           hipc::ServerSession*& out_server_session,
                           hipc::ClientSession*& out_client_session);
    result_t AcceptSession(hipc::ServerPort* server_port,
                           hipc::ServerSession*& out_server_session);
    // TODO: handles can only be Port or ServerSession
    result_t ReplyAndReceive(IThread* crnt_thread,
                             std::span<SynchronizationObject*> sync_objs,
                             hipc::ServerSession* reply_target_session,
                             i64 timeout, u32& out_signalled_index);
    result_t CreateCodeMemory(vaddr_t addr, u64 size,
                              CodeMemory*& out_code_memory);
    result_t ControlCodeMemory(CodeMemory* code_memory, CodeMemoryOperation op,
                               vaddr_t addr, u64 size, MemoryPermission perm);
    result_t GetProcessList(u64* process_id_buffer, u32 process_id_buffer_size,
                            u32& out_count);
    result_t SetProcessMemoryPermission(Process* process, vaddr_t addr,
                                        u64 size, MemoryPermission perm);
    result_t MapProcessMemory(Process* crnt_process, vaddr_t dst_addr,
                              Process* process, vaddr_t src_addr, u64 size);
    result_t MapProcessCodeMemory(Process* process, vaddr_t dst_addr,
                                  vaddr_t src_addr, u64 size);
    result_t UnmapProcessCodeMemory(Process* process, vaddr_t dst_addr,
                                    vaddr_t src_addr, u64 size);

  private:
    System& system;

    ProcessManager process_manager;
    hipc::ServiceManager<std::string> service_manager;

    // Critical section
    std::mutex critical_section_mutex;

    // Sync
    DoubleLinkedList<IThread*> cond_var_waiters;
    DoubleLinkedList<IThread*> arbiters;

    // Applet resource
    std::array<bool, MAX_APPLET_RESOURCES> free_applet_resource_user_ids = {
        true};

    // Helpers
    void TryAcquireMutex(Process* crnt_process, IThread* thread);
    void UnlockMutex(IThread* thread, uptr mutex_addr);

  public:
    REF_GETTER(process_manager, GetProcessManager);
    REF_GETTER(service_manager, GetServiceManager);
    REF_GETTER(critical_section_mutex, GetCriticalSectionMutex);
};

class CriticalSectionLock {
  public:
    CriticalSectionLock(Kernel& kernel_) : kernel{kernel_} {
        kernel.GetCriticalSectionMutex().lock();
    }

    ~CriticalSectionLock() { kernel.GetCriticalSectionMutex().unlock(); }

  private:
    Kernel& kernel;
};

} // namespace hydra::horizon::kernel
