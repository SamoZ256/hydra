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
    result_t requestImpl([[maybe_unused]] RequestContext& context,
                         u32 id) override;

  private:
    // Commands
    result_t popInData(RequestContext* ctx, System* system);
    result_t pushOutData(System* system, IService* storage_);
    result_t popInteractiveInData(RequestContext* ctx, System* system);
    result_t pushInteractiveOutData(System* system, IService* storage_);
    result_t exitProcessAndReturn(kernel::Process* process);
    result_t getLibraryAppletInfo(LibraryAppletInfo* out_info);
    result_t getCallerAppletIdentityInfo(AppletIdentityInfo* out_info);
    result_t getLibraryAppletInfoEx(u64* out_info); // 17.0.0+
};

} // namespace hydra::horizon::services::am
