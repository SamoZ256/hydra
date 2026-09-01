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
    io::FileStream out_stream(std::move(file));

    // Read
    const auto stream =
        const_cast<IFile*>(this)->Open(FileOpenFlags::Read); // HACK

    std::array<u8, 0x800> buffer;
    while (stream->GetSeek() < stream->GetSize()) {
        std::span<u8> span(buffer.data(),
                           buffer.data() +
                               std::min(static_cast<u64>(buffer.size()),
                                        stream->GetSize() - stream->GetSeek()));
        stream->ReadToSpan(span);
        out_stream.WriteSpan(std::span<const u8>(span));
    }

    delete stream;
}

} // namespace hydra::horizon::filesystem
