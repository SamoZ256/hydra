#pragma once

#include "core/horizon/services/const.hpp"

namespace hydra::horizon::services::nfp {

enum class PermissionLevel {
    Debug,
    User,
    System,
};

class INfp : public IService {
  public:
    explicit INfp(PermissionLevel perm_level_);

  private:
    // TODO: use
    [[maybe_unused]] PermissionLevel perm_level;

  protected:
    kernel::Event* availability_change_event;

    // Commands
    result_t initialize(u64 aruid, u64 zero,
                        InBuffer<BufferAttr::MapAlias> in_version_buffer);
    // TODO: is the buffer attr correct?
    result_t listDevices(i32* out_count,
                         OutBuffer<BufferAttr::HipcPointer> out_buffer);
    result_t getState(u32* out_state);
    result_t
    attachAvailabilityChangeEvent(kernel::Process* process,
                                  OutHandle<HandleAttr::Copy> out_handle);
};

} // namespace hydra::horizon::services::nfp
