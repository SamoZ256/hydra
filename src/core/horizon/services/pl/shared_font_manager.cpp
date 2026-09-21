#include "core/horizon/services/pl/shared_font_manager.hpp"

#include "core/horizon/kernel/process.hpp"
#include "core/system.hpp"

namespace hydra::horizon::services::pl {

result_t ISharedFontManager::requestLoad(SharedFontType font_type) {
    (void)font_type;

    // Do nothing, all the fonts are already loaded
    return RESULT_SUCCESS;
}

result_t ISharedFontManager::getLoadState(SharedFontType font_type,
                                          LoadState* out_state) {
    (void)font_type;

    *out_state = LoadState::Loaded;
    return RESULT_SUCCESS;
}

result_t ISharedFontManager::getSize(System* system, SharedFontType font_type,
                                     u32* out_size) {
    *out_size = static_cast<u32>(
        system->getOs().getSharedFontManager().getState(font_type).size);
    return RESULT_SUCCESS;
}

result_t ISharedFontManager::getSharedMemoryAddressOffset(
    System* system, SharedFontType font_type, u32* out_address_offset) {
    *out_address_offset = system->getOs()
                              .getSharedFontManager()
                              .getState(font_type)
                              .shared_memory_offset;
    return RESULT_SUCCESS;
}

result_t ISharedFontManager::getSharedMemoryNativeHandle(
    System* system, kernel::Process* process,
    OutHandle<HandleAttr::Copy> out_handle) {
    out_handle = process->addHandle(
        system->getOs().getSharedFontManager().getSharedMemory());
    return RESULT_SUCCESS;
}

result_t ISharedFontManager::getSharedFontInOrderOfPriority(
    System* system, LanguageCode language_code, u8* out_loaded, u32* out_count,
    OutBuffer<BufferAttr::MapAlias> out_types_buffer,
    OutBuffer<BufferAttr::MapAlias> out_offsets_buffer,
    OutBuffer<BufferAttr::MapAlias> out_sizes_buffer) {
    *out_loaded = 0;
    *out_count = 0;

    // TODO: sort by priority
    (void)language_code;
    for (SharedFontType type = SharedFontType::JapanUsEurope;
         type <= SharedFontType::NintendoExtended; type++) {
        const auto& state =
            system->getOs().getSharedFontManager().getState(type);
        out_types_buffer.stream->write(type);
        out_offsets_buffer.stream->write<u32>(state.shared_memory_offset);
        out_sizes_buffer.stream->write(static_cast<u32>(state.size));

        (*out_loaded)++;
        (*out_count)++;
    }

    return RESULT_SUCCESS;
}

result_t ISharedFontManager::getSharedFontInOrderOfPriorityForSystem(
    System* system, LanguageCode language_code, u8* out_loaded, u32* out_count,
    OutBuffer<BufferAttr::MapAlias> out_types_buffer,
    OutBuffer<BufferAttr::MapAlias> out_offsets_buffer,
    OutBuffer<BufferAttr::MapAlias> out_sizes_buffer) {
    // TODO: how is this different from GetSharedFontInOrderOfPriority?
    return getSharedFontInOrderOfPriority(
        system, language_code, out_loaded, out_count,
        std::move(out_types_buffer), std::move(out_offsets_buffer),
        std::move(out_sizes_buffer));
}

} // namespace hydra::horizon::services::pl
