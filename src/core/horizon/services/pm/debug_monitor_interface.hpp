#pragma once

#include "core/horizon/services/const.hpp"

namespace hydra::horizon::services::pm {

class IDebugMonitorInterface : public IService {
  protected:
    result_t requestImpl([[maybe_unused]] RequestContext& context,
                         u32 id) override;

  private:
    // Commands
    result_t getProcessId(u64 program_id, u64* out_pid);
    result_t getApplicationProcessId(kernel::Process* process, u64* out_pid);
};

} // namespace hydra::horizon::services::pm
