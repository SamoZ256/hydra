#pragma once

#include "core/horizon/services/const.hpp"
#include "core/horizon/services/usb/const.hpp"

namespace hydra::horizon::services::psm {

enum class ChargerType {
    Unconnected,
    EnoughPower,
    NoPower,
    NotSupported,
};

enum class ChargerMode : u32 {
    ChargeDisable = 0,
    ChargeBattery = 1,
    Org = 2,
};

enum class HiZMode : u8 {
    Disable = 0,
    Enable = 1,
};

enum class BatteryCharging : u8 {
    Disable = 0,
    Enable = 1,
};

enum class Vdd50State : u32 {
    Unknown = 0,
    Vdd50AOffVdd50BOff = 1,
    Vdd50AOnVdd50BOff = 2,
    Vdd50AOffVdd50BOn = 3,
};

enum class FastBatteryCharging : u8 {
    Disable = 0,
    Enable = 1,
};

enum class ControllerPowerSupply : u8 {
    Disable = 0,
    Enable = 1,
};

enum class OtgRequest : u8 {
    Disable = 0,
    Enable = 1,
};

struct BatteryChargeInfoFields {
    u32 input_current_limit;
    u32 boost_mode_current_limit;
    u32 fast_charge_current_limit;
    u32 charge_voltage_limit;
    ChargerMode charge_mode;
    HiZMode hi_z_mode;
    BatteryCharging battery_charging;
    u8 _reserved_x16[2];
    Vdd50State vdd50_state;
    u32 temperature_milli_celsius;
    u32 battery_charge_percentage; // 100,000 = 100%
    u32 battery_charge_milli_voltage;
    u32 battery_age_percentage; // 100,000 = 100%
    usb::UsbPowerRole usb_power_role;
    usb::UsbChargerType usb_charger_type;
    u32 charger_input_voltage_limit_milli_voltage;
    u32 charger_input_current_limit_milli_amps;
    FastBatteryCharging fast_battery_charging;
    ControllerPowerSupply controller_power_supply;
    OtgRequest otg_request;
    u8 _reserved_x3f;
    u8 _unknown_x40[0x14]; // 17.0.0+
};

class IPsmServer : public IService {
  protected:
    result_t requestImpl([[maybe_unused]] RequestContext& context,
                         u32 id) override;

  private:
    // Commands
    result_t getBatteryChargePercentage(u32* out_percentage);
    result_t getChargerType(ChargerType* out_type);
    result_t openSession(RequestContext* ctx);
    result_t getRawBatteryChargePercentage(f64* out_percentage);
    result_t isEnoughPowerSupplied(bool* out_is_enough);
    result_t getBatteryAgePercentage(f64* out_percentage);
    result_t getBatteryChargeInfoFields(BatteryChargeInfoFields* out_fields);
};

} // namespace hydra::horizon::services::psm
