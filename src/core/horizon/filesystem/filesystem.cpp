#include "core/horizon/filesystem/filesystem.hpp"

#include "core/horizon/filesystem/directory.hpp"
#include "core/horizon/filesystem/host_file.hpp"
#include "core/horizon/filesystem/ram_file.hpp"

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

SINGLETON_DEFINE_GET_INSTANCE(Filesystem, Filesystem)

Filesystem::Filesystem() {
    SINGLETON_SET_INSTANCE(Filesystem, Filesystem);

    // SD card
    std::filesystem::create_directories(CONFIG_INSTANCE.GetSdCardPath().Get());
    MountImpl(FS_SD_MOUNT,
              new Directory(CONFIG_INSTANCE.GetSdCardPath().Get()));

    // Save
    std::filesystem::create_directories(CONFIG_INSTANCE.GetSavePath().Get());
    MountImpl(FS_SAVE_MOUNT,
              new Directory(CONFIG_INSTANCE.GetSavePath().Get()));

    // Cache
    // TODO: support mounting to a real host path as well
    Mount(FS_CACHE_MOUNT);
}

Filesystem::~Filesystem() { SINGLETON_UNSET_INSTANCE(); }

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

FsResult Filesystem::AddEntry(const std::string_view path, EntryBase* entry,
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
        const auto host_path = fmt::format(
            "{}{}", CONFIG_INSTANCE.GetSdCardPath().Get(), entry_path);
        return AddEntry(path, new HostFile(host_path, size), add_intermediate);
    } else if (mount == FS_SAVE_MOUNT) {
        const auto host_path = fmt::format(
            "{}{}", CONFIG_INSTANCE.GetSavePath().Get(), entry_path);
        return AddEntry(path, new HostFile(host_path, size), add_intermediate);
    } else {
        LOG_WARN(Filesystem,
                 "Could not find host path for path \"{}\", falling back to "
                 "RAM backed file",
                 path);
        return AddEntry(path, new RamFile(size), add_intermediate);
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

FsResult Filesystem::GetEntry(const std::string_view path,
                              EntryBase*& out_entry) {
    VERIFY_PATH(path);
    return device.GetEntry(entry_path, out_entry);
}

FsResult Filesystem::GetFile(const std::string_view path, FileBase*& out_file) {
    EntryBase* entry;
    const auto res = GetEntry(path, entry);
    if (res != FsResult::Success)
        return res;

    out_file = dynamic_cast<FileBase*>(entry);
    if (!out_file)
        return FsResult::NotAFile;

    return res;
}

FsResult Filesystem::GetDirectory(const std::string_view path,
                                  Directory*& out_directory) {
    EntryBase* entry;
    const auto res = GetEntry(path, entry);
    if (res != FsResult::Success)
        return res;

    out_directory = dynamic_cast<Directory*>(entry);
    if (!out_directory)
        return FsResult::NotADirectory;

    return res;
}

void Filesystem::MountImpl(const std::string_view mount, Directory* root) {
    VERIFY_MOUNT(mount);
    devices.emplace(std::make_pair(mount, root));
    LOG_INFO(Filesystem, "Mounted \"{}\"", mount);
}

} // namespace hydra::horizon::filesystem
