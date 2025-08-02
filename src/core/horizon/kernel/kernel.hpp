#pragma once

#include "core/horizon/filesystem/filesystem.hpp"
#include "core/horizon/kernel/event.hpp"
#include "core/horizon/kernel/hipc/service_manager.hpp"
#include "core/horizon/kernel/mutex.hpp"
#include "core/horizon/kernel/process_manager.hpp"
#include "core/horizon/kernel/shared_memory.hpp"
#include "core/horizon/kernel/transfer_memory.hpp"

#define KERNEL_INSTANCE hydra::horizon::kernel::Kernel::GetInstance()

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
    static Kernel& GetInstance();

    Kernel();
    ~Kernel();

    void SupervisorCall(Process* crnt_process, IThread* crnt_thread,
                        hw::tegra_x1::cpu::IThread* guest_thread, u64 id);

    // SVCs
    result_t SetHeapSize(Process* crnt_process, usize size, uptr& out_base);
    result_t SetMemoryPermission(uptr addr, usize size, MemoryPermission perm);
    result_t SetMemoryAttribute(uptr addr, usize size, u32 mask, u32 value);
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
    result_t CreateTransferMemory(Process* crnt_process, uptr addr, u64 size,
                                  MemoryPermission perm,
                                  TransferMemory*& out_tmem);
    result_t CloseHandle(Process* crnt_process, handle_id_t handle_id);
    result_t ResetSignal(SynchronizationObject* sync_object);
    result_t WaitSynchronization(IThread* crnt_thread,
                                 std::span<SynchronizationObject*> sync_objects,
                                 i64 timeout, i32& out_signalled_index);
    result_t CancelSynchronization(IThread* thread);
    result_t ArbitrateLock(Process* crnt_process, u32 wait_tag, uptr mutex_addr,
                           u32 self_tag);
    result_t ArbitrateUnlock(Process* crnt_process, uptr mutex_addr);
    result_t WaitProcessWideKeyAtomic(Process* crnt_process, uptr mutex_addr,
                                      uptr var_addr, u32 self_tag, i64 timeout);
    result_t SignalProcessWideKey(uptr addr, i32 count);
    void GetSystemTick(u64& out_tick);
    result_t ConnectToNamedPort(const std::string_view name,
                                hipc::ClientSession*& out_client_session);
    result_t SendSyncRequest(Process* crnt_process, IThread* crnt_thread,
                             hw::tegra_x1::cpu::IMemory* tls_mem,
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
    result_t WaitForAddress(Process* crnt_process, vaddr_t addr,
                            ArbitrationType arbitration_type, u32 value,
                            u64 timeout);
    result_t CreateSession(bool is_light, u64 name,
                           hipc::ServerSession*& out_server_session,
                           hipc::ClientSession*& out_client_session);
    result_t AcceptSession(hipc::ServerPort* server_port,
                           hipc::ServerSession*& out_server_session);
    // TODO: handles can only be Port or ServerSession
    result_t ReplyAndReceive(IThread* crnt_thread,
                             std::span<SynchronizationObject*> sync_objs,
                             hipc::ServerSession* reply_target_session,
                             i64 timeout, i32& out_signalled_index);
    result_t CreateCodeMemory(vaddr_t addr, u64 size,
                              CodeMemory*& out_code_memory);
    result_t ControlCodeMemory(CodeMemory* code_memory, CodeMemoryOperation op,
                               vaddr_t addr, u64 size, MemoryPermission perm);
    result_t SetProcessMemoryPermission(Process* process, vaddr_t addr,
                                        u64 size, MemoryPermission perm);
    result_t MapProcessCodeMemory(Process* process, vaddr_t dst_addr,
                                  vaddr_t src_addr, u64 size);
    result_t UnmapProcessCodeMemory(Process* process, vaddr_t dst_addr,
                                    vaddr_t src_addr, u64 size);

  private:
    filesystem::Filesystem filesystem;
    ProcessManager process_manager;

    // Services
    hipc::ServiceManager<std::string> service_manager;

    std::mutex sync_mutex;
    // TODO: use a different container?
    std::map<vaddr_t, Mutex> mutex_map;
    std::map<vaddr_t, std::condition_variable> cond_var_map;

  public:
    REF_GETTER(process_manager, GetProcessManager);
    REF_GETTER(service_manager, GetServiceManager);
};

} // namespace hydra::horizon::kernel
