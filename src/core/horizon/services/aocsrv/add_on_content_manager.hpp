#pragma once

#include "core/horizon/services/const.hpp"

namespace hydra::horizon::services::aocsrv {

class IAddOnContentManager : public ServiceBase {
  protected:
    result_t RequestImpl(RequestContext& context, u32 id) override;

  private:
    // Commands
    // TODO: correct?
    result_t ListAddOnContent(u32 start_index, u32 buffer_size, u32* out_count,
                              OutBuffer<BufferAttr::MapAlias> out_buffer);
};

} // namespace hydra::horizon::services::aocsrv
