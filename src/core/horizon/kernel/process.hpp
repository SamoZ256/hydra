#pragma once

#include "core/horizon/kernel/kernel.hpp"
#include "core/horizon/kernel/synchronization_object.hpp"
#include "core/horizon/kernel/thread.hpp"
#include "core/hw/tegra_x1/cpu/memory.hpp"

namespace hydra::horizon::kernel {

class Process : public SynchronizationObject {
  public:
    Process(const std::string_view debug_name = "Process");
    ~Process() override;

    // Memory
    uptr CreateMemory(usize size, MemoryType type, MemoryPermission perm,
                      bool add_guard_page, vaddr_t& out_base);
    // TODO: should the caller be able to specify permissions?
    uptr CreateExecutableMemory(const std::string_view module_name, usize size,
                                MemoryPermission perm, bool add_guard_page,
                                vaddr_t& out_base);
    hw::tegra_x1::cpu::IMemory* CreateTlsMemory(vaddr_t& base);

    // Thread
    std::pair<Thread*, handle_id_t>
    CreateMainThread(u8 priority, u8 core_number, u32 stack_size);

    void Run();

    // Helpers

    // Handles
    AutoObject* GetHandle(handle_id_t handle_id) {
        if (handle_id == CURRENT_PROCESS_PSEUDO_HANDLE)
            return this;

        return handle_pool.Get(handle_id);
    }
    handle_id_t AddHandle(AutoObject* handle) {
        return handle_pool.Add(handle);
    }
    void FreeHandle(handle_id_t handle_id) {
        if (handle_id == CURRENT_PROCESS_PSEUDO_HANDLE)
            LOG_FATAL(Kernel, "Cannot free current process handle");

        handle_pool.Free(handle_id);
    }

  private:
    hw::tegra_x1::cpu::IMmu* mmu;

    u64 title_id{invalid<u64>()};
    u32 system_resource_size{invalid<u32>()};

    // Memory
    hw::tegra_x1::cpu::IMemory* heap_mem;
    std::vector<hw::tegra_x1::cpu::IMemory*> executable_mems;
    hw::tegra_x1::cpu::IMemory* main_thread_stack_mem;

    vaddr_t mem_base{0x40000000};
    vaddr_t tls_mem_base{TLS_REGION_BASE};

    // Thread
    Thread* main_thread{nullptr};

    // Handles
    DynamicHandlePool<AutoObject> handle_pool; // TODO: could be static?

  public:
    GETTER(mmu, GetMmu);
    REF_GETTER(heap_mem, GetHeapMemory);
    GETTER_AND_SETTER(title_id, GetTitleID, SetTitleID);
    GETTER_AND_SETTER(system_resource_size, GetSystemResourceSize,
                      SetSystemResourceSize);
};

} // namespace hydra::horizon::kernel
