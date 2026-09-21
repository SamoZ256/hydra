#include "core/horizon/services/nvdrv/ioctl/nvdisp_ctrl.hpp"

namespace hydra::horizon::services::nvdrv::ioctl {

DEFINE_IOCTL_TABLE(NvDispCtrl,
                   DEFINE_IOCTL_TABLE_ENTRY(NvDispCtrl, 0x02, 0x12, numOutputs,
                                            0x13, getDisplayProperties))

NvResult NvDispCtrl::numOutputs(u32* out_num_outputs) {
    LOG_FUNC_STUBBED(Services);

    // HACK
    *out_num_outputs = 1;
    return NvResult::Success;
}

NvResult NvDispCtrl::getDisplayProperties(
    InOutSingle<std::array<u8, 20>> inout_unknown_x0) {
    (void)inout_unknown_x0;
    LOG_FUNC_STUBBED(Services);
    return NvResult::Success;
}

} // namespace hydra::horizon::services::nvdrv::ioctl
