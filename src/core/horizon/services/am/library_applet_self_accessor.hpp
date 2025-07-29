#pragma once

#include "core/horizon/const.hpp"
#include "core/horizon/services/const.hpp"

namespace hydra::horizon::services::am {

struct LibraryAppletInfo {
    AppletId id;
    LibraryAppletMode mode;
};

struct AppletIdentityInfo {
    AppletId id;
    u32 _padding_x4;
    u64 application_id;
};

class ILibraryAppletSelfAccessor : public IService {
  public:
    ILibraryAppletSelfAccessor();

  protected:
    result_t RequestImpl(RequestContext& context, u32 id) override;

  private:
    // Commands
    result_t PopInData(RequestContext* ctx);
    result_t PushOutData(IService* storage_);
    result_t PopInteractiveInData(RequestContext* ctx);
    result_t PushInteractiveOutData(IService* storage_);
    result_t ExitProcessAndReturn(kernel::Process* process);
    result_t GetLibraryAppletInfo(LibraryAppletInfo* out_info);
    result_t GetCallerAppletIdentityInfo(AppletIdentityInfo* out_info);
};

} // namespace hydra::horizon::services::am
