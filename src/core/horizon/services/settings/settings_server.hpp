#pragma once

#include "core/horizon/const.hpp"
#include "core/horizon/services/const.hpp"

namespace hydra::horizon::services::settings {

enum class RegionCode : u32 {
    Japan = 0,
    Usa = 1,
    Europe = 2,
    Australia = 3,
    HongKongTaiwanKorea = 4,
    China = 5,
};

class ISettingsServer : public IService {
  protected:
    result_t RequestImpl([[maybe_unused]] RequestContext& context,
                         u32 id) override;

  private:
    // Commands
    result_t GetLanguageCode(LanguageCode* out_language_code);
    result_t
    GetAvailableLanguageCodes(i32* out_count,
                              OutBuffer<BufferAttr::HipcPointer> out_buffer);
    result_t GetAvailableLanguageCodeCount(i32* out_count);
    result_t GetRegionCode(RegionCode* out_code);
    result_t
    GetAvailableLanguageCodes2(i32* out_count,
                               OutBuffer<BufferAttr::MapAlias> out_buffer);
    result_t
    GetDeviceNickName(OutBuffer<BufferAttr::MapAlias> out_buffer); // 10.1.0+
};

} // namespace hydra::horizon::services::settings
