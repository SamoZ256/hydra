#pragma once

#include "core/horizon/kernel/kernel.hpp"
#include "core/horizon/services/const.hpp"

namespace hydra::horizon::services::am {

class IHomeMenuFunctions : public ServiceBase {
  public:
    IHomeMenuFunctions()
        : pop_from_general_channel_event{new kernel::Event(
              kernel::EventFlags::None, "Pop from general channel event")} {}

  protected:
    result_t RequestImpl(RequestContext& context, u32 id) override;

  private:
    kernel::Event* pop_from_general_channel_event;

    // Commands
    STUB_REQUEST_COMMAND(RequestToGetForeground);
    result_t
    GetPopFromGeneralChannelEvent(kernel::Process* process,
                                  OutHandle<HandleAttr::Copy> out_handle);
};

} // namespace hydra::horizon::services::am
