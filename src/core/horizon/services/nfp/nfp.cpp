#include "core/horizon/services/nfp/nfp.hpp"

#include "core/horizon/kernel/process.hpp"

namespace hydra::horizon::services::nfp {

INfp::INfp(PermissionLevel perm_level_)
    : perm_level{perm_level_}, availability_change_event(new kernel::Event(
                                   false, "NFP availability change event")) {}

result_t INfp::initialize(u64 aruid, u64 zero,
                          InBuffer<BufferAttr::MapAlias> in_version_buffer) {
    (void)aruid;
    (void)zero;
    (void)in_version_buffer;
    LOG_FUNC_STUBBED(Services);
    return RESULT_SUCCESS;
}

result_t INfp::listDevices(i32* out_count,
                           OutBuffer<BufferAttr::HipcPointer> out_buffer) {
    (void)out_buffer;
    LOG_FUNC_STUBBED(Services);

    // HACK
    *out_count = 0;
    return RESULT_SUCCESS;
}

result_t INfp::getState(u32* out_state) {
    LOG_FUNC_STUBBED(Services);

    // HACK
    *out_state = 0;
    return RESULT_SUCCESS;
};

result_t
INfp::attachAvailabilityChangeEvent(kernel::Process* process,
                                    OutHandle<HandleAttr::Copy> out_handle) {
    LOG_FUNC_STUBBED(Services);

    out_handle = process->addHandle(availability_change_event);
    return RESULT_SUCCESS;
}

} // namespace hydra::horizon::services::nfp
