#pragma once

#include "core/horizon/services/const.hpp"

namespace hydra::horizon::services::am {

class IStorageAccessor : public IService {
  public:
    explicit IStorageAccessor(std::span<u8> data_) : data{data_} {}

  protected:
    result_t requestImpl([[maybe_unused]] RequestContext& context,
                         u32 id) override;

  private:
    std::span<u8> data;

    // Commands
    result_t getSize(i64* out_size);
    result_t write(i64 offset, InBuffer<BufferAttr::AutoSelect> buffer);
    result_t read(i64 offset, OutBuffer<BufferAttr::AutoSelect> out_buffer);
};

} // namespace hydra::horizon::services::am
