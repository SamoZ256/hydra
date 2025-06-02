#pragma once

#include "core/horizon/services/const.hpp"

namespace hydra::horizon::services::pctl {

class IParentalControlService : public ServiceBase {
    friend class IParentalControlServiceFactory;

  protected:
    result_t RequestImpl(RequestContext& context, u32 id) override;

  private:
    // Commands
    result_t Initialize();
    result_t CheckFreeCommunicationPermission();
};

} // namespace hydra::horizon::services::pctl
