#include "core/horizon/services/pl/sharedresource/platform_shared_resource_manager.hpp"

#include "core/horizon/filesystem/content_archive.hpp"
#include "core/horizon/filesystem/file_base.hpp"
#include "core/horizon/filesystem/filesystem.hpp"
#include "core/horizon/filesystem/partition_filesystem.hpp"
#include "core/horizon/filesystem/romfs.hpp"
#include "core/horizon/kernel/process.hpp"

namespace hydra::horizon::services::pl::shared_resource {

namespace {

constexpr usize SHARED_MEMORY_SIZE = 0x01100000;

struct SharedFontName {
    std::string_view name;
    std::string_view filename;
};

#define SHARED_FONT_ENTRY(type, name, filename)                                \
    [(u32)SharedFontType::type] = SharedFontName { name, filename ".bfttf" }

constexpr SharedFontName shared_font_names[] = {
    SHARED_FONT_ENTRY(JapanUsEurope, "FontStandard", "nintendo_udsg-r_std_003"),
    SHARED_FONT_ENTRY(ChineseSimplified, "FontChineseSimple",
                      "nintendo_udsg-r_org_zh-cn_003"),
    SHARED_FONT_ENTRY(ExtendedChineseSimplified, "FontChineseSimple",
                      "nintendo_udsg-r_ext_zh-cn_003"),
    SHARED_FONT_ENTRY(ChineseTraditional, "FontChineseTraditional",
                      "nintendo_udjxh-db_zh-tw_003"),
    SHARED_FONT_ENTRY(Korean, "FontKorean", "nintendo_udsg-r_ko_003"),
    SHARED_FONT_ENTRY(NintendoExtended, "FontNintendoExtension",
                      "nintendo_ext_003"),
};

#undef SHARED_FONT_ENTRY

filesystem::FileBase* GetSharedFontFile(SharedFontType font_type) {
    const auto& name = shared_font_names[(u32)font_type];

    // NCA
    filesystem::FileBase* file;
    auto res = FILESYSTEM_INSTANCE.GetFile(
        fmt::format(FS_FIRMWARE_PATH "/{}", name.name), file);
    if (res != filesystem::FsResult::Success) {
        LOG_ERROR(Services, "Failed to get shared font {} file: {}", font_type,
                  res);
        return nullptr;
    }

    filesystem::ContentArchive content_archive(file);

    // Data
    filesystem::EntryBase* data_entry;
    res = content_archive.GetEntry("data", data_entry);
    if (res != filesystem::FsResult::Success) {
        LOG_ERROR(Services, "Failed to get shared font {} data: {}", font_type,
                  res);
        return nullptr;
    }

    auto data_file = dynamic_cast<filesystem::FileBase*>(data_entry);
    if (!data_file) {
        LOG_ERROR(Services, "Shared font {} data is not a file", font_type);
        return nullptr;
    }

    // RomFS
    filesystem::RomFS romfs(data_file);

    // Font
    filesystem::EntryBase* font_entry;
    res = romfs.GetEntry(name.filename, font_entry);
    if (res != filesystem::FsResult::Success) {
        LOG_ERROR(Services, "Failed to get shared font {}: {}", font_type, res);
        return nullptr;
    }

    auto font_file = dynamic_cast<filesystem::FileBase*>(font_entry);
    if (!font_file) {
        LOG_ERROR(Services, "Shared font {} is not a file", font_type);
        return nullptr;
    }

    return font_file;
}

constexpr u32 BFTTF_MAGIC = 0x18029a7f;
constexpr u32 FONT_KEY = 0x06186249;

result_t DecryptBFTTF(StreamReader reader, Writer writer) {
#define KEY_XOR(x) (x ^ FONT_KEY)

    const auto magic = KEY_XOR(reader.Read<u32>());
    if (magic != BFTTF_MAGIC) {
        LOG_ERROR(Services, "Invalid BFTTF magic");
        return MAKE_RESULT(Svc, 100); // TODO
    }

    reader.Skip(4);

    for (u32 i = 0; i < (reader.GetSize() - 8) / sizeof(u32); i++)
        writer.Write(KEY_XOR(reader.Read<u32>()));
#undef KEY_XOR

    return RESULT_SUCCESS;
}

} // namespace

DEFINE_SERVICE_COMMAND_TABLE(IPlatformSharedResourceManager, 0, RequestLoad, 1,
                             GetLoadState, 2, GetSize, 3,
                             GetSharedMemoryAddressOffset, 4,
                             GetSharedMemoryNativeHandle, 5,
                             GetSharedFontInOrderOfPriority)

IPlatformSharedResourceManager::IPlatformSharedResourceManager()
    : shared_memory{new kernel::SharedMemory(SHARED_MEMORY_SIZE)} {
    // Load fonts
    for (SharedFontType type = (SharedFontType)0; type < SharedFontType::Total;
         type++)
        LoadFont(type);
}

result_t IPlatformSharedResourceManager::RequestLoad(SharedFontType font_type) {
    // Do nothing, all the fonts are already loaded
    return RESULT_SUCCESS;
}

result_t IPlatformSharedResourceManager::GetLoadState(SharedFontType font_type,
                                                      LoadState* out_state) {
    *out_state = LoadState::Loaded;
    return RESULT_SUCCESS;
}

result_t IPlatformSharedResourceManager::GetSize(SharedFontType font_type,
                                                 u32* out_size) {
    *out_size = states[(u32)font_type].size;
    return RESULT_SUCCESS;
}

result_t IPlatformSharedResourceManager::GetSharedMemoryAddressOffset(
    SharedFontType font_type, u32* out_address_offset) {
    *out_address_offset = states[(u32)font_type].shared_memory_offset;
    return RESULT_SUCCESS;
}

result_t IPlatformSharedResourceManager::GetSharedMemoryNativeHandle(
    kernel::Process* process, OutHandle<HandleAttr::Copy> out_handle) {
    out_handle = process->AddHandle(shared_memory);
    return RESULT_SUCCESS;
}

result_t IPlatformSharedResourceManager::GetSharedFontInOrderOfPriority(
    LanguageCode language_code, u8* out_loaded, u32* out_count,
    OutBuffer<BufferAttr::MapAlias> out_types_buffer,
    OutBuffer<BufferAttr::MapAlias> out_offsets_buffer,
    OutBuffer<BufferAttr::MapAlias> out_sizes_buffer) {
    *out_loaded = 0;
    *out_count = 0;

    for (SharedFontType type = (SharedFontType)0; type < SharedFontType::Total;
         type++) {
        const auto& state = states[(u32)type];
        out_types_buffer.writer->Write(type);
        out_offsets_buffer.writer->Write<u32>(state.shared_memory_offset);
        out_sizes_buffer.writer->Write<u32>(state.size);

        (*out_loaded)++;
        (*out_count)++;
    }

    return RESULT_SUCCESS;
}

void IPlatformSharedResourceManager::LoadFont(const SharedFontType type) {
    auto file = GetSharedFontFile(type);
    if (!file)
        return;

    // Load
    auto stream = file->Open(filesystem::FileOpenFlags::Read);
    auto reader = stream.CreateReader();

    const auto res = DecryptBFTTF(
        reader, Writer((u8*)shared_memory->GetPtr() + shared_memory_offset,
                       SHARED_MEMORY_SIZE - shared_memory_offset));
    file->Close(stream);
    if (res != RESULT_SUCCESS)
        return;

    // Set state
    auto& state = states[(u32)type];
    state.shared_memory_offset = shared_memory_offset;
    state.size = file->GetSize();
    shared_memory_offset += file->GetSize();
}

} // namespace hydra::horizon::services::pl::shared_resource
