#pragma once

#include "core/horizon/services/const.hpp"

namespace hydra::horizon::services::am {

class IProxy : public IService {
  protected:
    // Commands
    result_t GetCommonStateGetter(RequestContext* ctx);
    result_t GetSelfController(RequestContext* ctx);
    result_t GetWindowController(RequestContext* ctx);
    result_t GetAudioController(RequestContext* ctx);
    result_t GetDisplayController(RequestContext* ctx);
    result_t GetProcessWindingController(RequestContext* ctx);
    result_t GetLibraryAppletCreator(RequestContext* ctx);
    result_t GetAppletCommonFunctions(RequestContext* ctx); // 7.0.0+
    result_t GetHomeMenuFunctions(RequestContext* ctx);     // 15.0.0+
    result_t GetGlobalStateController(RequestContext* ctx); // 15.0.0+
    result_t GetDebugFunctions(RequestContext* ctx);
};

} // namespace hydra::horizon::services::am
