#pragma once

#include "core/horizon/kernel/hipc/const.hpp"

namespace hydra {
class System;
}

namespace hydra::horizon::kernel {
class Process;
}

namespace hydra::horizon::services {

using result_t = kernel::result_t;

class Server;
class IService;

struct RequestContext {
    System& system;
    kernel::Process* process;
    kernel::hipc::Streams& streams;
};

class IService {
  public:
    void HandleRequest(System& system, kernel::Process* caller_process,
                       uptr ptr);

    void AddService(RequestContext& context, IService* service);
    IService* GetService(RequestContext& context, handle_id_t handle_id);

    // Reference counting
    IService* Retain() {
        ref_count.fetch_add(1, std::memory_order_relaxed);
        return this;
    }
    void Release() {
        if (ref_count.fetch_sub(1, std::memory_order_acq_rel) == 1)
            delete this;
    }

  protected:
    virtual ~IService();

    virtual result_t RequestImpl(RequestContext& context, u32 id) = 0;

    u32 AddSubservice(IService* service) {
        if (!service)
            return INVALID_HANDLE_ID;

        return parent->subservice_pool->Add(service);
    }

    void FreeSubservice(handle_id_t handle_id) {
        parent->subservice_pool->Get(handle_id)->Release();
        parent->subservice_pool->Free(handle_id);
    }

    IService* GetSubservice(handle_id_t handle_id) const {
        return parent->subservice_pool->Get(handle_id);
    }

  private:
    Server* server{nullptr};

    std::atomic<i32> ref_count{1};

    // Domain
    bool is_domain{false};
    IService* parent{this};
    DynamicPool<IService*>* subservice_pool{nullptr};

    void Close();
    void Request(RequestContext& context);
    void CmifRequest(RequestContext& context);
    void Control(RequestContext& context);
    void Clone(RequestContext& context);
    void TipcRequest(RequestContext& context, const u32 command_id);

  public:
    SETTER(server, SetServer);
    GETTER(is_domain, IsDomain);
};

} // namespace hydra::horizon::services
