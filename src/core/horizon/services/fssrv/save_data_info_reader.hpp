#pragma once

#include "core/horizon/services/const.hpp"
#include "core/horizon/services/fssrv/const.hpp"

namespace hydra::horizon::services::fssrv {

class ISaveDataInfoReader : public ServiceBase {
  protected:
    result_t RequestImpl(RequestContext& context, u32 id) override;

  private:
    u32 entry_index{0};

    // Commands
    result_t ReadSaveDataInfo(i64* out_entry_count,
                              OutBuffer<BufferAttr::MapAlias> out_entry_buffer);
};

} // namespace hydra::horizon::services::fssrv
