#pragma once

#include <set>

#include "core/horizon/services/timesrv/const.hpp"

namespace hydra::horizon::filesystem {
class Filesystem;
}

namespace hydra::horizon::services::timesrv::internal {

class TimeZoneManager {
  public:
    TimeZoneManager(filesystem::Filesystem& filesystem_);

    std::string_view GetDeviceLocationName();
    void LoadTimeZoneRule(std::string_view location_name,
                          TimeZoneRule& out_rule);

  private:
    filesystem::Filesystem& filesystem;

    std::set<std::string, std::less<>> locations;
    TimeZoneRule my_time_zone_rule;

  public:
    CONST_REF_GETTER(locations, GetLocations);
    CONST_REF_GETTER(my_time_zone_rule, GetMyTimeZoneRule);
};

} // namespace hydra::horizon::services::timesrv::internal
