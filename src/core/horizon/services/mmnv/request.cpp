#include "core/horizon/services/mmnv/request.hpp"

namespace hydra::horizon::services::mmnv {

DEFINE_SERVICE_COMMAND_TABLE(IRequest, 0, initializeOld, 1, finalizeOld, 2,
                             setAndWaitOld, 3, getOld, 4, initialize, 5,
                             finalize, 6, setAndWait, 7, get)

result_t IRequest::initializeOld(Module module, u32 priority,
                                 u32 event_clear_mode) {
    u32 unused;
    return initialize(module, priority, event_clear_mode, &unused);
}

result_t IRequest::finalizeOld(Module module) {
    LOG_FUNC_WITH_ARGS_STUBBED(Services, "module: {}", module);
    return RESULT_SUCCESS;
}

result_t IRequest::setAndWaitOld(Module module, u32 setting_min,
                                 u32 setting_max) {
    LOG_FUNC_WITH_ARGS_STUBBED(Services,
                               "module: {}, setting min: {}, setting max: {}",
                               module, setting_min, setting_max);

    return RESULT_SUCCESS;
}

result_t IRequest::getOld(Module module, u32* out_setting_current) {
    LOG_FUNC_WITH_ARGS_STUBBED(Services, "module: {}", module);

    // HACK
    *out_setting_current = 0;
    return RESULT_SUCCESS;
}

result_t IRequest::initialize(Module module, u32 priority, u32 event_clear_mode,
                              u32* out_request_id) {
    LOG_FUNC_WITH_ARGS_STUBBED(Services,
                               "module: {}, prio: {}, event autoclear: {}",
                               module, priority, event_clear_mode != 0);

    // HACK
    *out_request_id = 1;
    return RESULT_SUCCESS;
}

result_t IRequest::finalize(u32 request_id) {
    LOG_FUNC_WITH_ARGS_STUBBED(Services, "request ID: {}", request_id);
    return RESULT_SUCCESS;
}

result_t IRequest::setAndWait(u32 request_id, u32 setting_min,
                              u32 setting_max) {
    LOG_FUNC_WITH_ARGS_STUBBED(
        Services, "request ID: {}, setting min: {}, setting max: {}",
        request_id, setting_min, setting_max);
    return RESULT_SUCCESS;
}

result_t IRequest::get(u32 request_id, u32* out_setting_current) {
    LOG_FUNC_WITH_ARGS_STUBBED(Services, "request ID: {}", request_id);

    // HACK
    *out_setting_current = 0;
    return RESULT_SUCCESS;
}

} // namespace hydra::horizon::services::mmnv
