#pragma once

#include "core/horizon/filesystem/filesystem.hpp"
#include "core/horizon/kernel/event.hpp"
#include "core/horizon/kernel/handle_pool.hpp"
#include "core/horizon/kernel/mutex.hpp"
#include "core/horizon/kernel/process_manager.hpp"
#include "core/horizon/kernel/shared_memory.hpp"
#include "core/horizon/kernel/transfer_memory.hpp"

#define KERNEL_INSTANCE hydra::horizon::kernel::Kernel::GetInstance()

namespace hydra::hw::tegra_x1::cpu {
class IMmu;
class IThread;
} // namespace hydra::hw::tegra_x1::cpu

namespace hydra::hw {
class Bus;
}

namespace hydra::horizon::kernel {

class ServiceBase;
class Thread;
class Process;

class Kernel {
  public:
    static Kernel& GetInstance();

    Kernel();
    ~Kernel();

    void ConnectServiceToPort(const std::string& port_name,
                              ServiceBase* service) {
        service_ports[std::string(port_name)] = service;
    }

    void SupervisorCall(Process* process, Thread* thread,
                        hw::tegra_x1::cpu::IThread* guest_thread, u64 id);

    // SVCs
    result_t svcSetHeapSize(Process* process, usize size, uptr& out_base);
    result_t svcSetMemoryPermission(uptr addr, usize size,
                                    MemoryPermission perm);
    result_t svcSetMemoryAttribute(uptr addr, usize size, u32 mask, u32 value);
    result_t svcMapMemory(Process* process, uptr dst_addr, uptr src_addr,
                          usize size);
    result_t svcUnmapMemory(Process* process, uptr dst_addr, uptr src_addr,
                            usize size);
    result_t svcQueryMemory(Process* process, uptr addr,
                            MemoryInfo& out_mem_info, u32& out_page_info);
    void svcExitProcess(Process* process);
    result_t svcCreateThread(Process* process, vaddr_t entry_point,
                             vaddr_t args_addr, vaddr_t stack_top_addr,
                             i32 priority, i32 processor_id,
                             handle_id_t& out_thread_handle_id);
    result_t svcStartThread(Process* process, handle_id_t thread_handle_id);
    void svcExitThread(Thread* thread);
    void svcSleepThread(i64 nano);
    result_t svcGetThreadPriority(Process* process,
                                  handle_id_t thread_handle_id,
                                  i32& out_priority);
    result_t svcSetThreadPriority(Process* process,
                                  handle_id_t thread_handle_id, i32 priority);
    result_t svcGetThreadCoreMask(Process* process,
                                  handle_id_t thread_handle_id,
                                  i32& out_core_mask0, u64& out_core_mask1);
    result_t svcSetThreadCoreMask(Process* process,
                                  handle_id_t thread_handle_id, i32 core_mask0,
                                  u64 core_mask1);
    void svcGetCurrentProcessorNumber(u32& out_number);
    result_t svcSignalEvent(Process* process, handle_id_t event_handle_id);
    result_t svcClearEvent(Process* process, handle_id_t event_handle_id);
    result_t svcMapSharedMemory(Process* process,
                                handle_id_t shared_mem_handle_id, uptr addr,
                                usize size, MemoryPermission perm);
    result_t svcUnmapSharedMemory(Process* process,
                                  handle_id_t shared_mem_handle_id, uptr addr,
                                  usize size);
    result_t svcCreateTransferMemory(Process* process, uptr addr, u64 size,
                                     MemoryPermission perm,
                                     handle_id_t& out_transfer_mem_handle_id);
    result_t svcCloseHandle(Process* process, handle_id_t handle_id);
    result_t svcResetSignal(Process* process, handle_id_t handle_id);
    result_t svcWaitSynchronization(Process* process, handle_id_t* handle_ids,
                                    i32 handle_count, i64 timeout,
                                    u64& out_handle_index);
    result_t svcCancelSynchronization(Process* process,
                                      handle_id_t thread_handle_id);
    result_t svcArbitrateLock(Process* process, u32 wait_tag, uptr mutex_addr,
                              u32 self_tag);
    result_t svcArbitrateUnlock(Process* process, uptr mutex_addr);
    result_t svcWaitProcessWideKeyAtomic(Process* process, uptr mutex_addr,
                                         uptr var_addr, u32 self_tag,
                                         i64 timeout);
    result_t svcSignalProcessWideKey(uptr addr, i32 count);
    void svcGetSystemTick(u64& out_tick);
    result_t svcConnectToNamedPort(Process* process, const std::string& name,
                                   handle_id_t& out_session_handle_id);
    result_t svcSendSyncRequest(Process* process,
                                hw::tegra_x1::cpu::IMemory* tls_mem,
                                handle_id_t session_handle_id);
    result_t svcGetThreadId(Process* process, handle_id_t thread_handle_id,
                            u64& out_thread_id);
    result_t svcBreak(BreakReason reason, uptr buffer_ptr, usize buffer_size);
    result_t svcOutputDebugString(const char* str, usize len);
    result_t svcGetInfo(Process* process, InfoType info_type,
                        handle_id_t handle_id, u64 info_sub_type,
                        u64& out_info);
    result_t svcMapPhysicalMemory(Process* process, vaddr_t addr, usize size);
    result_t svcSetThreadActivity(Process* process,
                                  handle_id_t thread_handle_id,
                                  ThreadActivity activity);
    result_t svcGetThreadContext3(Process* process,
                                  handle_id_t thread_handle_id,
                                  ThreadContext& out_thread_context);
    result_t svcWaitForAddress(Process* process, vaddr_t addr,
                               ArbitrationType arbitration_type, u32 value,
                               u64 timeout);

  private:
    filesystem::Filesystem filesystem;
    ProcessManager process_manager;

    // Services
    std::map<std::string, ServiceBase*> service_ports;

    std::mutex sync_mutex;
    // TODO: use a different container?
    std::map<vaddr_t, Mutex> mutex_map;
    std::map<vaddr_t, std::condition_variable> cond_var_map;

  public:
    REF_GETTER(process_manager, GetProcessManager);
};

} // namespace hydra::horizon::kernel
