#include "core/horizon/services/settings/factory_settings_server.hpp"

namespace hydra::horizon::services::settings {

DEFINE_SERVICE_COMMAND_TABLE(IFactorySettingsServer, 14,
                             getEciDeviceCertificate)

result_t IFactorySettingsServer::getEciDeviceCertificate(
    OutBuffer<BufferAttr::MapAlias> out_buffer) {
    (void)out_buffer;
    LOG_FUNC_STUBBED(Services);
    return RESULT_SUCCESS;
}

} // namespace hydra::horizon::services::settings
