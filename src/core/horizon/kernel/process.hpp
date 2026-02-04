#pragma once

#include "core/horizon/kernel/applet_state.hpp"
#include "core/horizon/kernel/synchronization_object.hpp"
#include "core/horizon/kernel/thread.hpp"
#include "core/hw/tegra_x1/cpu/memory.hpp"

// TODO: remove dependency
#include "core/horizon/kernel/guest_thread.hpp"

namespace hydra::hw::tegra_x1::cpu {
class IMmu;
} // namespace hydra::hw::tegra_x1::cpu

namespace hydra::hw::tegra_x1::gpu {
class GMmu;
} // namespace hydra::hw::tegra_x1::gpu

namespace hydra::horizon::kernel {

enum class ProcessState {
    Created = 0,
    CreatedAttached = 1,
    Started = 2,
    Crashed = 3, // Only in debug mode
    StartedAttached = 4,
    Exiting = 5,
    Exited = 6,
    DebugSuspended = 7,
};

struct CodeSet {
    u64 size;
    Range<u64> code;
    Range<u64> ro_data;
    Range<u64> data;
};

class Process : public SynchronizationObject {
  public:
    Process(const std::string_view debug_name = "Process");
    ~Process() override;

    // Memory
    uptr CreateMemory(Range<vaddr_t> region, usize size, MemoryType type,
                      MemoryPermission perm, vaddr_t& out_base);
    uptr CreateExecutableMemory(const std::string_view module_name,
                                CodeSet code_set, vaddr_t& out_base);
    hw::tegra_x1::cpu::IMemory* CreateTlsMemory(vaddr_t& base);
    void CreateStackMemory(usize stack_size);

    // Thread
    handle_id_t SetMainThread(GuestThread* thread) {
        main_thread = thread;
        return AddHandle(main_thread);
    }

    void RegisterThread(IThread* thread) {
        std::lock_guard lock(thread_mutex);
        threads.push_back(thread);
    }
    void UnregisterThread(IThread* thread) {
        std::lock_guard lock(thread_mutex);
        threads.erase(std::remove(threads.begin(), threads.end(), thread),
                      threads.end());

        // Signal
        if (threads.empty())
            SignalStateChange(ProcessState::Exited);
    }

    void Start();
    void Stop();

    void SupervisorPause();
    void SupervisorResume();

    bool IsRunning() {
        std::lock_guard lock(thread_mutex);
        return !threads.empty();
    }

    ProcessState GetState() const { return state; }

    // Helpers

    // Handles
    template <typename T>
    T* GetHandle(handle_id_t handle_id) {
        static_assert(std::is_base_of<AutoObject, T>::value,
                      "T must be derived from AutoObject");

        if (handle_id == INVALID_HANDLE_ID)
            return nullptr;

        AutoObject* obj;
        if (handle_id == CURRENT_PROCESS_PSEUDO_HANDLE) [[unlikely]] {
            obj = this;
        } else if (handle_id == CURRENT_THREAD_PSEUDO_HANDLE) [[unlikely]] {
            obj = tls_current_thread;
        } else {
            if (!handle_pool.IsValid(handle_id))
                return nullptr;

            obj = handle_pool.Get(handle_id);
        }

        auto cast_obj = dynamic_cast<T*>(obj);
        ASSERT_DEBUG(cast_obj != nullptr, Kernel, "Invalid handle type");

        return cast_obj;
    }

    handle_id_t AddHandleNoRetain(AutoObject* obj) {
        if (!obj) [[unlikely]]
            return INVALID_HANDLE_ID;

        return handle_pool.Add(obj);
    }

    handle_id_t AddHandle(AutoObject* obj) {
        if (!obj) [[unlikely]]
            return INVALID_HANDLE_ID;

        obj->Retain();
        return handle_pool.Add(obj);
    }

    void FreeHandle(handle_id_t handle_id) {
        ASSERT_DEBUG(handle_id != CURRENT_PROCESS_PSEUDO_HANDLE, Kernel,
                     "Cannot free current process handle");
        ASSERT_DEBUG(handle_id != CURRENT_THREAD_PSEUDO_HANDLE, Kernel,
                     "Cannot free current thread handle");
        handle_pool.Get(handle_id)->Release();
        handle_pool.Free(handle_id);
    }

  private:
    hw::tegra_x1::cpu::IMmu* mmu;
    hw::tegra_x1::gpu::GMmu* gmmu;

    AppletState applet_state;

    u64 title_id{invalid<u64>()};
    u32 system_resource_size{invalid<u32>()};

    // Random entropy
    std::array<u64, 4> random_entropy;

    // Memory
    hw::tegra_x1::cpu::IMemory* heap_mem{nullptr};
    std::vector<hw::tegra_x1::cpu::IMemory*> executable_mems;
    hw::tegra_x1::cpu::IMemory* main_thread_stack_mem{nullptr};

    vaddr_t tls_mem_base{TLS_REGION.GetBegin()};

    // Thread
    GuestThread* main_thread{nullptr};
    std::mutex thread_mutex;
    std::vector<IThread*> threads;

    // Handles
    StaticPool<AutoObject*, 512>
        handle_pool; // TODO: get the size from capabilities

    std::atomic<ProcessState> state{ProcessState::Created};

    void CleanUp();

    void SignalStateChange(ProcessState new_state);

  public:
    GETTER(mmu, GetMmu);
    GETTER(gmmu, GetGMmu);
    REF_GETTER(applet_state, GetAppletState);
    GETTER_AND_SETTER(title_id, GetTitleID, SetTitleID);
    GETTER_AND_SETTER(system_resource_size, GetSystemResourceSize,
                      SetSystemResourceSize);
    CONST_REF_GETTER(random_entropy, GetRandomEntropy);
    REF_GETTER(heap_mem, GetHeapMemory);
    GETTER(main_thread, GetMainThread);
};

} // namespace hydra::horizon::kernel
