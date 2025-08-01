#pragma once

#include "core/horizon/kernel/hipc/const.hpp"

namespace hydra::horizon::kernel {
class Process;
}

namespace hydra::horizon::services {

using result_t = kernel::result_t;

class Server;
class IService;

struct RequestContext {
    kernel::Process* process;
    kernel::hipc::Readers& readers;
    kernel::hipc::Writers& writers;
};

class IService {
  public:
    virtual ~IService();

    void HandleRequest(kernel::Process* caller_process, uptr ptr);

    void AddService(RequestContext& context, IService* service);
    IService* GetService(RequestContext& context, handle_id_t handle_id);

  protected:
    virtual result_t RequestImpl(RequestContext& context, u32 id) = 0;
    virtual usize GetPointerBufferSize() { return 0; }

    u32 AddSubservice(IService* service) {
        if (!service)
            return INVALID_HANDLE_ID;

        return parent->subservice_pool->Add(service);
    }

    void FreeSubservice(handle_id_t handle_id) {
        // TODO: should it be deleted?
        // delete parent->subservice_pool->Get(handle_id);
        parent->subservice_pool->Free(handle_id);
    }

    IService* GetSubservice(handle_id_t handle_id) const {
        return parent->subservice_pool->Get(handle_id);
    }

  private:
    Server* server{nullptr};

    // Domain
    bool is_domain{false};
    IService* parent{this};
    DynamicPool<IService*>* subservice_pool{nullptr};

    void Close();
    void Request(RequestContext& context);
    void CmifRequest(RequestContext& context);
    void Control(kernel::Process* caller_process,
                 kernel::hipc::Readers& readers,
                 kernel::hipc::Writers& writers);
    void Clone(kernel::Process* caller_process, kernel::hipc::Writers& writers);
    void TipcRequest(RequestContext& context, const u32 command_id);

  public:
    SETTER(server, SetServer);
    GETTER(is_domain, IsDomain);
};

} // namespace hydra::horizon::services
