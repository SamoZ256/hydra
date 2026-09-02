#pragma once

#include "core/horizon/filesystem/file.hpp"
#include "core/horizon/services/const.hpp"

namespace hydra::horizon::services::fssrv {

// TODO: does IStorage always need to be backed by a file?
class IStorage : public IService {
  public:
    IStorage(filesystem::IFile* file_, filesystem::FileOpenFlags flags);
    ~IStorage() override;

  protected:
    result_t RequestImpl([[maybe_unused]] RequestContext& context,
                         u32 id) override;

    // Commands
    result_t Read(u64 offset, u64 size,
                  OutBuffer<BufferAttr::MapAlias> out_buffer);
    result_t Write(u64 offset, u64 size,
                   InBuffer<BufferAttr::MapAlias> in_buffer);
    STUB_REQUEST_COMMAND(Flush);
    result_t SetSize(u64 size);
    result_t GetSize(u64* out_size);

  private:
    filesystem::IFile* file;
    ztd::io::IStream* stream;
};

} // namespace hydra::horizon::services::fssrv
