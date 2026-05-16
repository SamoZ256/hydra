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
  protected:
    result_t RequestImpl([[maybe_unused]] RequestContext& context,
                         u32 id) override;

  private:
    // Commands
    result_t PopInData(RequestContext* ctx, System* system);
    result_t PushOutData(System* system, IService* storage_);
    result_t PopInteractiveInData(RequestContext* ctx, System* system);
    result_t PushInteractiveOutData(System* system, IService* storage_);
    result_t ExitProcessAndReturn(kernel::Process* process);
    result_t GetLibraryAppletInfo(LibraryAppletInfo* out_info);
    result_t GetCallerAppletIdentityInfo(AppletIdentityInfo* out_info);
};

} // namespace hydra::horizon::services::am
