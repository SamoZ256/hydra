#pragma once

#include "core/horizon/services/const.hpp"

namespace hydra::horizon::services::sm {

class IUserInterface : public IService {
  protected:
    result_t RequestImpl([[maybe_unused]] RequestContext& context,
                         u32 id) override;

  private:
    // Commands
    STUB_REQUEST_COMMAND(RegisterClient);
    result_t GetServiceHandle(System* system, kernel::Process* process,
                              u64 name, OutHandle<HandleAttr::Move> out_handle);
    result_t RegisterService(System* system, kernel::Process* process, u64 name,
                             bool is_light, i32 max_sessions,
                             OutHandle<HandleAttr::Move> out_port_handle);

    // Atmosphere commands
    result_t AtmosphereHasService(System* system, u64 name,
                                  bool* out_has_service);
    result_t AtmosphereWaitService(u64 name);
};

} // namespace hydra::horizon::services::sm
