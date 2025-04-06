#pragma once

#include "horizon/input_manager.hpp"
#include "horizon/kernel.hpp"
#include "horizon/state_manager.hpp"
#include "hw/tegra_x1/gpu/const.hpp"

namespace Hydra::HW::Display {
class DisplayBase;
}

namespace Hydra::Horizon {

namespace Services::Sm {
class IUserInterface;
}

// TODO: move to a separate file
struct ParcelData {
    u32 unknown0;
    u32 unknown1;
    u32 binder_id;
    u32 unknown2[3];
    u64 str;
    u64 unknown3;
};

struct Parcel {
    u32 data_size;
    u32 data_offset;
    u32 objects_size;
    u32 objects_offset;
};

struct ParcelFlattenedObject {
    i32 size;
    i32 fd_count;
};

struct DisplayBuffer {
    bool initialized = false;
    bool queued = false;
    HW::TegraX1::GPU::NvGraphicsBuffer buff;
};

constexpr usize MAX_BINDER_BUFFER_COUNT = 32;

struct DisplayBinder {
  public:
    // TODO: make private
    u32 weak_ref_count = 0;
    u32 strong_ref_count = 0;

    DisplayBinder(KernelHandleWithId<SynchronizationHandle>& event_handle_)
        : event_handle{event_handle_} {}

    void AddBuffer(i32 slot, HW::TegraX1::GPU::NvGraphicsBuffer buff) {
        buffers[slot].initialized = true;
        buffers[slot].buff = buff;
        buffer_count++;
    }

    i32 GetAvailableSlot() {
        // Wait for a slot to become available
        std::unique_lock<std::mutex> lock(queue_mutex);
        queue_cv.wait(lock,
                      [&] { return queued_buffers.size() != buffer_count; });

        // Find an available slot
        for (i32 i = 0; i < MAX_BINDER_BUFFER_COUNT; i++) {
            if (buffers[i].initialized && !buffers[i].queued) {
                return i;
            }
        }

        LOG_ERROR(Horizon, "No available slots");
    }

    void QueueBuffer(i32 slot) {
        std::lock_guard<std::mutex> lock(queue_mutex);
        queued_buffers.push(slot);
        buffers[slot].queued = true;
        queue_cv.notify_all();

        // TODO: correct?
        // Signal event
        event_handle.handle->Signal();
    }

    i32 ConsumeBuffer() {
        // Wait for a buffer to become available
        std::unique_lock<std::mutex> lock(queue_mutex);
        // TODO: should there be a timeout?
        queue_cv.wait_for(lock, std::chrono::nanoseconds(67 * 1000 * 1000),
                          [&] { return !queued_buffers.empty(); });

        if (queued_buffers.empty())
            return -1;

        // Get the first queued buffer
        i32 slot = queued_buffers.front();
        queued_buffers.pop();
        buffers[slot].queued = false;
        queue_cv.notify_all();

        return slot;
    }

    // Getters
    HW::TegraX1::GPU::NvGraphicsBuffer& GetBuffer(i32 slot) {
        return buffers[slot].buff;
    }

  private:
    KernelHandleWithId<SynchronizationHandle>& event_handle;

    DisplayBuffer buffers[MAX_BINDER_BUFFER_COUNT]; // TODO: what should be the
                                                    // max number of buffers?
    u32 buffer_count = 0;
    std::queue<i32> queued_buffers;
    std::mutex queue_mutex;
    std::condition_variable queue_cv;
};

class DisplayBinderManager {
  public:
    DisplayBinderManager() : event_handle(new SynchronizationHandle(true)) {}

    u32 AddBinder() {
        u32 id = binders.size();
        binders.push_back(new DisplayBinder(event_handle));

        return id;
    }

    // Getters
    DisplayBinder& GetBinder(u32 id) { return *binders[id]; }

    const KernelHandleWithId<SynchronizationHandle>& GetEventHandle() const {
        return event_handle;
    }

  private:
    std::vector<DisplayBinder*> binders;
    KernelHandleWithId<SynchronizationHandle> event_handle;
};

class OS {
  public:
    static OS& GetInstance();

    OS(HW::Bus& bus, HW::TegraX1::CPU::MMUBase* mmu_);
    ~OS();

    // Getters
    Kernel& GetKernel() { return kernel; }
    StateManager& GetStateManager() { return state_manager; }
    DisplayBinderManager& GetDisplayBinderManager() {
        return display_binder_manager;
    }
    InputManager& GetInputManager() { return input_manager; }

    bool IsInHandheldMode() const {
        // TODO: make this configurable
        return true;
    }

  private:
    HW::TegraX1::CPU::MMUBase* mmu;

    Kernel kernel;

    // Services
    Services::Sm::IUserInterface* sm_user_interface;

    // Managers
    StateManager state_manager;
    DisplayBinderManager display_binder_manager;
    InputManager input_manager;
};

} // namespace Hydra::Horizon
