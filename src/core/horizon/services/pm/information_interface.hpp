#pragma once

#include "core/horizon/services/const.hpp"

namespace hydra::horizon::services::pm {

class IInformationInterface : public IService {
  protected:
    result_t requestImpl([[maybe_unused]] RequestContext& context,
                         u32 id) override;

  private:
    // Commands
    result_t getProgramId(kernel::Process* process, u64* out_program_id);
};

} // namespace hydra::horizon::services::pm
