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
    result_t RequestImpl([[maybe_unused]] RequestContext& context,
                         u32 id) override;

  private:
    kernel::Event* interface_available_event;
    kernel::Event* interface_state_change_event;

    // Commands
    result_t BindClientProcess(); // 2.0.0+
    result_t CreateInterfaceAvailableEvent(
        kernel::Process* process, Aligned<u8, 2> index,
        DeviceFilter device_filter, OutHandle<HandleAttr::Copy> out_handle);
    result_t
    GetInterfaceStateChangeEvent(kernel::Process* process,
                                 OutHandle<HandleAttr::Copy> out_handle);
};

} // namespace hydra::horizon::services::usb::hs

ENABLE_STRUCT_FORMATTING(hydra::horizon::services::usb::hs::DeviceFilter, flags,
                         ":#x", "flags", vendor_id, "", "vendor ID", product_id,
                         "", "product ID", bcd_device_min, "", "BCD device min",
                         bcd_device_max, "", "BCD device max", b_device_class,
                         "", "B device class", b_device_subclass, "",
                         "B device subclass", b_device_protocol, "",
                         "B device protocol", b_interface_class, "",
                         "B interface class", b_interface_subclass, "",
                         "B interface subclass", b_interface_protocol, "",
                         "B interface protocol")
