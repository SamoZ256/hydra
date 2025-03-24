#include "horizon/filesystem/file.hpp"

namespace Hydra::Horizon::Filesystem {

File::~File() {
    if (stream) {
        LOG_WARNING(HorizonFilesystem, "File not closed properly");
        Close();
    }
}

void File::Open(usize& out_size) {
    ASSERT_DEBUG(!stream, HorizonFilesystem, "File is already open");
    stream = new std::ifstream(host_path,
                               std::ios::in | std::ios::binary | std::ios::ate);
    out_size = stream->tellg();
    stream->seekg(0, std::ios::beg);
}

void File::Close() {
    ASSERT_DEBUG(stream, HorizonFilesystem, "File not open");
    stream->close();
    delete stream;
    stream = nullptr;
}

} // namespace Hydra::Horizon::Filesystem
