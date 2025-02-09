#pragma once

#include "horizon/services/service.hpp"

namespace Hydra::Horizon::Services {

class FileSystemProxy : public ServiceBase {
  public:
    FileSystemProxy(Handle handle_) : ServiceBase(handle_) {}

    void Request(Kernel& kernel, Writer& writer, u8* in_ptr) override;

  private:
};

} // namespace Hydra::Horizon::Services
