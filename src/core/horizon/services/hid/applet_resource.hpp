#pragma once

#include "core/horizon/services/const.hpp"

namespace hydra::horizon::services::hid {

class IAppletResource : public ServiceBase {
  protected:
    result_t RequestImpl(RequestContext& context, u32 id) override;

  private:
    // Commands
    result_t GetSharedMemoryHandle(OutHandle<HandleAttr::Copy> out_handle);
};

} // namespace hydra::horizon::services::hid
