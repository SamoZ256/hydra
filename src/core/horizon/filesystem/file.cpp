#include "core/horizon/filesystem/file.hpp"

namespace hydra::horizon::filesystem {

void IFile::Save(std::string_view host_path) const {
    // Create file
    // HACK: construct a temporary string
    ZTD_ASSIGN_OR(auto file,
                  ztd::fs::openFileAbsolute(std::string(host_path),
                                            ztd::fs::File::OpenFlags::Write),
                  {
                      LOG_ERROR(Services, "Failed to write user at path {}",
                                host_path);
                      return;
                  });
    ztd::io::FileStream out_stream(file);

    // Read
    const auto stream =
        const_cast<IFile*>(this)->Open(FileOpenFlags::Read); // HACK

    std::array<u8, 0x800> buffer;
    while (stream->getSeek() < stream->getSize()) {
        std::span<u8> span(buffer.data(),
                           buffer.data() +
                               std::min(static_cast<u64>(buffer.size()),
                                        stream->getSize() - stream->getSeek()));
        stream->readToSpan(span);
        out_stream.writeSpan(std::span<const u8>(span));
    }

    delete stream;
}

} // namespace hydra::horizon::filesystem
