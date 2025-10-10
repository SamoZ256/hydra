#pragma once

#include "core/horizon/services/const.hpp"

namespace hydra::horizon::services::pdm {

class IQueryService : public IService {
  protected:
    result_t RequestImpl(RequestContext& context, u32 id) override;

  private:
    // Commands
    result_t QueryPlayEvent(i32 entry_index, i32* out_total_entries,
                            OutBuffer<BufferAttr::MapAlias> out_buffer);
    result_t
    QueryAccountPlayEvent(i32 entry_index, uuid_t user_id,
                          i32* out_total_entries,
                          OutBuffer<BufferAttr::MapAlias> out_buffer); // 4.0.0+
};

} // namespace hydra::horizon::services::pdm
