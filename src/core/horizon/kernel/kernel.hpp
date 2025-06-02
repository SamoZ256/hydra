#pragma once

#include "core/horizon/filesystem/filesystem.hpp"
#include "core/horizon/kernel/handle_pool.hpp"
#include "core/horizon/kernel/shared_memory.hpp"

#define KERNEL_INSTANCE hydra::horizon::kernel::Kernel::GetInstance()

namespace hydra::hw::tegra_x1::cpu {
class MMUBase;
class ThreadBase;
} // namespace hydra::hw::tegra_x1::cpu

namespace hydra::hw {
class Bus;
}

namespace hydra::horizon::kernel {

class ServiceBase;
class Thread;

// TODO: should this be used?
constexpr u32 HANDLE_WAIT_MASK = 0x40000000;

class Mutex {
  public:
    void Lock(u32& value, u32 self_tag) {
        std::unique_lock<std::mutex> lock(mutex);
        // TODO: why is this necessary?
        value = value | HANDLE_WAIT_MASK;
        cv.wait(lock, [&] { return (value & ~HANDLE_WAIT_MASK) == 0; });
        value = self_tag | (value & HANDLE_WAIT_MASK);
    }

    void Unlock(u32& value) {
        std::unique_lock<std::mutex> lock(mutex);
        value = (value & HANDLE_WAIT_MASK);
        cv.notify_one();
    }

    // Getters
    std::mutex& GetNativeHandle() { return mutex; }

  private:
    std::mutex mutex;
    std::condition_variable cv;
};

class Event : public Handle {
  public:
    Event(bool autoclear_ = false, bool signaled_ = false)
        : autoclear{autoclear_}, signaled{signaled_} {}

    void Signal() {
        std::unique_lock<std::mutex> lock(mutex);
        signaled = true;
        cv.notify_all();
    }

    bool Clear() {
        bool was_signaled;
        {
            std::unique_lock<std::mutex> lock(mutex);
            was_signaled = signaled;
            signaled = false;
        }

        return was_signaled;
    }

    // Returns true if the event was signaled, false on timeout
    bool Wait(i64 timeout = INFINITE_TIMEOUT) {
        std::unique_lock<std::mutex> lock(mutex);
        bool was_signaled = WaitImpl(lock, timeout);
        // TODO: correct?
        if (autoclear)
            signaled = false;

        return was_signaled;
    }

  private:
    std::mutex mutex;
    std::condition_variable cv;
    bool autoclear;
    bool signaled;

    bool WaitImpl(std::unique_lock<std::mutex>& lock, i64 timeout) {
        // First, check if the event is already signaled
        if (signaled)
            return true;

        if (timeout == INFINITE_TIMEOUT) {
            cv.wait(lock);
            return true;
        } else {
            const auto status =
                cv.wait_for(lock, std::chrono::nanoseconds(timeout));
            return (status == std::cv_status::no_timeout);
        }
    }
};

class TransferMemory : public Handle {
  public:
    TransferMemory(vaddr_t addr_, u64 size_, MemoryPermission perm_)
        : addr{addr_}, size{size_}, perm{perm_} {}

    vaddr_t GetAddress() const { return addr; }
    u64 GetSize() const { return size; }
    MemoryPermission GetPermission() const { return perm; }

  private:
    vaddr_t addr;
    u64 size;
    MemoryPermission perm;
};

class Kernel {
  public:
    static Kernel& GetInstance();

    Kernel(hw::Bus& bus_, hw::tegra_x1::cpu::MMUBase* mmu_);
    ~Kernel();

    // Loading
    uptr CreateRomMemory(usize size, MemoryType type, MemoryPermission perm,
                         bool add_guard_page, vaddr_t& out_base);
    // TODO: should the caller be able to specify permissions?
    uptr CreateExecutableMemory(usize size, MemoryPermission perm,
                                bool add_guard_page, vaddr_t& out_base);

    void ConnectServiceToPort(const std::string& port_name,
                              ServiceBase* service) {
        service_ports[std::string(port_name)] = service;
    }

    bool SupervisorCall(hw::tegra_x1::cpu::ThreadBase* thread, u64 id);

