#pragma once

#include "core/horizon/hid.hpp"
#include "core/horizon/services/const.hpp"

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
  public:
    IIrSensorServer();

  protected:
    result_t RequestImpl(RequestContext& context, u32 id) override;

  private:
    kernel::SharedMemory* shared_mem; // TODO: make this global

    // Commands
    STUB_REQUEST_COMMAND(DeactivateIrsensor);
    // TODO: PID descriptor
    result_t
    GetIrsensorSharedMemoryHandle(kernel::Process* process, u64 aruid,
                                  OutHandle<HandleAttr::Copy> out_handle);
    result_t GetNpadIrCameraHandle(::hydra::horizon::hid::NpadIdType npad_id,
                                   IrCameraHandle* out_handle);
    // TODO: PID descriptor
    result_t
    ActivateIrsensorWithFunctionLevel(PackedFunctionLevel function_level,
                                      u64 aruid); // 4.0.0+
};

} // namespace hydra::horizon::services::irsensor
