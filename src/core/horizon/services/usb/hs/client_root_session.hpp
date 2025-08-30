#pragma once

#include "core/horizon/services/const.hpp"

namespace hydra::horizon::kernel {
class Event;
}

namespace hydra::horizon::services::usb::hs {

struct DeviceFilter {
    u16 flags;
    u16 vendor_id;
    u16 product_id;
    u16 bcd_device_min;
    u16 bcd_device_max;
    u8 b_device_class;
    u8 b_device_subclass;
    u8 b_device_protocol;
    u8 b_interface_class;
    u8 b_interface_subclass;
    u8 b_interface_protocol;
};

// NOTE: The commands are completely different on 1.0.0
class IClientRootSession : public IService {
  public:
    IClientRootSession();

  protected:
    result_t RequestImpl(RequestContext& context, u32 id) override;

  private:
    kernel::Event* interface_available_event;
    kernel::Event* interface_state_change_event;

    // Commands
    result_t BindClientProcess(); // 2.0.0+
    result_t CreateInterfaceAvailableEvent(
        kernel::Process* process, aligned<u8, 2> index,
        DeviceFilter device_filter, OutHandle<HandleAttr::Copy> out_handle);
    result_t
    GetInterfaceStateChangeEvent(kernel::Process* process,
                                 OutHandle<HandleAttr::Copy> out_handle);
};

} // namespace hydra::horizon::services::usb::hs
