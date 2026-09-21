#include "core/horizon/services/i2c/manager.hpp"

#include "core/horizon/services/i2c/session.hpp"

namespace hydra::horizon::services::i2c {

DEFINE_SERVICE_COMMAND_TABLE(IManager, 1, openSession, 2, hasDevice)

result_t IManager::openSession(RequestContext* ctx, I2cDevice device) {
    addService(*ctx, new ISession(device));
    return RESULT_SUCCESS;
}

result_t IManager::hasDevice(I2cDevice device, bool* out_has) {
    LOG_FUNC_WITH_ARGS_STUBBED(Services, "device: {}", device);

    // HACK
    *out_has = true;
    return RESULT_SUCCESS;
}

} // namespace hydra::horizon::services::i2c
