#pragma once

#include "core/horizon/services/const.hpp"

namespace hydra::horizon::services::nfc::mifare {

class IUser : public ServiceBase {
  protected:
    result_t RequestImpl(RequestContext& context, u32 id) override;

  private:
    // Commands
    result_t Initialize(u64 aruid, u64 zero,
                        InBuffer<BufferAttr::MapAlias> in_version_buffer);
};

} // namespace hydra::horizon::services::nfc::mifare
