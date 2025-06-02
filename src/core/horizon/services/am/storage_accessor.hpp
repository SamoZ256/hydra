#pragma once

#include "core/horizon/services/const.hpp"

namespace hydra::horizon::services::am {

class IStorageAccessor : public ServiceBase {
  public:
    IStorageAccessor(const sized_ptr data_) : data{data_} {}

  protected:
    result_t RequestImpl(RequestContext& context, u32 id) override;

  private:
    const sized_ptr data;

    // Commands
    result_t GetSize(i64* out_size);
    result_t Write(i64 offset, InBuffer<BufferAttr::AutoSelect> buffer);
    result_t Read(i64 offset, OutBuffer<BufferAttr::AutoSelect> out_buffer);
};

} // namespace hydra::horizon::services::am
