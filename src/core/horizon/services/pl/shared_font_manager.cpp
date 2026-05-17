#include "core/horizon/services/pl/shared_font_manager.hpp"

#include "core/horizon/kernel/process.hpp"
#include "core/system.hpp"

namespace hydra::horizon::services::pl {

result_t ISharedFontManager::RequestLoad(SharedFontType font_type) {
    (void)font_type;

    // Do nothing, all the fonts are already loaded
    return RESULT_SUCCESS;
}

result_t ISharedFontManager::GetLoadState(SharedFontType font_type,
                                          LoadState* out_state) {
    (void)font_type;

    *out_state = LoadState::Loaded;
    return RESULT_SUCCESS;
}

result_t ISharedFontManager::GetSize(System* system, SharedFontType font_type,
                                     u32* out_size) {
    *out_size = static_cast<u32>(
        system->GetOS().GetSharedFontManager().GetState(font_type).size);
    return RESULT_SUCCESS;
}

result_t ISharedFontManager::GetSharedMemoryAddressOffset(
    System* system, SharedFontType font_type, u32* out_address_offset) {
    *out_address_offset = system->GetOS()
                              .GetSharedFontManager()
                              .GetState(font_type)
                              .shared_memory_offset;
    return RESULT_SUCCESS;
}

result_t ISharedFontManager::GetSharedMemoryNativeHandle(
    System* system, kernel::Process* process,
    OutHandle<HandleAttr::Copy> out_handle) {
    out_handle = process->AddHandle(
        system->GetOS().GetSharedFontManager().GetSharedMemory());
    return RESULT_SUCCESS;
}

result_t ISharedFontManager::GetSharedFontInOrderOfPriority(
    System* system, LanguageCode language_code, u8* out_loaded, u32* out_count,
    OutBuffer<BufferAttr::MapAlias> out_types_buffer,
    OutBuffer<BufferAttr::MapAlias> out_offsets_buffer,
    OutBuffer<BufferAttr::MapAlias> out_sizes_buffer) {
    *out_loaded = 0;
    *out_count = 0;

    // TODO: sort by priority
    (void)language_code;
    for (SharedFontType type = SharedFontType::JapanUsEurope;
         type < SharedFontType::Total; type++) {
        const auto& state =
            system->GetOS().GetSharedFontManager().GetState(type);
        out_types_buffer.stream->Write(type);
        out_offsets_buffer.stream->Write<u32>(state.shared_memory_offset);
        out_sizes_buffer.stream->Write(static_cast<u32>(state.size));

        (*out_loaded)++;
        (*out_count)++;
    }

    return RESULT_SUCCESS;
}

} // namespace hydra::horizon::services::pl
