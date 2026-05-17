#pragma once

#include "core/horizon/services/const.hpp"
#include "core/horizon/services/hid/const.hpp"

namespace hydra::horizon::services::irsensor {

struct IrCameraHandle {
    u8 player_number;
    u8 device_type;
    u16 _reserved_x2;
};

enum class IrSensorFunctionLevel : u8 {
    Unknown0 = 0, // 1.0.0+
    Unknown1 = 1, // 4.0.0+
    Unknown2 = 2, // 5.0.0+
    Unknown3 = 3, // 6.0.0+
    Unknown4 = 4, // 8.0.0+
};

struct PackedFunctionLevel {
    IrSensorFunctionLevel function_level;
    u8 _reserved_x1[0x3];
};

class IIrSensorServer : public IService {
  protected:
    result_t RequestImpl([[maybe_unused]] RequestContext& context,
                         u32 id) override;

  private:
    // Commands
    STUB_REQUEST_COMMAND(DeactivateIrsensor);
    // TODO: PID descriptor
    result_t
    GetIrsensorSharedMemoryHandle(System* system, kernel::Process* process,
                                  u64 aruid,
                                  OutHandle<HandleAttr::Copy> out_handle);
    result_t GetNpadIrCameraHandle(hid::NpadIdType npad_id,
                                   IrCameraHandle* out_handle);
    // TODO: PID descriptor
    result_t
    ActivateIrsensorWithFunctionLevel(PackedFunctionLevel function_level,
                                      u64 aruid); // 4.0.0+
};

} // namespace hydra::horizon::services::irsensor

ENABLE_ENUM_FORMATTING(
    hydra::horizon::services::irsensor::IrSensorFunctionLevel, Unknown0,
    "unknown0", Unknown1, "unknown1", Unknown2, "unknown2", Unknown3,
    "unknown3", Unknown4, "unknown4")

ENABLE_STRUCT_FORMATTING(
    hydra::horizon::services::irsensor::PackedFunctionLevel, function_level, "",
    "function level")
