#pragma once

#include "core/horizon/services/const.hpp"

namespace hydra::horizon::services::ns {

class IApplicationManagerInterface : public IService {
  protected:
    result_t RequestImpl(RequestContext& context, u32 id) override;

  private:
    // Commands
    result_t
    ListApplicationRecord(i32 entry_offset, i32* out_entry_count,
                          OutBuffer<BufferAttr::MapAlias> out_record_buffer);
};

} // namespace hydra::horizon::services::ns
