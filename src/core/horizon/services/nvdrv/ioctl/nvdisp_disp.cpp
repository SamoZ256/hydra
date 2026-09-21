#include "core/horizon/services/nvdrv/ioctl/nvdisp_disp.hpp"

namespace hydra::horizon::services::nvdrv::ioctl {

DEFINE_IOCTL_TABLE(NvDispDisp,
                   DEFINE_IOCTL_TABLE_ENTRY(NvDispDisp, 0x02, 0x01, getWindow,
                                            0x0e, setCmu))

NvResult NvDispDisp::getWindow(u32 unknown_x0) {
    (void)display_index;
    (void)unknown_x0;
    LOG_FUNC_STUBBED(Services);
    return NvResult::Success;
}

NvResult
NvDispDisp::setCmu(InOutSingle<std::array<u8, 2458>> inout_unknown_x0) {
    (void)inout_unknown_x0;
    LOG_FUNC_STUBBED(Services);
    return NvResult::Success;
}

} // namespace hydra::horizon::services::nvdrv::ioctl
