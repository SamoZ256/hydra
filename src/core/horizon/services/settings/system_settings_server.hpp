#pragma once

#include "core/horizon/services/const.hpp"

namespace hydra::horizon::services::settings {

enum class ColorSetId : i32 {
    BasicWhite,
    BasicBlack,
};

enum class TvFlags : u32 {
    None = 0,
    Allows4k = ZTD_BIT(0),
    Allows3d = ZTD_BIT(1),
    AllowsCec = ZTD_BIT(2),
    PreventsScreenBurnIn = ZTD_BIT(3),
};
ZTD_ENABLE_ENUM_BITWISE_OPERATORS(TvFlags)

enum class TvResolution : u32 {
    Auto = 0,
    _1080p = 1,
    _720p = 2,
    _480p = 3,
};

enum class HdmiContentType : u32 {
    None = 0,
    Graphics = 1,
    Cinema = 2,
    Photo = 3,
    Game = 4,
};

enum class RgbRange : u32 {
    Auto = 0,
    Full = 1,
    Limited = 2,
};

enum class CmuMode : u32 {
    None = 0,
    ColorInvert = 1,
    HighContrast = 2,
    GrayScale = 3,
};

struct TvSettings {
    TvFlags flags;
    TvResolution resolution;
    HdmiContentType hdmi_content_type;
    RgbRange rgb_range;
    CmuMode cmu_mode;
    u32 tv_underscan;
    u32 tv_gamma;
    u32 contrast_ratio;
};

class ISystemSettingsServer : public IService {
  protected:
    result_t RequestImpl([[maybe_unused]] RequestContext& context,
                         u32 id) override;

  private:
    // Commands
    result_t GetFirmwareVersion(OutBuffer<BufferAttr::HipcPointer> out_buffer);
    result_t GetFirmwareVersion2(OutBuffer<BufferAttr::HipcPointer> out_buffer);
    result_t GetColorSetId(ColorSetId* out_id);
    result_t GetSettingsItemValueSize(
        InBuffer<BufferAttr::HipcPointer> in_name_buffer,
        InBuffer<BufferAttr::HipcPointer> in_item_key_buffer, u64* out_size);
    result_t
    GetSettingsItemValue(InBuffer<BufferAttr::HipcPointer> in_name_buffer,
                         InBuffer<BufferAttr::HipcPointer> in_item_key_buffer,
                         u64* out_size,
                         OutBuffer<BufferAttr::MapAlias> out_buffer);
    result_t GetTvSettings(TvSettings* out_settings);
    result_t GetDebugModeFlag(bool* out_flag);
    result_t GetDeviceNickName(OutBuffer<BufferAttr::MapAlias> out_buffer);
};

} // namespace hydra::horizon::services::settings
