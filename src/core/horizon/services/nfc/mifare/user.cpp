#include "core/horizon/services/nfc/mifare/user.hpp"

namespace hydra::horizon::services::nfc::mifare {

DEFINE_SERVICE_COMMAND_TABLE(IUser, 0, Initialize)

result_t IUser::Initialize(u64 aruid, u64 zero,
                           InBuffer<BufferAttr::MapAlias> in_version_buffer) {
    LOG_FUNC_STUBBED(Services);
    return RESULT_SUCCESS;
}

} // namespace hydra::horizon::services::nfc::mifare
