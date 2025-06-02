#pragma once

#include "core/horizon/services/const.hpp"

namespace hydra::horizon::services::settings {

enum class ColorSetId : i32 {
    BasicWhite,
    BasicBlack,
};

class ISystemSettingsServer : public ServiceBase {
  protected:
    result_t RequestImpl(RequestContext& context, u32 id) override;

  private:
    // Commands
    result_t GetFirmwareVersion(OutBuffer<BufferAttr::HipcPointer> out_buffer);
    result_t GetColorSetId(ColorSetId* out_id);
};

} // namespace hydra::horizon::services::settings
