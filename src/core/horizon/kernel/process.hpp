#pragma once

#include "core/horizon/handle_pool.hpp"
#include "core/horizon/kernel/applet_state.hpp"
#include "core/horizon/kernel/synchronization_object.hpp"
#include "core/horizon/kernel/thread.hpp"
#include "core/hw/tegra_x1/cpu/memory.hpp"
#include "core/hw/tegra_x1/gpu/gmmu.hpp"

// TODO: remove dependency
#include "core/horizon/kernel/guest_thread.hpp"

namespace hydra::hw::tegra_x1::cpu {
class IMmu;
} // namespace hydra::hw::tegra_x1::cpu

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
    ztd::Range<u64> code;
    ztd::Range<u64> ro_data;
    ztd::Range<u64> data;
};

class Process : public SynchronizationObject {
  public:
    static constexpr AutoObjectTypeId TYPE_ID = AutoObjectTypeId::Process;

    Process(System& system_, std::string_view debug_name = "Process");
    ~Process() override;

    // Memory
    uptr CreateMemory(ztd::Range<vaddr_t> region, u64 size, MemoryType type,
                      MemoryPermission perm, vaddr_t& out_base);
    uptr CreateExecutableMemory(const std::string_view module_name,
                                CodeSet code_set, vaddr_t& out_base);
    hw::tegra_x1::cpu::IMemory* CreateTlsMemory(vaddr_t& base);
    void CreateStackMemory(u64 stack_size);
    void ResizeHeap(u64 size);

    // Thread
    Handle SetMainThread(GuestThread* thread) {
        main_thread = thread;
        return AddHandle(main_thread);
    }

    void RegisterThread(IThread* thread) {
        std::scoped_lock lock(thread_mutex);
        threads.push_back(thread);
    }
    void UnregisterThread(IThread* thread) {
        std::scoped_lock lock(thread_mutex);
        std::erase(threads, thread);

        // Signal
        if (threads.empty())
            SignalStateChange(ProcessState::Exited);
    }

    void Start();
    void Stop();

    void SupervisorPause();
    void SupervisorResume();

    bool IsRunning() {
        std::scoped_lock lock(thread_mutex);
        return !threads.empty();
    }

    ProcessState GetState() const { return state; }

    // Helpers

    // Handles
    template <typename T>
    // TODO: uncomment
    /*std::optional<T*>*/ T* GetHandle(Handle handle) {
        static_assert(std::is_base_of_v<AutoObject, T>,
                      "T must be derived from AutoObject");

        if (!handle.IsValid())
            return nullptr; // TODO: std::nullopt

        if constexpr (std::is_base_of_v<T, Process>) {
            if (handle == CURRENT_PROCESS_PSEUDO_HANDLE) [[unlikely]] {
                return this;
            }
        }

        if constexpr (std::is_base_of_v<T, IThread>) {
            if (handle == CURRENT_THREAD_PSEUDO_HANDLE) [[unlikely]] {
                return tls_current_thread;
            }
        }

        // HACK
        return handle_pool.Get(handle)
            .transform(
                [](AutoObject* obj) -> auto { return static_cast<T*>(obj); })
            .value_or(nullptr);
    }

    Handle AddHandleNoRetain(AutoObject* obj) {
        // TODO: remove
        if (obj == nullptr) [[unlikely]]
            return INVALID_HANDLE;

        return handle_pool.Insert(obj).value();
    }

    Handle AddHandle(AutoObject* obj) {
        // TODO: remove
        if (obj == nullptr) [[unlikely]]
            return INVALID_HANDLE;

        obj->Retain();
        return handle_pool.Insert(obj).value();
    }

    bool FreeHandle(Handle handle) {
        ASSERT_DEBUG(handle != CURRENT_PROCESS_PSEUDO_HANDLE, Kernel,
                     "Cannot free current process handle");
        ASSERT_DEBUG(handle != CURRENT_THREAD_PSEUDO_HANDLE, Kernel,
                     "Cannot free current thread handle");

        const auto object = handle_pool.Get(handle);
        if (!object.has_value()) {
            LOG_WARN(Kernel, "Invalid handle {}", handle);
            return false;
        }

        object.value()->Release();
        ASSERT_DEBUG(handle_pool.Free(handle), Kernel,
                     "Failed to free handle {}", handle);
        return true;
    }

    hw::tegra_x1::cpu::IMmu* GetMmu() const { return mmu.get(); }
    hw::tegra_x1::cpu::IMemory* GetHeapMemory() const { return heap_mem.get(); }

  private:
    System& system;

    std::unique_ptr<hw::tegra_x1::cpu::IMmu> mmu;
    hw::tegra_x1::gpu::GMmu gmmu;

    AppletState applet_state;

    u64 title_id{invalid<u64>()};
    u32 system_resource_size{invalid<u32>()};

    // Random entropy
    std::array<u64, 4> random_entropy;

    // Memory
    std::vector<std::unique_ptr<hw::tegra_x1::cpu::IMemory>> executable_mems;
    std::unique_ptr<hw::tegra_x1::cpu::IMemory> main_thread_stack_mem;
    std::unique_ptr<hw::tegra_x1::cpu::IMemory> heap_mem;

    vaddr_t tls_mem_base{TLS_REGION.getBegin()};

    // Thread
    GuestThread* main_thread{nullptr};
    std::mutex thread_mutex;
    std::vector<IThread*> threads;

    // Handles
    // TODO: store as strong refs
    StaticHandlePool<AutoObject*, 512>
        handle_pool; // TODO: get the size from capabilities

    std::atomic<ProcessState> state{ProcessState::Created};

    void CleanUp();

    void SignalStateChange(ProcessState new_state);

  public:
    REF_GETTER(gmmu, GetGMmu);
    REF_GETTER(applet_state, GetAppletState);
    GETTER_AND_SETTER(title_id, GetTitleID, SetTitleID);
    GETTER_AND_SETTER(system_resource_size, GetSystemResourceSize,
                      SetSystemResourceSize);
    CONST_REF_GETTER(random_entropy, GetRandomEntropy);
    GETTER(main_thread, GetMainThread);
};

} // namespace hydra::horizon::kernel
