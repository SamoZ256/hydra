#pragma once

#include "core/horizon/services/const.hpp"

namespace hydra::horizon::services::spl {

enum class ConfigItem : u32 {
    DisableProgramVerification = 1,
    DramId,
    SecurityEngineInterruptNumber,
    FuseVersion,
    HardwareType,
    HardwareState,
    IsRecoveryBoot,
    DeviceId,
    BootReason,
    MemoryMode,
    IsDevelopmentFunctionEnabled,
    KernelConfiguration,
    IsChargerHiZModeEnabled,
    RetailInteractiveDisplayState,
    RegulatorType,
    DeviceUniqueKeyGeneration,
    Package2Hash,
};

class IGeneralInterface : public ServiceBase {
  protected:
    result_t RequestImpl(RequestContext& context, u32 id) override;

  private:
    // Commands
    result_t GetConfig(ConfigItem item, u64* out_config);
};

} // namespace hydra::horizon::services::spl

ENABLE_ENUM_FORMATTING(
    hydra::horizon::services::spl::ConfigItem, DisableProgramVerification,
    "disable program verification", DramId, "DRAM ID",
    SecurityEngineInterruptNumber, "security engine interrupt number",
    FuseVersion, "fuse version", HardwareType, "hardware type", HardwareState,
    "hardware state", IsRecoveryBoot, "is recovery boot", DeviceId, "device ID",
    BootReason, "boot reason", MemoryMode, "memory mode",
    IsDevelopmentFunctionEnabled, "is development function enabled",
    KernelConfiguration, "kernel configuration", IsChargerHiZModeEnabled,
    "is charger HiZ mode enabled", RetailInteractiveDisplayState,
    "retail interactive display state", RegulatorType, "regulator type",
    DeviceUniqueKeyGeneration, "device unique key generation", Package2Hash,
    "package 2 hash")
