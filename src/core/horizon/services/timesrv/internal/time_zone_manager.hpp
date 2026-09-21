#pragma once

#include <set>

#include "core/horizon/services/timesrv/const.hpp"

namespace hydra::horizon::filesystem {
class Filesystem;
}

namespace hydra::horizon::services::timesrv::internal {

class TimeZoneManager {
  public:
    explicit TimeZoneManager(filesystem::Filesystem& filesystem_);

    std::string_view getDeviceLocationName();
    void loadRule(std::string_view location_name, TimeZoneRule& out_rule) const;
    void loadMyRule();

  private:
    filesystem::Filesystem& filesystem;

    std::set<std::string, std::less<>> locations;
    TimeZoneRule my_rule;

  public:
    CONST_REF_GETTER(locations, getLocations);
    CONST_REF_GETTER(my_rule, getMyRule);
};

} // namespace hydra::horizon::services::timesrv::internal
