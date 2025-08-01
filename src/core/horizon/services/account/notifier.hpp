#pragma once

#include "core/horizon/services/account/const.hpp"

namespace hydra::horizon::kernel {
class Event;
}

namespace hydra::horizon::services::account {

// TODO: actually implement notifications
class INotifier : public IService {
  public:
    INotifier();

  protected:
    result_t RequestImpl(RequestContext& context, u32 id) override;

  private:
    kernel::Event* event;

    // Commands
    result_t GetSystemEvent(kernel::Process* process,
                            OutHandle<HandleAttr::Copy> out_handle);
};

} // namespace hydra::horizon::services::account
