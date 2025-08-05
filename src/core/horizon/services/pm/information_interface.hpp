#pragma once

#include "core/horizon/services/const.hpp"

namespace hydra::horizon::services::pm {

class IInformationInterface : public IService {
  protected:
    result_t RequestImpl(RequestContext& context, u32 id) override;

  private:
    // Commands
    result_t GetProgramId(kernel::Process* process, u64* out_program_id);
};

} // namespace hydra::horizon::services::pm
