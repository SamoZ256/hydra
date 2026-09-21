#pragma once

#include "core/horizon/const.hpp"
#include "core/horizon/kernel/kernel.hpp"
#include "core/horizon/services/const.hpp"
#include "core/horizon/services/pl/const.hpp"

namespace hydra::horizon::services::pl::internal {

struct FontState {
    u32 shared_memory_offset{0};
    u64 size{0};
};

class SharedFontManager {
  public:
    explicit SharedFontManager(System& system_);
    ~SharedFontManager();

    void loadFonts();

  private:
    System& system;

    kernel::SharedMemory* shared_memory;
    u32 shared_memory_offset{0};
    FontState states[6]{};

    // Helpers
    void loadFont(const SharedFontType type);

  public:
    GETTER(shared_memory, getSharedMemory);
    const FontState& getState(SharedFontType type) const {
        return states[static_cast<u32>(type)];
    }
};

} // namespace hydra::horizon::services::pl::internal
