#pragma once

#include "common/allocators/dynamic_pool.hpp"
#include "common/allocators/static_pool.hpp"
#include "horizon/filesystem/filesystem.hpp"
#include "horizon/shared_memory.hpp"
#include <condition_variable>
#include <type_traits>

namespace Hydra::HW::TegraX1::CPU {
class MMUBase;
class ThreadBase;
} // namespace Hydra::HW::TegraX1::CPU

namespace Hydra::HW {
class Bus;
}

namespace Hydra::Horizon {

namespace Services {
class ServiceBase;
}

class KernelHandle {
  public:
    virtual ~KernelHandle() = default;
};

class SynchronizationHandle : public KernelHandle {
  public:
    SynchronizationHandle(bool signaled_ = false) : signaled{signaled_} {}

    void Signal() {
        std::unique_lock<std::mutex> lock(mutex);
        signaled = true;
        cv.notify_all();
    }

    void Wait(i64 timeout) {
        std::unique_lock<std::mutex> lock(mutex);
        if (IS_TIMEOUT_INFINITE(timeout)) {
            cv.wait(lock, [this] { return signaled; });
        } else {
            cv.wait_for(lock, std::chrono::nanoseconds(timeout),
                        [this] { return signaled; });
        }

        // TODO: correct?
        signaled = false;
    }

  private:
    std::mutex mutex;
    std::condition_variable cv;
    bool signaled = false;
};

class ThreadHandle : public KernelHandle {
  public:
    ThreadHandle(HW::TegraX1::CPU::MemoryBase* tls_mem_, vaddr tls_addr_,
                 vaddr entry_point_, vaddr args_addr_, vaddr stack_top_addr_,
                 i32 priority_)
        : tls_mem{tls_mem_}, tls_addr{tls_addr_}, entry_point{entry_point_},
          args_addr{args_addr_}, stack_top_addr{stack_top_addr_},
          priority{priority_} {}
    ~ThreadHandle() override;

    void Start();

  private:
    HW::TegraX1::CPU::MemoryBase* tls_mem;
    vaddr tls_addr;
    vaddr entry_point;
    vaddr args_addr;
    vaddr stack_top_addr;
    i32 priority;

    std::thread* t = nullptr;
};

class TransferMemory : public KernelHandle {
  public:
    TransferMemory(uptr addr_, u64 size_, MemoryPermission perm_)
        : addr{addr_}, size{size_}, perm{perm_} {}

  private:
    uptr addr;
    u64 size;
    MemoryPermission perm;
};

constexpr usize ARG_COUNT = 2;

class Kernel {
  public:
    static Kernel& GetInstance();

    Kernel(HW::Bus& bus_, HW::TegraX1::CPU::MMUBase* mmu_);
    ~Kernel();

    void ConfigureThread(HW::TegraX1::CPU::ThreadBase* thread,
                         vaddr entry_point, vaddr tls_addr,
                         vaddr stack_top_addr);
    void ConfigureMainThread(HW::TegraX1::CPU::ThreadBase* thread);

    // Loading
    // TODO: should the caller be able to specify permissions?
    uptr CreateExecutableMemory(usize size, vaddr& out_base,
                                MemoryPermission perm);
    void SetMainThreadEntryPoint(uptr main_thread_entry_point_) {
        main_thread_entry_point = main_thread_entry_point_;
    }
    void SetMainThreadArg(u32 index, u64 value) {
        ASSERT_DEBUG(index < ARG_COUNT, HorizonKernel, "Invalid arg index {}",
                     index);
        main_thread_args[index] = value;
    }

    void ConnectServiceToPort(const std::string& port_name,
                              Services::ServiceBase* service) {
        service_ports[port_name] = service;
    }

    bool SupervisorCall(HW::TegraX1::CPU::ThreadBase* thread, u64 id);

