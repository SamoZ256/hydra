#pragma once

#include "core/horizon/services/const.hpp"

namespace hydra::horizon::kernel {
class Event;
}

namespace hydra::horizon::services::friends {

class INotificationService : public IService {
  public:
    explicit INotificationService(uuid_t user_id_);

  protected:
    result_t RequestImpl([[maybe_unused]] RequestContext& context,
                         u32 id) override;

  private:
    // TODO: use
    [[maybe_unused]] uuid_t user_id;

    kernel::Event* event;

    // Commands
    result_t GetEvent(kernel::Process* process,
                      OutHandle<HandleAttr::Copy> out_handle); // 2.0.0+
    STUB_REQUEST_COMMAND(Clear);                               // 2.0.0+
};

} // namespace hydra::horizon::services::friends
