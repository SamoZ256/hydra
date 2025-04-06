#include "horizon/services/psm/psm_server.hpp"

// TODO: report real data on devices with batteries + make this configurable

namespace Hydra::Horizon::Services::Psm {

namespace {

enum class ChargerType {
    Unconnected,
    EnoughPower,
    NoPower,
    NotSupported,
};

}

DEFINE_SERVICE_COMMAND_TABLE(IPsmServer, 0, GetBatteryChargePercentage, 1,
                             GetChargerType, 13, GetRawBatteryChargePercentage,
                             14, IsEnoughPowerSupplied, 15,
                             GetBatteryAgePercentage)

void IPsmServer::GetBatteryChargePercentage(REQUEST_COMMAND_PARAMS) {
    writers.writer.Write<u32>(100);
}

void IPsmServer::GetChargerType(REQUEST_COMMAND_PARAMS) {
    writers.writer.Write(ChargerType::EnoughPower);
}

void IPsmServer::GetRawBatteryChargePercentage(REQUEST_COMMAND_PARAMS) {
    writers.writer.Write(100.0);
}

void IPsmServer::IsEnoughPowerSupplied(REQUEST_COMMAND_PARAMS) {
    writers.writer.Write(true);
}

void IPsmServer::GetBatteryAgePercentage(REQUEST_COMMAND_PARAMS) {
    writers.writer.Write(0.0);
}

} // namespace Hydra::Horizon::Services::Psm
