#include "core/horizon/filesystem/disk_file.hpp"

namespace hydra::horizon::filesystem {

DiskFile::DiskFile(const std::string_view path_, bool is_mutable_)
    : path{path_}, is_mutable{is_mutable_} {
    if (std::filesystem::exists(path)) {
        // size = std::filesystem::file_size(host_path);
    } else {
        ASSERT_THROWING(is_mutable, Filesystem,
                        InitError::ImmutableFileDoesNotExist,
                        "Immutable file \"{}\" does not exist", path);

        // Intermediate directories
        std::filesystem::create_directories(
            std::filesystem::path(path).parent_path());

        // Empty file
        // TODO: is there a better way to create an empty file?
        std::ofstream ofs(path);
        ofs.close();
        // std::filesystem::resize_file(host_path, size);

        LOG_FS_ACCESS(path, "file created");
    }
}

DiskFile::~DiskFile() {
    // Resize the file to the requested size
    // if (is_mutable && std::filesystem::exists(host_path)) {
    //    if (std::filesystem::file_size(host_path) != size)
    //        std::filesystem::resize_file(host_path, size);
    //}
}

void DiskFile::Resize(usize new_size) {
    ASSERT(is_mutable, Filesystem, "Immutable file cannot be resized");

    // size = new_size;
    std::filesystem::resize_file(path, new_size);

    LOG_FS_ACCESS(path, "file resized (size: {})", new_size);
}

void DiskFile::Flush() {
    // ASSERT(is_mutable, Filesystem, "Immutable file cannot be flushed");

    // Flush the file size
    // std::filesystem::resize_file(host_path, size);

    // LOG_FS_ACCESS(host_path, "file flushed");
}

io::IStream* DiskFile::Open(FileOpenFlags flags) {
    std::ios::openmode std_flags = std::ios::binary;
    if (any(flags & FileOpenFlags::Read))
        std_flags |= std::ios::in;
    if (any(flags & FileOpenFlags::Write))
        std_flags |= std::ios::out;
    if (any(flags & FileOpenFlags::Append))
        std_flags |= std::ios::app;

    return new DiskStream(path, std_flags);
}

usize DiskFile::GetSize() {
    // return size;
    return std::filesystem::file_size(path);
}

void DiskFile::DeleteImpl() {
    std::filesystem::remove(path);

    LOG_FS_ACCESS(path, "file deleted");
}

} // namespace hydra::horizon::filesystem
