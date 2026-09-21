#pragma once

#include "core/horizon/services/const.hpp"
#include "core/horizon/services/hid/const.hpp"

namespace hydra::horizon::services::hid {

class IHidSystemServer : public IService {
  public:
    IHidSystemServer();

  protected:
    result_t requestImpl([[maybe_unused]] RequestContext& context,
                         u32 id) override;

  private:
    kernel::Event* home_button_event;
    kernel::Event* sleep_button_event;

    // Commands
    result_t
    acquireHomeButtonEventHandle(kernel::Process* process,
                                 OutHandle<HandleAttr::Copy> out_handle);
    result_t
    acquireSleepButtonEventHandle(kernel::Process* process,
                                  OutHandle<HandleAttr::Copy> out_handle);
    STUB_REQUEST_COMMAND(applyNpadSystemCommonPolicy);
    result_t
    getUniquePadsFromNpad(NpadIdType npad_id, i64* out_count,
                          OutBuffer<BufferAttr::HipcPointer> out_buffer);
    result_t enableAppletToGetInput(u8 enable, u64 aruid);
};

} // namespace hydra::horizon::services::hid
