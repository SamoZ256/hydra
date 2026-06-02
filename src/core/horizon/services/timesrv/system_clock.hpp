#pragma once

#include "core/horizon/services/const.hpp"

namespace hydra::horizon::services::timesrv {

enum class SystemClockType {
    StandardUser,
    StandardNetwork,
    StandardLocal,
    EphemeralNetwork,
};

class ISystemClock : public IService {
  public:
    ISystemClock(SystemClockType type_) : type{type_} {}

  protected:
    result_t RequestImpl([[maybe_unused]] RequestContext& context,
                         u32 id) override;

  private:
    SystemClockType type;

    // Commands
    result_t GetCurrentTime(RequestContext* ctx, i64* out_posix_time);
    STUB_REQUEST_COMMAND(GetSystemClockContext);
};

} // namespace hydra::horizon::services::timesrv
