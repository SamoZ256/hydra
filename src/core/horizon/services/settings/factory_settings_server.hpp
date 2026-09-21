#pragma once

#include "core/horizon/services/const.hpp"

namespace hydra::horizon::services::settings {

class IFactorySettingsServer : public IService {
  protected:
    result_t requestImpl([[maybe_unused]] RequestContext& context,
                         u32 id) override;

  private:
    // Commands
    result_t
    getEciDeviceCertificate(OutBuffer<BufferAttr::MapAlias> out_buffer);
};

} // namespace hydra::horizon::services::settings
