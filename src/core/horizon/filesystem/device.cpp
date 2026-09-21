#include "core/horizon/filesystem/device.hpp"

#include "core/horizon/filesystem/directory.hpp"

#define VERIFY_PATH(path)                                                      \
    if (path.empty() || path[0] != '/')                                        \
        return FsResult::DoesNotExist;

namespace hydra::horizon::filesystem {

Device::~Device() { delete root; }

FsResult Device::addEntry(const std::string_view path, IEntry* entry,
                          bool add_intermediate) {
    VERIFY_PATH(path);
    return root->addEntry(path.substr(1), entry, add_intermediate);
}

FsResult Device::addEntry(const std::string_view path,
                          const std::string_view host_path,
                          bool add_intermediate) {
    VERIFY_PATH(path);
    return root->addEntry(path.substr(1), host_path, add_intermediate);
}

FsResult Device::deleteEntry(const std::string_view path, bool recursive) {
    VERIFY_PATH(path);
    return root->deleteEntry(path.substr(1), recursive);
}

FsResult Device::getEntry(const std::string_view path, IEntry*& out_entry) {
    VERIFY_PATH(path);
    return root->getEntry(path.substr(1), out_entry);
}

FsResult Device::getFile(const std::string_view path, IFile*& out_file) {
    VERIFY_PATH(path);
    return root->getFile(path.substr(1), out_file);
}

FsResult Device::getDirectory(const std::string_view path,
                              Directory*& out_directory) {
    VERIFY_PATH(path);
    return root->getDirectory(path.substr(1), out_directory);
}

} // namespace hydra::horizon::filesystem
