#include "core/horizon/filesystem/directory.hpp"

#include "core/horizon/filesystem/disk_file.hpp"

#define COMMON                                                                 \
    std::vector<std::string_view> broken_path;                                 \
    BreakPath(path, broken_path);

namespace hydra::horizon::filesystem {

Directory::Directory(const std::string_view host_path) {
    ASSERT(std::filesystem::is_directory(host_path), Filesystem,
           "\"{}\" is not a directory", host_path);

    for (const auto& entry : std::filesystem::directory_iterator(host_path)) {
        const auto& entry_path = entry.path().string();
        const auto entry_name =
            entry_path.substr(entry_path.find_last_of("/") + 1);

        // Ignore certain entries
        if (entry_name == ".DS_Store")
            continue;

        const auto res = AddEntry(entry_name, entry_path);
        ASSERT(res == FsResult::Success, Filesystem,
               "Failed to add entry \"{}\": {}", entry_name, res);
    }
}

Directory::~Directory() {
    // TODO: when should entries be deleted?
    // for (const auto [name, entry] : entries)
    //   delete entry;
}

void Directory::Save(std::string_view host_path) const {
    std::filesystem::create_directories(host_path);
    for (const auto& entry : entries) {
        if (!entry.second)
            continue;

        entry.second->Save(fmt::format("{}/{}", host_path, entry.first));
    }
}

FsResult Directory::Delete(bool recursive) {
    if (!recursive) {
        for (const auto& entry : entries) {
            if (entry.second && entry.second->IsDirectory())
                return FsResult::DirectoryNotEmpty;
        }
    }

    for (const auto& entry : entries) {
        if (!entry.second)
            continue;

        if (entry.second->IsDirectory()) {
            auto dir = static_cast<Directory*>(entry.second);
            const auto res = dir->Delete(true);
            if (res != FsResult::Success)
                return res;
            delete dir;
        } else {
            auto file = static_cast<IFile*>(entry.second);
            const auto res = file->Delete();
            if (res != FsResult::Success)
                return res;
            delete file;
        }
    }

    entries.clear();
    return FsResult::Success;
}

FsResult Directory::AddEntry(const std::string_view path, IEntry* entry,
                             bool add_intermediate) {
    COMMON;

    // TODO: handle this differently?
    // ASSERT(!broken_path.empty(), Filesystem, "Path cannot be empty");
    if (broken_path.empty()) {
        LOG_WARN(Filesystem, "Path cannot be empty");
        return FsResult::Success;
    }

    return AddEntryImpl(broken_path, entry, add_intermediate);
}

FsResult Directory::AddEntry(const std::string_view path,
                             const std::string_view host_path,
                             bool add_intermediate) {
    ASSERT(std::filesystem::exists(host_path), Filesystem,
           "Host path \"{}\" does not exist", host_path);

    IEntry* entry{nullptr};
    if (std::filesystem::is_directory(host_path)) {
        entry = new Directory(host_path);
    } else if (std::filesystem::is_regular_file(host_path)) {
        entry = new DiskFile(host_path, true);
    } else {
        LOG_ERROR(Filesystem, "Invalid host path \"{}\"", host_path);
    }

    return AddEntry(path, entry, add_intermediate);
}

FsResult Directory::DeleteEntry(const std::string_view path, bool recursive) {
    COMMON;
    ASSERT(!broken_path.empty(), Filesystem, "Path cannot be empty");
    return DeleteEntryImpl(broken_path, recursive);
}

FsResult Directory::GetEntry(const std::string_view path,
                             IEntry*& out_entry) const {
    COMMON;
    if (broken_path.empty()) {
        out_entry = const_cast<Directory*>(this);
        return FsResult::Success;
    }

    return GetEntryImpl(broken_path, out_entry);
}

FsResult Directory::GetFile(const std::string_view path,
                            IFile*& out_file) const {
    IEntry* entry;
    const auto res = GetEntry(path, entry);
    if (res != FsResult::Success)
        return res;

    if (!entry->IsFile())
        return FsResult::NotAFile;
    out_file = static_cast<IFile*>(entry);

    return FsResult::Success;
}

FsResult Directory::GetDirectory(const std::string_view path,
                                 Directory*& out_directory) const {
    IEntry* entry;
    const auto res = GetEntry(path, entry);
    if (res != FsResult::Success)
        return res;

    if (!entry->IsDirectory())
        return FsResult::NotADirectory;
    out_directory = static_cast<Directory*>(entry);

    return FsResult::Success;
}

FsResult Directory::AddEntryImpl(const std::span<std::string_view> path,
                                 IEntry* entry, bool add_intermediate) {
    const auto entry_name = path[0];
    auto& e = entries[std::string(entry_name)];
    if (path.size() == 1) {
        if (e)
            return FsResult::AlreadyExists;

        entry->SetParent(this);
        e = entry;
        return FsResult::Success;
    } else {
        if (!e) {
            if (add_intermediate) {
                e = new Directory();
                e->SetParent(this);
            } else {
                return FsResult::DoesNotExist;
            }
        }

        if (!e->IsDirectory())
            return FsResult::NotADirectory;
        auto sub_dir = static_cast<Directory*>(e);

        return sub_dir->AddEntryImpl(path.subspan(1), entry, add_intermediate);
    }
}

FsResult Directory::DeleteEntryImpl(const std::span<std::string_view> path,
                                    bool recursive) {
    const auto entry_name = path[0];
    auto it = entries.find(std::string(entry_name));
    if (path.size() == 1) {
        if (it == entries.end())
            return FsResult::DoesNotExist;

        auto res = it->second->Delete(recursive);
        if (res != FsResult::Success)
            return res;

        delete it->second;
        entries.erase(it);

        return FsResult::Success;
    } else {
        if (it == entries.end())
            return FsResult::DoesNotExist;

        if (!it->second->IsDirectory())
            return FsResult::NotADirectory;
        auto sub_dir = static_cast<Directory*>(it->second);

        return sub_dir->DeleteEntryImpl(path.subspan(1), recursive);
    }
}

FsResult Directory::GetEntryImpl(const std::span<std::string_view> path,
                                 IEntry*& out_entry) const {
    const auto entry_name = path[0];
    auto it = entries.find(std::string(entry_name));
    if (path.size() == 1) {
        if (it == entries.end())
            return FsResult::DoesNotExist;

        out_entry = it->second;
        return FsResult::Success;
    } else {
        if (it == entries.end())
            return FsResult::DoesNotExist;

        if (!it->second->IsDirectory())
            return FsResult::NotADirectory;
        auto sub_dir = static_cast<Directory*>(it->second);

        return sub_dir->GetEntryImpl(path.subspan(1), out_entry);
    }
}

void Directory::BreakPath(std::string_view path,
                          std::vector<std::string_view>& out_path) const {
    // Reserve the maximum possible count
    out_path.reserve(
        static_cast<usize>(std::count(path.begin(), path.end(), '/')));

    while (!path.empty()) {
        const auto slash_pos = path.find('/');

        const auto entry = path.substr(0, slash_pos);
        if (entry == "." || entry.empty()) {
            // Do nothing
        } else if (entry == "..") {
            out_path.pop_back();
        } else {
            // TODO: verify the name doesn't contain forbidden characters
            out_path.push_back(entry);
        }

        if (slash_pos != std::string::npos)
            path = path.substr(slash_pos + 1);
        else
            break;
    }
}

} // namespace hydra::horizon::filesystem
