#pragma once

#include "core/horizon/services/const.hpp"

namespace hydra::horizon::services::nfc {

enum class PermissionLevel {
    User,
    System,
};

class INfc : public IService {
  public:
    explicit INfc(PermissionLevel perm_level_) : perm_level{perm_level_} {}

  protected:
    // Commands
    result_t initialize(u64 aruid, u64 zero,
                        InBuffer<BufferAttr::MapAlias> in_version_buffer);
    result_t getState(u32* out_state);

  private:
    // TODO: use
    [[maybe_unused]] PermissionLevel perm_level;
};

} // namespace hydra::horizon::services::nfc
