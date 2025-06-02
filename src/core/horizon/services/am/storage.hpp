#pragma once

#include "core/horizon/services/const.hpp"

namespace hydra::horizon::services::am {

class IStorage : public ServiceBase {
  public:
    IStorage(const sized_ptr data_) : data{data_} {}
    ~IStorage() override {
        // TODO: uncomment
        // free(data.GetPtrU8());
    }

    const sized_ptr GetData() const { return data; }

  protected:
    result_t RequestImpl(RequestContext& context, u32 id) override;

  private:
    const sized_ptr data;

    // Commands
    result_t Open(add_service_fn_t add_service);
};

} // namespace hydra::horizon::services::am
