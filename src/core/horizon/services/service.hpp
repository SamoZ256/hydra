#pragma once

#include "core/horizon/handle_pool.hpp"
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
    IService() noexcept = default;
    virtual ~IService() noexcept = default;

    ZTD_MAKE_NON_COPYABLE(IService);

    void HandleRequest(System& system, kernel::Process* caller_process,
                       uptr ptr);

    void AddService(RequestContext& context, IService* service);
    IService* GetService(RequestContext& context, Handle handle);

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
    virtual result_t RequestImpl(RequestContext& context, u32 id) = 0;

    Handle AddSubservice(IService* service) {
        if (service == nullptr)
            return INVALID_HANDLE;

        return parent->subservice_pool->Insert(service).value();
    }

    void FreeSubservice(Handle handle) {
        parent->subservice_pool->Get(handle).value()->Release();
        ASSERT_DEBUG(parent->subservice_pool->Free(handle), Services,
                     "Failed to free subservice");
    }

    IService* GetSubservice(Handle handle) const {
        return parent->subservice_pool->Get(handle).value();
    }

  private:
    Server* server{nullptr};

    std::atomic<i32> ref_count{1};

    // Domain
    bool is_domain{false};
    IService* parent{this};
    // TODO: dynamic pool?
    std::optional<StaticHandlePool<IService*, 512>> subservice_pool;

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
