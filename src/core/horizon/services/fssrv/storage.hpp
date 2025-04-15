#pragma once

#include "core/horizon/services/service_base.hpp"

namespace Hydra::Horizon::Services::Fssrv {

// TODO: does IStorage always need to be backed by a file?
class IStorage : public ServiceBase {
  public:
    DEFINE_SERVICE_VIRTUAL_FUNCTIONS(IStorage)

    IStorage(Filesystem::File* file_);
    ~IStorage() override;

  protected:
    void RequestImpl(REQUEST_IMPL_PARAMS) override;

    // Helpers
    void ReadImpl(u8* ptr, u64 offset, usize& size);

  private:
    Filesystem::File* file;

    // Commands
    virtual void Read(REQUEST_COMMAND_PARAMS);
    void GetSize(REQUEST_COMMAND_PARAMS);
};

} // namespace Hydra::Horizon::Services::Fssrv
