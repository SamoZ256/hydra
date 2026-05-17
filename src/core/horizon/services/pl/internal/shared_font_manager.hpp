#pragma once

#include "core/horizon/const.hpp"
#include "core/horizon/kernel/kernel.hpp"
#include "core/horizon/services/const.hpp"
#include "core/horizon/services/pl/const.hpp"

namespace hydra::horizon::services::pl::internal {

struct FontState {
    u32 shared_memory_offset{0};
    usize size{0};
};

class SharedFontManager {
  public:
    SharedFontManager(System& system_);
    ~SharedFontManager();

    void LoadFonts();

  private:
    System& system;

    kernel::SharedMemory* shared_memory;
    u32 shared_memory_offset{0};
    FontState states[u32(SharedFontType::Total)]{};

    // Helpers
    void LoadFont(const SharedFontType type);

  public:
    GETTER(shared_memory, GetSharedMemory);
    const FontState& GetState(SharedFontType type) const {
        return states[u32(type)];
    }
};

} // namespace hydra::horizon::services::pl::internal
