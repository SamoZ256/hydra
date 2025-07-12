#pragma once

#include "core/horizon/const.hpp"
#include "core/horizon/kernel/kernel.hpp"
#include "core/horizon/services/const.hpp"

namespace hydra::horizon::services::pl::shared_resource {

enum class SharedFontType : u32 {
    JapanUsEurope = 0,
    ChineseSimplified = 1,
    ExtendedChineseSimplified = 2,
    ChineseTraditional = 3,
    Korean = 4,
    NintendoExtended = 5,

    Total,
};
ENABLE_ENUM_ARITHMETIC_OPERATORS(SharedFontType)

enum class LoadState : u32 {
    Loading = 0,
    Loaded = 1,
};

class IPlatformSharedResourceManager : public ServiceBase {
  public:
    IPlatformSharedResourceManager();

    // HACK
    usize GetPointerBufferSize() override { return 0x1000; }

  protected:
    result_t RequestImpl(RequestContext& context, u32 id) override;

  private:
    kernel::SharedMemory* shared_memory;
    u32 shared_memory_offset{0};
    struct {
        u32 shared_memory_offset{0};
        usize size{0};
    } states[(u32)SharedFontType::Total]{};

    // Commands
    result_t RequestLoad(SharedFontType font_type);
    result_t GetLoadState(SharedFontType font_type, LoadState* out_state);
    result_t GetSize(SharedFontType font_type, u32* out_size);
    result_t GetSharedMemoryAddressOffset(SharedFontType font_type,
                                          u32* out_address_offset);
    result_t
    GetSharedMemoryNativeHandle(kernel::Process* process,
                                OutHandle<HandleAttr::Copy> out_handle);
    // TODO: buffer attr
    // TODO: should out_loaded be a bool?
    result_t GetSharedFontInOrderOfPriority(
        LanguageCode language_code, u8* out_loaded, u32* out_count,
        OutBuffer<BufferAttr::MapAlias> out_types_buffer,
        OutBuffer<BufferAttr::MapAlias> out_offsets_buffer,
        OutBuffer<BufferAttr::MapAlias> out_sizes_buffer);

    // Helpers
    void LoadFont(const SharedFontType type);
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
