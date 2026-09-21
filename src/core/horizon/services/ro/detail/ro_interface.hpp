#pragma once

#include "core/horizon/services/const.hpp"

namespace hydra::horizon::services::ro::detail {

class IRoInterface : public IService {
  protected:
    result_t requestImpl([[maybe_unused]] RequestContext& context,
                         u32 id) override;

  private:
    // Commands
    // TODO: PID
    result_t mapManualLoadModuleMemory(kernel::Process* process,
                                       u64 pid_reserved, u64 nro_addr,
                                       u64 nro_size, u64 bss_addr, u64 bss_size,
                                       u64* out_addr);
    STUB_REQUEST_COMMAND(registerModuleInfo);
    STUB_REQUEST_COMMAND(registerProcessHandle);
};

} // namespace hydra::horizon::services::ro::detail
