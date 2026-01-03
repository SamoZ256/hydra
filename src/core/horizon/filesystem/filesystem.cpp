#include "core/horizon/filesystem/filesystem.hpp"

#include "core/horizon/filesystem/directory.hpp"
#include "core/horizon/filesystem/disk_file.hpp"
#include "core/horizon/filesystem/memory_file.hpp"

#define GET_MOUNT(path)                                                        \
    if (path.empty())                                                          \
        return FsResult::NotMounted;                                           \
    const auto slash_pos = path.find('/');                                     \
    if (slash_pos == std::string::npos)                                        \
        return FsResult::NotMounted;                                           \
    const auto mount = path.substr(0, slash_pos);                              \
    if (mount.empty())                                                         \
        return FsResult::NotMounted;                                           \
    const auto entry_path = path.substr(slash_pos);

#define VERIFY_PATH(path)                                                      \
    GET_MOUNT(path)                                                            \
    auto it = devices.find(std::string(mount));                                \
    if (it == devices.end())                                                   \
        return FsResult::NotMounted;                                           \
    auto& device = it->second;

#define VERIFY_MOUNT(mount)                                                    \
    ASSERT(mount.find("/") == std::string::npos, Filesystem,                   \
           "Invalid mount point \"{}\"", mount);

namespace hydra::horizon::filesystem {

Filesystem::Filesystem() {
    // SD card
    std::filesystem::create_directories(CONFIG_INSTANCE.GetSdCardPath());
    MountImpl(FS_SD_MOUNT, new Directory(CONFIG_INSTANCE.GetSdCardPath()));

    // Save
    std::filesystem::create_directories(CONFIG_INSTANCE.GetSavePath());
    MountImpl(FS_SAVE_MOUNT, new Directory(CONFIG_INSTANCE.GetSavePath()));

    // Cache
    // TODO: support mounting to a real host path as well
    Mount(FS_CACHE_MOUNT);

    // Content
    Mount(FS_CONTENT_MOUNT);
}

void Filesystem::Mount(const std::string_view mount) {
    MountImpl(mount, new Directory());
}

void Filesystem::Mount(const std::string_view mount,
                       const std::string_view root_path) {
    Directory* root;
    auto res = GetDirectory(root_path, root);
    ASSERT(res == FsResult::Success, Filesystem,
           "Failed to get root directory \"{}\" for mount \"{}\"", root_path,
           mount);
    MountImpl(mount, root);
}

FsResult Filesystem::AddEntry(const std::string_view path, IEntry* entry,
                              bool add_intermediate) {
    VERIFY_PATH(path);
    return device.AddEntry(entry_path, entry, add_intermediate);
}

FsResult Filesystem::AddEntry(const std::string_view path,
                              const std::string_view host_path,
                              bool add_intermediate) {
    VERIFY_PATH(path);
    return device.AddEntry(entry_path, host_path, add_intermediate);
}

FsResult Filesystem::CreateFile(const std::string_view path, usize size,
                                bool add_intermediate) {
    GET_MOUNT(path);

    // TODO: keep a list of host paths for each mount point instead
    if (mount == FS_SD_MOUNT) {
        const auto host_path =
            fmt::format("{}{}", CONFIG_INSTANCE.GetSdCardPath(), entry_path);
        auto file = new DiskFile(host_path, true);
        file->Resize(size);
        return AddEntry(path, file, add_intermediate);
    } else if (mount == FS_SAVE_MOUNT) {
        const auto host_path =
            fmt::format("{}{}", CONFIG_INSTANCE.GetSavePath(), entry_path);
        auto file = new DiskFile(host_path, true);
        file->Resize(size);
        return AddEntry(path, file, add_intermediate);
    } else {
        LOG_WARN(Filesystem,
                 "Could not find host path for path \"{}\", falling back to "
                 "memory backed file",
                 path);
        return AddEntry(path, new MemoryFile(size), add_intermediate);
    }
}

FsResult Filesystem::CreateDirectory(const std::string_view path,
                                     bool add_intermediate) {
    return AddEntry(path, new Directory(), add_intermediate);
}

FsResult Filesystem::DeleteEntry(const std::string_view path, bool recursive) {
    VERIFY_PATH(path);
    return device.DeleteEntry(entry_path, recursive);
}

FsResult Filesystem::GetEntry(const std::string_view path, IEntry*& out_entry) {
    VERIFY_PATH(path);
    return device.GetEntry(entry_path, out_entry);
}

FsResult Filesystem::GetFile(const std::string_view path, IFile*& out_file) {
    VERIFY_PATH(path);
    return device.GetFile(entry_path, out_file);
}

FsResult Filesystem::GetDirectory(const std::string_view path,
                                  Directory*& out_directory) {
    VERIFY_PATH(path);
    return device.GetDirectory(entry_path, out_directory);
}

void Filesystem::MountImpl(const std::string_view mount, Directory* root) {
    VERIFY_MOUNT(mount);
    devices.emplace(std::make_pair(mount, root));
    LOG_INFO(Filesystem, "Mounted \"{}\"", mount);
}

} // namespace hydra::horizon::filesystem
