#pragma once

#include "core/horizon/services/const.hpp"

namespace hydra::horizon::services::aocsrv {

class IAddOnContentManager : public IService {
  protected:
    result_t requestImpl([[maybe_unused]] RequestContext& context,
                         u32 id) override;

  private:
    // Commands
    result_t countAddOnContent(i64* out_count);
    // TODO: correct?
    result_t listAddOnContent(u32 start_index, u32 buffer_size, u32* out_count,
                              OutBuffer<BufferAttr::MapAlias> out_buffer);
    STUB_REQUEST_COMMAND(checkAddOnContentMountStatus);
};

} // namespace hydra::horizon::services::aocsrv