    // SVCs
    result_t svcSetHeapSize(usize size, uptr& out_base);
    result_t svcSetMemoryPermission(uptr addr, usize size,
                                    MemoryPermission perm);
    result_t svcSetMemoryAttribute(uptr addr, usize size, u32 mask, u32 value);
    result_t svcMapMemory(uptr dst_addr, uptr src_addr, usize size);
    result_t svcUnmapMemory(uptr dst_addr, uptr src_addr, usize size);
    result_t svcQueryMemory(uptr addr, MemoryInfo& out_mem_info,
                            u32& out_page_info);
    void svcExitProcess();
    result_t svcCreateThread(vaddr_t entry_point, vaddr_t args_addr,
                             vaddr_t stack_top_addr, i32 priority,
                             i32 processor_id,
                             handle_id_t& out_thread_handle_id);
    result_t svcStartThread(handle_id_t thread_handle_id);
    void svcExitThread();
    void svcSleepThread(i64 nano);
    result_t svcGetThreadPriority(handle_id_t thread_handle_id,
                                  i32& out_priority);
    result_t svcSetThreadPriority(handle_id_t thread_handle_id, i32 priority);
    result_t svcGetThreadCoreMask(handle_id_t thread_handle_id,
                                  i32& out_core_mask0, u64& out_core_mask1);
    result_t svcSetThreadCoreMask(handle_id_t thread_handle_id, i32 core_mask0,
                                  u64 core_mask1);
    void svcGetCurrentProcessorNumber(u32& out_number);
    result_t svcSignalEvent(handle_id_t event_handle_id);
    result_t svcClearEvent(handle_id_t event_handle_id);
    result_t svcMapSharedMemory(handle_id_t shared_mem_handle_id, uptr addr,
                                usize size, MemoryPermission perm);
    result_t svcUnmapSharedMemory(handle_id_t shared_mem_handle_id, uptr addr,
                                  usize size);
    result_t svcCreateTransferMemory(uptr addr, u64 size, MemoryPermission perm,
                                     handle_id_t& out_transfer_mem_handle_id);
    result_t svcCloseHandle(handle_id_t handle_id);
    result_t svcResetSignal(handle_id_t handle_id);
    result_t svcWaitSynchronization(handle_id_t* handle_ids, i32 handle_count,
                                    i64 timeout, u64& out_handle_index);
    result_t svcCancelSynchronization(handle_id_t thread_handle_id);
    result_t svcArbitrateLock(u32 wait_tag, uptr mutex_addr, u32 self_tag);
    result_t svcArbitrateUnlock(uptr mutex_addr);
    result_t svcWaitProcessWideKeyAtomic(uptr mutex_addr, uptr var_addr,
                                         u32 self_tag, i64 timeout);
    result_t svcSignalProcessWideKey(uptr addr, i32 count);
    void svcGetSystemTick(u64& out_tick);
    result_t svcConnectToNamedPort(const std::string& name,
                                   handle_id_t& out_session_handle_id);
    result_t svcSendSyncRequest(hw::tegra_x1::cpu::MemoryBase* tls_mem,
                                handle_id_t session_handle_id);
    result_t svcGetThreadId(handle_id_t thread_handle_id, u64& out_thread_id);
    result_t svcBreak(BreakReason reason, uptr buffer_ptr, usize buffer_size);
    result_t svcOutputDebugString(const char* str, usize len);
    result_t svcGetInfo(InfoType info_type, handle_id_t handle_id,
                        u64 info_sub_type, u64& out_info);
    result_t svcMapPhysicalMemory(vaddr_t addr, usize size);
    result_t svcSetThreadActivity(handle_id_t thread_handle_id,
                                  ThreadActivity activity);
    result_t svcGetThreadContext3(handle_id_t thread_handle_id,
                                  ThreadContext& out_thread_context);

    // Helpers
    Handle* GetHandle(handle_id_t handle_id) const {
        return handle_pool.Get(handle_id);
    }
    handle_id_t AddHandle(Handle* handle) { return handle_pool.Add(handle); }
    void FreeHandle(handle_id_t handle_id) { handle_pool.Free(handle_id); }

    hw::tegra_x1::cpu::MemoryBase* CreateTlsMemory(vaddr_t& base);

    hw::Bus& GetBus() const { return bus; }
    hw::tegra_x1::cpu::MMUBase* GetMMU() const { return mmu; }

    u64 GetTitleID() const { return title_id; }
    void SetTitleId(const u64 title_id_) { title_id = title_id_; }

  private:
    hw::Bus& bus;
    hw::tegra_x1::cpu::MMUBase* mmu;

    filesystem::Filesystem filesystem;

    u64 title_id{invalid<u64>()};

    // Memory
    hw::tegra_x1::cpu::MemoryBase* heap_mem;
    std::vector<hw::tegra_x1::cpu::MemoryBase*> executable_mems;

    vaddr_t executable_mem_base{0x40000000};
    vaddr_t tls_mem_base{TLS_REGION_BASE};

    // Handles
    DynamicHandlePool<Handle> handle_pool;

    std::mutex sync_mutex;
    // TODO: use a different container?
    std::map<vaddr_t, Mutex> mutex_map;
    std::map<vaddr_t, std::condition_variable> cond_var_map;

    // Services
    std::map<std::string, ServiceBase*> service_ports;
};

template <typename T> struct HandleWithId {
    static_assert(std::is_convertible_v<T*, Handle*>,
                  "Type does not inherit from Handle");

    T* handle;
    handle_id_t id;

    HandleWithId(T* handle_) : handle{handle_} {
        id = KERNEL_INSTANCE.AddHandle(handle);
    }

    ~HandleWithId() { KERNEL_INSTANCE.FreeHandle(id); }
};

} // namespace hydra::horizon::kernel
