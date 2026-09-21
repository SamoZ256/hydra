#pragma once

#include "core/horizon/services/const.hpp"

namespace hydra::horizon::services::am {

class IProxy : public IService {
  protected:
    // Commands
    result_t getCommonStateGetter(RequestContext* ctx);
    result_t getSelfController(RequestContext* ctx);
    result_t getWindowController(RequestContext* ctx);
    result_t getAudioController(RequestContext* ctx);
    result_t getDisplayController(RequestContext* ctx);
    result_t getProcessWindingController(RequestContext* ctx);
    result_t getLibraryAppletCreator(RequestContext* ctx);
    result_t getAppletCommonFunctions(RequestContext* ctx); // 7.0.0+
    result_t getHomeMenuFunctions(RequestContext* ctx);     // 15.0.0+
    result_t getGlobalStateController(RequestContext* ctx); // 15.0.0+
    result_t getDebugFunctions(RequestContext* ctx);
};

} // namespace hydra::horizon::services::am
