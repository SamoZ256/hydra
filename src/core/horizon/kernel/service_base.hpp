#pragma once

#include "core/horizon/kernel/hipc.hpp"

namespace hydra::horizon::kernel {

class ServiceBase;

typedef std::function<void(ServiceBase*)> add_service_fn_t;
typedef std::function<ServiceBase*(handle_id_t)> get_service_fn_t;

struct RequestContext {
    hipc::Readers& readers;
    hipc::Writers& writers;
    const add_service_fn_t& add_service;
    const get_service_fn_t& get_service;
};

class ServiceBase : public Handle {
  public:
    virtual void Request(RequestContext& context);
    virtual usize GetPointerBufferSize() { return 0; }

  protected:
    virtual result_t RequestImpl(RequestContext& context, u32 id) = 0;
};

} // namespace hydra::horizon::kernel
