#pragma once

#include "core/horizon/services/const.hpp"

namespace hydra::horizon::services::am {

class IStorage : public IService {
  public:
    IStorage(std::span<u8> data_) : data{data_} {}

    template <typename T>
    IStorage(T* ptr) : data(reinterpret_cast<u8*>(ptr), sizeof(T)) {}

    ~IStorage() override {
        // TODO: uncomment
        // free(data.data());
    }

    std::span<u8> GetData() const { return data; }

  protected:
    result_t RequestImpl([[maybe_unused]] RequestContext& context,
                         u32 id) override;

  private:
    std::span<u8> data;

    // Commands
    result_t Open(RequestContext* ctx);
};

} // namespace hydra::horizon::services::am
