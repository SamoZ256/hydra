#pragma once

#include "core/horizon/services/const.hpp"

#include "core/horizon/services/fssrv/const.hpp"

namespace hydra::horizon::filesystem {
class Directory;
}

namespace hydra::horizon::services::fssrv {

class IDirectory : public ServiceBase {
  public:
    IDirectory(filesystem::Directory* directory_,
               DirectoryFilterFlags filter_flags_)
        : directory{directory_}, filter_flags{filter_flags_} {}

  protected:
    result_t RequestImpl(RequestContext& context, u32 id) override;

  private:
    filesystem::Directory* directory;
    DirectoryFilterFlags filter_flags;

    u32 entry_index{0};

    // Commands
    result_t Read(u64* out_entry_count,
                  OutBuffer<BufferAttr::MapAlias> out_entries);
    result_t GetEntryCount(u64* out_count);
};

} // namespace hydra::horizon::services::fssrv
