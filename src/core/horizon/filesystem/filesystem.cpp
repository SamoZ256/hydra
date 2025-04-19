#include "core/horizon/filesystem/filesystem.hpp"

#include "core/horizon/filesystem/directory.hpp"
#include "core/horizon/filesystem/ram_file.hpp"

#define VERIFY_PATH(path)                                                      \
    if (path.empty())                                                          \
        return FsResult::NotMounted;                                           \
    const auto slash_pos = path.find('/');                                     \
    if (slash_pos == std::string::npos)                                        \
        return FsResult::NotMounted;                                           \
    const auto mount = path.substr(0, slash_pos);                              \
    if (mount.empty())                                                         \
        return FsResult::NotMounted;                                           \
    const auto entry_path = path.substr(slash_pos);                            \
    auto it = devices.find(mount);                                             \
    if (it == devices.end())                                                   \
        return FsResult::NotMounted;                                           \
    auto& device = it->second;

#define VERIFY_MOUNT(mount)                                                    \
    ASSERT(mount.find("/") == std::string::npos, HorizonFilesystem,            \
           "Invalid mount point \"{}\"", mount);

namespace Hydra::Horizon::Filesystem {

SINGLETON_DEFINE_GET_INSTANCE(Filesystem, HorizonFilesystem, "Filesystem")

Filesystem::Filesystem() {
    SINGLETON_SET_INSTANCE(HorizonFilesystem, "Filesystem");
}

Filesystem::~Filesystem() { SINGLETON_UNSET_INSTANCE(); }

void Filesystem::Mount(const std::string& mount) {
    MountImpl(mount, new Directory());
}

void Filesystem::Mount(const std::string& mount, const std::string& root_path) {
    Directory* root;
    auto res = GetDirectory(root_path, root);
    ASSERT(res == FsResult::Success, HorizonFilesystem,
           "Failed to get root directory \"{}\" for mount \"{}\"", root_path,
           mount);
    MountImpl(mount, root);
}

FsResult Filesystem::AddEntry(const std::string& path, EntryBase* entry,
                              bool add_intermediate) {
    VERIFY_PATH(path);
    return device.AddEntry(entry_path, entry, add_intermediate);
}

FsResult Filesystem::AddEntry(const std::string& path,
                              const std::string& host_path,
                              bool add_intermediate) {
    VERIFY_PATH(path);
    return device.AddEntry(entry_path, host_path, add_intermediate);
}

FsResult Filesystem::CreateFile(const std::string& path,
                                bool add_intermediate) {
    // TODO: don't always create RamFiles
    return AddEntry(path, new RamFile(), add_intermediate);
}

FsResult Filesystem::GetEntry(const std::string& path, EntryBase*& out_entry) {
    VERIFY_PATH(path);
    return device.GetEntry(entry_path, out_entry);
}

FsResult Filesystem::GetFile(const std::string& path, FileBase*& out_file) {
    EntryBase* entry;
    const auto res = GetEntry(path, entry);
    if (res != FsResult::Success)
        return res;

    out_file = dynamic_cast<FileBase*>(entry);
    if (!out_file)
        return FsResult::NotAFile;

    return res;
}

FsResult Filesystem::GetDirectory(const std::string& path,
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

void Filesystem::MountImpl(const std::string& mount, Directory* root) {
    VERIFY_MOUNT(mount);
    devices.emplace(std::make_pair(mount, root));
    LOG_INFO(HorizonFilesystem, "Mounted \"{}\"", mount);
}

} // namespace Hydra::Horizon::Filesystem
