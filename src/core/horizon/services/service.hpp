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

using kernel::result_t;

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

    void handleRequest(System& system, kernel::Process* caller_process,
                       uptr ptr);

    void addService(RequestContext& context, IService* service);
    IService* getService(RequestContext& context, Handle handle);

    // Reference counting
    IService* retain() {
        ref_count.fetch_add(1, std::memory_order_relaxed);
        return this;
    }
    void release() {
        if (ref_count.fetch_sub(1, std::memory_order_acq_rel) == 1)
            delete this;
    }

  protected:
    virtual result_t requestImpl(RequestContext& context, u32 id) = 0;

    Handle addSubservice(IService* service) {
        if (service == nullptr)
            return INVALID_HANDLE;

        return parent->subservice_pool->insert(service).value();
    }

    void freeSubservice(Handle handle) {
        parent->subservice_pool->get(handle).value()->release();
        ASSERT_DEBUG(parent->subservice_pool->free(handle), Services,
                     "Failed to free subservice");
    }

    IService* getSubservice(Handle handle) const {
        return parent->subservice_pool->get(handle).value();
    }

  private:
    Server* server{nullptr};

    std::atomic<i32> ref_count{1};

    // Domain
    bool is_domain{false};
    IService* parent{this};
    // TODO: dynamic pool?
    std::optional<StaticHandlePool<IService*, 512>> subservice_pool;

    void close();
    void request(RequestContext& context);
    void cmifRequest(RequestContext& context);
    void control(RequestContext& context);
    void clone(RequestContext& context);
    void tipcRequest(RequestContext& context, const u32 command_id);

  public:
    SETTER(server, setServer);
    GETTER(is_domain, isDomain);
};

} // namespace hydra::horizon::services
