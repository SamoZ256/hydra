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

    explicit Process(System& system_, std::string_view debug_name = "Process");
    ~Process() override;

    // Memory
    uptr createMemory(ztd::Range<vaddr_t> region, u64 size, MemoryType type,
                      MemoryPermission perm, vaddr_t& out_base);
    uptr createExecutableMemory(const std::string_view module_name,
                                CodeSet code_set, vaddr_t& out_base);
    hw::tegra_x1::cpu::IMemory* createTlsMemory(vaddr_t& base);
    void createStackMemory(u64 stack_size);
    void resizeHeap(u64 size);

    // Thread
    Handle setMainThread(GuestThread* thread) {
        main_thread = thread;
        return addHandle(main_thread);
    }

    void registerThread(IThread* thread) {
        std::scoped_lock lock(thread_mutex);
        threads.push_back(thread);
    }
    void unregisterThread(IThread* thread) {
        std::scoped_lock lock(thread_mutex);
        std::erase(threads, thread);

        // Signal
        if (threads.empty())
            signalStateChange(ProcessState::Exited);
    }

    void start();
    void stop();

    void supervisorPause();
    void supervisorResume();

    bool isRunning() {
        std::scoped_lock lock(thread_mutex);
        return !threads.empty();
    }

    ProcessState getState() const { return state; }

    // Helpers

    // Handles
    template <typename T>
    // TODO: uncomment
    /*std::optional<T*>*/ T* getHandle(Handle handle) {
        static_assert(std::is_base_of_v<AutoObject, T>,
                      "T must be derived from AutoObject");

        if (!handle.isValid())
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
        return handle_pool.get(handle)
            .transform(
                [](AutoObject* obj) -> auto { return static_cast<T*>(obj); })
            .value_or(nullptr);
    }

    Handle addHandleNoRetain(AutoObject* obj) {
        // TODO: remove
        if (obj == nullptr) [[unlikely]]
            return INVALID_HANDLE;

        return handle_pool.insert(obj).value();
    }

    Handle addHandle(AutoObject* obj) {
        // TODO: remove
        if (obj == nullptr) [[unlikely]]
            return INVALID_HANDLE;

        obj->retain();
        return handle_pool.insert(obj).value();
    }

    bool freeHandle(Handle handle) {
        ASSERT_DEBUG(handle != CURRENT_PROCESS_PSEUDO_HANDLE, Kernel,
                     "Cannot free current process handle");
        ASSERT_DEBUG(handle != CURRENT_THREAD_PSEUDO_HANDLE, Kernel,
                     "Cannot free current thread handle");

        const auto object = handle_pool.get(handle);
        if (!object.has_value()) {
            LOG_WARN(Kernel, "Invalid handle {}", handle);
            return false;
        }

        object.value()->release();
        ASSERT_DEBUG(handle_pool.free(handle), Kernel,
                     "Failed to free handle {}", handle);
        return true;
    }

    hw::tegra_x1::cpu::IMmu* getMmu() const { return mmu.get(); }
    hw::tegra_x1::cpu::IMemory* getHeapMemory() const { return heap_mem.get(); }

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

    void cleanUp();

    void signalStateChange(ProcessState new_state);

  public:
    REF_GETTER(gmmu, getGMmu);
    REF_GETTER(applet_state, getAppletState);
    GETTER_AND_SETTER(title_id, getTitleId, setTitleId);
    GETTER_AND_SETTER(system_resource_size, getSystemResourceSize,
                      setSystemResourceSize);
    CONST_REF_GETTER(random_entropy, getRandomEntropy);
    GETTER(main_thread, getMainThread);
};

} // namespace hydra::horizon::kernel
