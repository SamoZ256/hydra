#include "core/horizon/services/pl/shared_font_manager.hpp"

#include "core/horizon/kernel/process.hpp"
#include "core/horizon/os.hpp"

namespace hydra::horizon::services::pl {

result_t ISharedFontManager::RequestLoad(SharedFontType font_type) {
    // Do nothing, all the fonts are already loaded
    return RESULT_SUCCESS;
}

result_t ISharedFontManager::GetLoadState(SharedFontType font_type,
                                          LoadState* out_state) {
    *out_state = LoadState::Loaded;
    return RESULT_SUCCESS;
}

result_t ISharedFontManager::GetSize(SharedFontType font_type, u32* out_size) {
    *out_size =
        static_cast<u32>(SHARED_FONT_MANAGER_INSTANCE.GetState(font_type).size);
    return RESULT_SUCCESS;
}

result_t
ISharedFontManager::GetSharedMemoryAddressOffset(SharedFontType font_type,
                                                 u32* out_address_offset) {
    *out_address_offset =
        SHARED_FONT_MANAGER_INSTANCE.GetState(font_type).shared_memory_offset;
    return RESULT_SUCCESS;
}

result_t ISharedFontManager::GetSharedMemoryNativeHandle(
    kernel::Process* process, OutHandle<HandleAttr::Copy> out_handle) {
    out_handle =
        process->AddHandle(SHARED_FONT_MANAGER_INSTANCE.GetSharedMemory());
    return RESULT_SUCCESS;
}

result_t ISharedFontManager::GetSharedFontInOrderOfPriority(
    LanguageCode language_code, u8* out_loaded, u32* out_count,
    OutBuffer<BufferAttr::MapAlias> out_types_buffer,
    OutBuffer<BufferAttr::MapAlias> out_offsets_buffer,
    OutBuffer<BufferAttr::MapAlias> out_sizes_buffer) {
    *out_loaded = 0;
    *out_count = 0;

    for (SharedFontType type = (SharedFontType)0; type < SharedFontType::Total;
         type++) {
        const auto& state = SHARED_FONT_MANAGER_INSTANCE.GetState(type);
        out_types_buffer.writer->Write(type);
        out_offsets_buffer.writer->Write<u32>(state.shared_memory_offset);
        out_sizes_buffer.writer->Write(static_cast<u32>(state.size));

        (*out_loaded)++;
        (*out_count)++;
    }

    return RESULT_SUCCESS;
}

} // namespace hydra::horizon::services::pl
