#pragma once

#include "core/horizon/kernel/kernel.hpp"
#include "core/horizon/services/const.hpp"

namespace hydra::horizon::services::am {

class IHomeMenuFunctions : public IService {
  public:
    IHomeMenuFunctions()
        : pop_from_general_channel_event{
              new kernel::Event(false, "Pop from general channel event")} {}

  protected:
    result_t requestImpl([[maybe_unused]] RequestContext& context,
                         u32 id) override;

  private:
    kernel::Event* pop_from_general_channel_event;

    // Commands
    STUB_REQUEST_COMMAND(requestToGetForeground);
    result_t
    getPopFromGeneralChannelEvent(kernel::Process* process,
                                  OutHandle<HandleAttr::Copy> out_handle);
};

} // namespace hydra::horizon::services::am
