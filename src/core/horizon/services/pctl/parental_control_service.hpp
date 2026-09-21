#pragma once

#include "core/horizon/services/const.hpp"

namespace hydra::horizon::services::pctl {

class IParentalControlService : public IService {
    friend class IParentalControlServiceFactory;

  protected:
    result_t requestImpl([[maybe_unused]] RequestContext& context,
                         u32 id) override;

  private:
    // Commands
    result_t initialize();
    result_t checkFreeCommunicationPermission();
};

} // namespace hydra::horizon::services::pctl
