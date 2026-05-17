#pragma once

#include "core/horizon/const.hpp"
#include "core/horizon/kernel/kernel.hpp"
#include "core/horizon/services/const.hpp"
#include "core/horizon/services/pl/const.hpp"

namespace hydra::horizon::services::pl {

class ISharedFontManager : public IService {
  protected:
    // Commands
    result_t RequestLoad(SharedFontType font_type);
    result_t GetLoadState(SharedFontType font_type, LoadState* out_state);
    result_t GetSize(System* system, SharedFontType font_type, u32* out_size);
    result_t GetSharedMemoryAddressOffset(System* system,
                                          SharedFontType font_type,
                                          u32* out_address_offset);
    result_t
    GetSharedMemoryNativeHandle(System* system, kernel::Process* process,
                                OutHandle<HandleAttr::Copy> out_handle);
    // TODO: buffer attr
    // TODO: should out_loaded be a bool?
    result_t GetSharedFontInOrderOfPriority(
        System* system, LanguageCode language_code, u8* out_loaded,
        u32* out_count, OutBuffer<BufferAttr::MapAlias> out_types_buffer,
        OutBuffer<BufferAttr::MapAlias> out_offsets_buffer,
        OutBuffer<BufferAttr::MapAlias> out_sizes_buffer);
};

} // namespace hydra::horizon::services::pl
