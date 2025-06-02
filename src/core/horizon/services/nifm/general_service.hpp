#pragma once

#include "core/horizon/services/const.hpp"

namespace hydra::horizon::services::nifm {

enum class InternetConnectionType : u32 {
    WiFi = 1,
    Ethernet = 2,
};

enum class InternetConnectionStatus {
    ConnectingUnknown1 = 0,
    ConnectingUnknown2 = 1,
    ConnectingUnknown3 = 2,
    ConnectingUnknown4 = 3,
    Connected = 4,
};

struct IpV4Address {
    u8 addr[4];
};

class IGeneralService : public ServiceBase {
  protected:
    result_t RequestImpl(RequestContext& context, u32 id) override;

  private:
    // Commands
    result_t CreateRequest(add_service_fn_t add_service,
                           i32 requirement_preset);
    result_t
    GetCurrentNetworkProfile(OutBuffer<BufferAttr::HipcPointer> out_buffer);
    result_t GetCurrentIpAddress(IpV4Address* out_ip);
    result_t GetInternetConnectionStatus(InternetConnectionType* out_type,
                                         u32* out_wifi_strength,
                                         InternetConnectionStatus* out_status);
};

} // namespace hydra::horizon::services::nifm
