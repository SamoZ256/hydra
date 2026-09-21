#pragma once

#include "core/horizon/services/const.hpp"
#include "core/horizon/services/fssrv/const.hpp"

namespace hydra::horizon::services::fssrv {

class ISaveDataInfoReader : public IService {
  protected:
    result_t requestImpl([[maybe_unused]] RequestContext& context,
                         u32 id) override;

  private:
    u32 entry_index{0};

    // Commands
    result_t
    readSaveDataInfo(i64* out_entry_count,
                     OutBuffer<BufferAttr::MapAlias> out_entry_buffer) const;
};

} // namespace hydra::horizon::services::fssrv
