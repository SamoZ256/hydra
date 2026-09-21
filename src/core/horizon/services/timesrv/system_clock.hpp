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
    explicit ISystemClock(SystemClockType type_) : type{type_} {}

  protected:
    result_t requestImpl([[maybe_unused]] RequestContext& context,
                         u32 id) override;

  private:
    SystemClockType type;

    // Commands
    result_t getCurrentTime(RequestContext* ctx, i64* out_posix_time);
    STUB_REQUEST_COMMAND(getSystemClockContext);
};

} // namespace hydra::horizon::services::timesrv
