#pragma once

#include "core/horizon/filesystem/file.hpp"
#include "core/horizon/services/const.hpp"

namespace hydra::horizon::services::fssrv {

class IFile : public IService {
  public:
    IFile(filesystem::IFile* file_, filesystem::FileOpenFlags flags);
    ~IFile() override;

  private:
    filesystem::IFile* file;
    ztd::io::IStream* stream;

    result_t RequestImpl([[maybe_unused]] RequestContext& context,
                         u32 id) override;

    // Commands
    result_t Read(Aligned<u32, 8> option, u64 offset, u64 size,
                  u64* out_written_size,
                  OutBuffer<BufferAttr::MapAlias> out_buffer);
    result_t Write(Aligned<u32, 8> option, u64 offset, u64 size,
                   InBuffer<BufferAttr::MapAlias> in_buffer);
    result_t Flush();
    result_t SetSize(u64 size);
    result_t GetSize(u64* out_size);
};

} // namespace hydra::horizon::services::fssrv