    // SVCs
    Result svcSetHeapSize(usize size, uptr& out_base);
    Result svcSetMemoryPermission(uptr addr, usize size, MemoryPermission perm);
    Result svcSetMemoryAttribute(uptr addr, usize size, u32 mask, u32 value);
    Result svcMapMemory(uptr dst_addr, uptr src_addr, usize size);
    Result svcUnmapMemory(uptr dst_addr, uptr src_addr, usize size);
    Result svcQueryMemory(uptr addr, MemoryInfo& out_mem_info,
                          u32& out_page_info);
    void svcExitProcess();
    Result svcCreateThread(vaddr entry_point, vaddr args_addr,
                           vaddr stack_top_addr, i32 priority, i32 processor_id,
                           HandleId& out_thread_handle_id);
    void svcStartThread(HandleId thread_handle_id);
    void svcSleepThread(i64 nano);
    Result svcGetThreadPriority(HandleId thread_handle_id, i32& out_priority);
    Result svcSetThreadPriority(HandleId thread_handle_id, i32 priority);
    Result svcMapSharedMemory(HandleId shared_mem_handle_id, uptr addr,
                              usize size, MemoryPermission perm);
    Result svcUnmapSharedMemory(HandleId shared_mem_handle_id, uptr addr,
                                usize size);
    Result svcCreateTransferMemory(uptr addr, u64 size, MemoryPermission perm,
                                   HandleId& out_transfer_mem_handle_id);
    Result svcCloseHandle(HandleId handle_id);
    Result svcResetSignal(HandleId handle_id);
    Result svcWaitSynchronization(HandleId* handle_ids, i32 handle_count,
                                  i64 timeout, u64& out_handle_index);
    Result svcArbitrateLock(u32 wait_tag, uptr mutex_addr, u32 self_tag);
    Result svcArbitrateUnlock(uptr mutex_addr);
    Result svcWaitProcessWideKeyAtomic(uptr mutex_addr, uptr var_addr,
                                       u32 self_tag, i64 timeout);
    Result svcSignalProcessWideKey(uptr addr, i32 v);
    void svcGetSystemTick(u64& out_tick);
    Result svcConnectToNamedPort(const std::string& name,
                                 HandleId& out_session_handle_id);
    Result svcSendSyncRequest(HW::TegraX1::CPU::MemoryBase* tls_mem,
                              HandleId session_handle_id);
    Result svcGetThreadId(HandleId thread_handle_id, u64& out_thread_id);
    Result svcBreak(BreakReason reason, uptr buffer_ptr, usize buffer_size);
    Result svcOutputDebugString(const char* str, usize len);
    Result svcGetInfo(InfoType info_type, HandleId handle_id, u64 info_sub_type,
                      u64& out_info);

    // Getters
    HW::Bus& GetBus() const { return bus; }

    Filesystem::Filesystem& GetFilesystem() { return filesystem; }

    HW::TegraX1::CPU::MemoryBase* GetTlsMemory() const { return tls_mem; }

    // Helpers
    KernelHandle* GetHandle(HandleId handle_id) const {
        return handle_pool.GetObject(handle_id);
    }
    void SetHandle(HandleId handle_id, KernelHandle* handle);
    HandleId AddHandle(KernelHandle* handle);
    void FreeHandle(HandleId handle_id) {
        delete GetHandle(handle_id);
        handle_pool.FreeByIndex(handle_id);
    }

    HandleId CreateSharedMemory(usize size);

    const SharedMemory& GetSharedMemory(HandleId handle_id) const {
        return *shared_memory_pool.GetObject(handle_id);
    }

    HW::TegraX1::CPU::MemoryBase* CreateTlsMemory(vaddr& base);

  private:
    HW::Bus& bus;
    HW::TegraX1::CPU::MMUBase* mmu;

    uptr main_thread_entry_point{0x0};
    u64 main_thread_args[ARG_COUNT] = {0x0};

    Filesystem::Filesystem filesystem;

    // Memory
    HW::TegraX1::CPU::MemoryBase* stack_mem;
    HW::TegraX1::CPU::MemoryBase* tls_mem; // TODO: remove this
    HW::TegraX1::CPU::MemoryBase* heap_mem;
    std::vector<HW::TegraX1::CPU::MemoryBase*> executable_mems;

    vaddr executable_mem_base{0x80000000};
    vaddr tls_mem_base{TLS_REGION_BASE};

    // Handles
    Allocators::DynamicPool<KernelHandle*> handle_pool;
    Allocators::DynamicPool<SharedMemory*> shared_memory_pool;

    // Services
    std::map<std::string, Services::ServiceBase*> service_ports;
    u8 service_scratch_buffer[0x200];
    u8 service_scratch_buffer_objects[0x100];
    u8 service_scratch_buffer_move_handles[0x100];
    u8 service_scratch_buffer_copy_handles[0x100];
};

template <typename T> struct KernelHandleWithId {
    static_assert(std::is_convertible_v<T*, KernelHandle*>,
                  "Type does not inherit from KernelHandle");

    T* handle;
    HandleId id;

    KernelHandleWithId(T* handle_) : handle{handle_} {
        id = Kernel::GetInstance().AddHandle(handle);
    }

    ~KernelHandleWithId() { Kernel::GetInstance().FreeHandle(id); }
};

} // namespace Hydra::Horizon
