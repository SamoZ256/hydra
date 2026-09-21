#pragma once

#include "core/horizon/services/account/const.hpp"

namespace hydra::horizon::services::account {

class IProfile : public IService {
  public:
    explicit IProfile(uuid_t user_id_) : user_id{user_id_} {}

  protected:
    result_t RequestImpl([[maybe_unused]] RequestContext& context,
                         u32 id) override;

  private:
    uuid_t user_id;

    // Commands
    result_t Get(System* system, ProfileBase* out_base,
                 OutBuffer<BufferAttr::HipcPointer> out_user_data_buffer) const;
    result_t GetBase(System* system, ProfileBase* out_base) const;
    result_t GetImageSize(System* system, u32* out_size) const;
    result_t LoadImage(System* system,
                       OutBuffer<BufferAttr::MapAlias> out_buffer,
                       u32* out_size) const;
};

} // namespace hydra::horizon::services::account
