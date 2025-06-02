#pragma once

#include "core/horizon/const.hpp"
#include "core/horizon/services/const.hpp"

namespace hydra::horizon::services::settings {

class ISettingsServer : public ServiceBase {
  public:
    usize GetPointerBufferSize() override { return 0x1000; }

  protected:
    result_t RequestImpl(RequestContext& context, u32 id) override;

  private:
    // Commands
    result_t GetLanguageCode(LanguageCode* out_language_code);
    result_t
    GetAvailableLanguageCodes(i32* out_count,
                              OutBuffer<BufferAttr::HipcPointer> out_buffer);
    result_t GetAvailableLanguageCodeCount(i32* out_count);
    result_t
    GetAvailableLanguageCodes2(i32* out_count,
                               OutBuffer<BufferAttr::MapAlias> out_buffer);
};

} // namespace hydra::horizon::services::settings
