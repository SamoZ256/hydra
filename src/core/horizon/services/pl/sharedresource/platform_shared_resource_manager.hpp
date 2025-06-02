#pragma once

#include "core/horizon/kernel/kernel.hpp"
#include "core/horizon/services/const.hpp"

namespace hydra::horizon::services::pl::shared_resource {

enum class SharedFontType : u32 {
    JapanUsEurope,
    ChineseSimplified,
    ExtendedChineseSimplified,
    ChineseTraditional,
    Korean,
    NintendoExtended,
};

enum class LoadState : u32 {
    Loading,
    Loaded,
};

class IPlatformSharedResourceManager : public ServiceBase {
  public:
    IPlatformSharedResourceManager();

  protected:
    result_t RequestImpl(RequestContext& context, u32 id) override;

  private:
    kernel::HandleWithId<kernel::SharedMemory> shared_memory_handle;

    // Commands
    result_t RequestLoad(SharedFontType font_type);
    result_t GetLoadState(SharedFontType font_type, LoadState* out_state);
    result_t GetSize(SharedFontType font_type, u32* out_size);
    result_t GetSharedMemoryAddressOffset(SharedFontType font_type,
                                          u32* out_address_offset);
    result_t
    GetSharedMemoryNativeHandle(OutHandle<HandleAttr::Copy> out_handle);
    result_t GetSharedFontInOrderOfPriority();
};

} // namespace hydra::horizon::services::pl::shared_resource

ENABLE_ENUM_FORMATTING(
    hydra::horizon::services::pl::shared_resource::SharedFontType,
    JapanUsEurope, "Japan/US/Europe", ChineseSimplified, "Chinese Simplified",
    ExtendedChineseSimplified, "Extended Chinese Simplified",
    ChineseTraditional, "Chinese Traditional", Korean, "Korean",
    NintendoExtended, "Nintendo Extended")

ENABLE_ENUM_FORMATTING(hydra::horizon::services::pl::shared_resource::LoadState,
                       Loading, "Loading", Loaded, "Loaded")
