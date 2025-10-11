#include "core/horizon/avatar_image_loader.hpp"

#include <yaz0.h>

#include "core/horizon/filesystem/content_archive.hpp"
#include "core/horizon/filesystem/filesystem.hpp"
#include "core/horizon/filesystem/host_file.hpp"
#include "core/horizon/filesystem/romfs.hpp"

namespace hydra::horizon {

namespace {

constexpr usize AVATAR_UNCOMPRESSED_IMAGE_SIZE = 0x40000;

}

void AvatarImageLoader::LoadSystemAvatars(filesystem::Filesystem& fs) {
    // NCA
    filesystem::FileBase* file;
    auto res = fs.GetFile(FS_FIRMWARE_PATH "/AvatarImage", file);
    if (res != filesystem::FsResult::Success) {
        LOG_ERROR(Services, "Failed to get avatars file: {}", res);
        return;
    }

    auto content_archive = new filesystem::ContentArchive(file);

    // Data
    filesystem::FileBase* data_file;
    res = content_archive->GetFile("data", data_file);
    if (res != filesystem::FsResult::Success) {
        LOG_ERROR(Services, "Failed to get avatars data: {}", res);
        return;
    }

    // RomFS
    auto romfs = new filesystem::RomFS(data_file);

    // Background
    // Not necessary

    // Characters
    filesystem::Directory* character_dir;
    res = romfs->GetDirectory("chara", character_dir);
    ASSERT(res == filesystem::FsResult::Success, Services,
           "Failed to get \"chara\" avatars directory: {}", res);
    for (const auto& [name, entry] : character_dir->GetEntries()) {
        if (name.ends_with(".szs"))
            images[name] = static_cast<filesystem::FileBase*>(entry);
    }
}

void AvatarImageLoader::LoadAvatarImage(const std::string& path,
                                        std::vector<u8>& out_data) const {
    // Decompress
    auto file = images.at(path);

    auto stream = file->Open(filesystem::FileOpenFlags::Read);
    auto reader = stream.CreateReader();

    std::vector<u8> compressed(reader.GetSize());
    reader.ReadWhole<u8>(compressed.data());

    file->Close(stream);

#define YAZ0_ASSERT(expr)                                                      \
    {                                                                          \
        const auto res = expr;                                                 \
        ASSERT(res == YAZ0_OK, Services, #expr " failed: {}", res);            \
    }
    Yaz0Stream* yaz0;
    YAZ0_ASSERT(yaz0Init(&yaz0));
    YAZ0_ASSERT(yaz0ModeDecompress(yaz0));
    YAZ0_ASSERT(yaz0Input(yaz0, compressed.data(), compressed.size()));
    out_data.resize(AVATAR_UNCOMPRESSED_IMAGE_SIZE);
    YAZ0_ASSERT(
        yaz0Output(yaz0, out_data.data(), AVATAR_UNCOMPRESSED_IMAGE_SIZE));
    YAZ0_ASSERT(yaz0Run(yaz0));
    YAZ0_ASSERT(yaz0Destroy(yaz0));
#undef YAZ0_ASSERT
}

} // namespace hydra::horizon
