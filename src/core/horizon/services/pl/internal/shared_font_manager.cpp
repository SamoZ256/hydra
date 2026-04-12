#include "core/horizon/services/pl/internal/shared_font_manager.hpp"

#include "core/horizon/filesystem/content_archive.hpp"
#include "core/horizon/filesystem/file.hpp"
#include "core/horizon/filesystem/filesystem.hpp"
#include "core/horizon/filesystem/partition_filesystem.hpp"
#include "core/horizon/filesystem/romfs/romfs.hpp"

namespace hydra::horizon::services::pl::internal {

namespace {

constexpr usize SHARED_MEMORY_SIZE = 0x01100000;

struct SharedFontName {
    std::string_view name;
    std::string_view filename;
};

#define SHARED_FONT_ENTRY(type, name, filename)                                \
    SharedFontName{name, filename ".bfttf"}

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

filesystem::IFile* GetSharedFontFile(SharedFontType font_type) {
    const auto& name = shared_font_names[(u32)font_type];

    // NCA
    filesystem::IFile* file;
    auto res = KERNEL_INSTANCE.GetFilesystem().GetFile(
        fmt::format(FS_FIRMWARE_PATH "/{}", name.name), file);
    if (res != filesystem::FsResult::Success) {
        LOG_ERROR(Services, "Failed to get shared font {} file: {}", font_type,
                  res);
        return nullptr;
    }

    filesystem::ContentArchive content_archive(file);

    // Data
    filesystem::IFile* data_file;
    res = content_archive.GetFile("data", data_file);
    if (res != filesystem::FsResult::Success) {
        LOG_ERROR(Services, "Failed to get shared font {} data: {}", font_type,
                  res);
        return nullptr;
    }

    // RomFS
    filesystem::romfs::RomFS romfs(data_file);

    // Font
    filesystem::IFile* font_file;
    res = romfs.GetFile(name.filename, font_file);
    if (res != filesystem::FsResult::Success) {
        LOG_ERROR(Services, "Failed to get shared font {}: {}", font_type, res);
        return nullptr;
    }

    return font_file;
}

constexpr u32 BFTTF_MAGIC = 0x18029a7f;
constexpr u32 FONT_KEY = 0x06186249;

result_t DecryptBFTTF(io::IStream* in_stream, io::IStream* out_stream) {
#define KEY_XOR(x) (x ^ FONT_KEY)

    const auto magic = KEY_XOR(in_stream->Read<u32>());
    if (magic != BFTTF_MAGIC) {
        LOG_ERROR(Services, "Invalid BFTTF magic");
        return MAKE_RESULT(Svc, 100); // TODO
    }

    in_stream->SeekBy(4);

    for (u32 i = 0; i < (in_stream->GetSize() - 8) / sizeof(u32); i++)
        out_stream->Write(KEY_XOR(in_stream->Read<u32>()));

#undef KEY_XOR

    return RESULT_SUCCESS;
}

} // namespace

SharedFontManager::SharedFontManager()
    : shared_memory{new kernel::SharedMemory(SHARED_MEMORY_SIZE)} {}

SharedFontManager::~SharedFontManager() { delete shared_memory; }

void SharedFontManager::LoadFonts() {
    for (SharedFontType type = SharedFontType(0); type < SharedFontType::Total;
         type++)
        LoadFont(type);
}

void SharedFontManager::LoadFont(const SharedFontType type) {
    auto file = GetSharedFontFile(type);
    if (!file)
        return;

    // Load
    auto stream = file->Open(filesystem::FileOpenFlags::Read);

    io::MemoryStream out_stream(std::span(
        reinterpret_cast<u8*>(shared_memory->GetPtr()) + shared_memory_offset,
        SHARED_MEMORY_SIZE - shared_memory_offset));
    const auto res = DecryptBFTTF(stream, &out_stream);

    delete stream;
    if (res != RESULT_SUCCESS)
        return;

    // Set state
    auto& state = states[u32(type)];
    state.shared_memory_offset = shared_memory_offset;
    state.size = file->GetSize();
    shared_memory_offset += file->GetSize();
}

} // namespace hydra::horizon::services::pl::internal
