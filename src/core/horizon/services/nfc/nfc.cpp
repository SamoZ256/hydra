#include "core/horizon/services/nfc/nfc.hpp"

namespace hydra::horizon::services::nfc {

result_t INfc::initialize(u64 aruid, u64 zero,
                          InBuffer<BufferAttr::MapAlias> in_version_buffer) {
    (void)aruid;
    (void)zero;
    (void)in_version_buffer;
    LOG_FUNC_STUBBED(Services);
    return RESULT_SUCCESS;
}

result_t INfc::getState(u32* out_state) {
    LOG_FUNC_STUBBED(Services);

    // HACK
    *out_state = 0;
    return RESULT_SUCCESS;
};

} // namespace hydra::horizon::services::nfc
