#pragma once

#include "horizon/kernel.hpp"

namespace Hydra::HW::MMU {
class MMUBase;
}

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

struct DisplayBinder {
    u32 weak_ref_count = 0;
    u32 strong_ref_count = 0;
};

class DisplayBinderManager {
  public:
    u32 AddBinder() {
        binders[binder_count] = DisplayBinder{};
        return binder_count++;
    }

    DisplayBinder& GetBinder(u32 id) { return binders[id]; }

  private:
    DisplayBinder
        binders[0x100]; // TODO: what should be the max number of binders?
    u32 binder_count = 0;
};

class OS {
  public:
    static OS& GetInstance();

    OS(HW::Bus& bus);
    ~OS();

    void SetMMU(HW::MMU::MMUBase* mmu);

    // Getters
    Kernel& GetKernel() { return kernel; }

    DisplayBinderManager& GetDisplayBinderManager() {
        return display_binder_manager;
    }

  private:
    Kernel kernel;

    // Services
    Services::Sm::IUserInterface* sm_user_interface;

    // Managers
    DisplayBinderManager display_binder_manager;
};

} // namespace Hydra::Horizon
