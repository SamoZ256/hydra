#pragma once

#include "core/horizon/services/const.hpp"

namespace hydra::horizon::services::am {

class IStorage : public IService {
  public:
    explicit IStorage(std::vector<u8> data_) : data{std::move(data_)} {}

    template <typename T>
    explicit IStorage(const T& data_) {
        data.resize(sizeof(T));
        std::memcpy(data.data(), reinterpret_cast<const void*>(&data_),
                    sizeof(T));
    }

    std::span<u8> getData() { return data; }

  protected:
    result_t requestImpl([[maybe_unused]] RequestContext& context,
                         u32 id) override;

  private:
    std::vector<u8> data;

    // Commands
    result_t open(RequestContext* ctx);
};

} // namespace hydra::horizon::services::am
