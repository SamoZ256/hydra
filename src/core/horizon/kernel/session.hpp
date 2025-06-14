#pragma once

#include "core/horizon/kernel/auto_object.hpp"
#include "core/horizon/kernel/service_base.hpp"

namespace hydra::horizon::kernel {

class Session : public AutoObject {
  public:
    Session(ServiceBase* service_) : service{service_} {}

    void Close();
    void Request(RequestContext& context);
    void Control(hipc::Readers& readers, hipc::Writers& writers);
    void TipcRequest(RequestContext& context, const u32 command_id);

    // Getters
    ServiceBase* GetService() const { return service; }

    // Setters
    void SetHandleId(handle_id_t handle_id_) { handle_id = handle_id_; }

  private:
    ServiceBase* service;

    handle_id_t handle_id{invalid<handle_id_t>()};
};

} // namespace hydra::horizon::kernel
