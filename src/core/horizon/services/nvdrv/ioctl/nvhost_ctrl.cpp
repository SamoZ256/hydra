#include "core/horizon/services/nvdrv/ioctl/nvhost_ctrl.hpp"

#include "core/horizon/services/settings/nx_settings.hpp"

namespace hydra::horizon::services::nvdrv::ioctl {

DEFINE_IOCTL_TABLE(NvHostCtrl,
                   DEFINE_IOCTL_TABLE_ENTRY(NvHostCtrl, 0x00, 0x1b, GetConfig,
                                            0x1d, SyncptWaitEvent))

NvResult NvHostCtrl::GetConfig(std::array<char, 0x41> name,
                               std::array<char, 0x41> key,
                               std::array<u8, 0x101>* out_value) {
    // TODO: use a different connecting char?
    const auto key_str =
        to_lower(fmt::format("{}:{}", name.data(), key.data()));
    LOG_DEBUG(Services, "Key: {}", key_str);

    auto it = settings::nx_settings.find(key_str);
    if (it == settings::nx_settings.end()) {
        LOG_WARN(Services, "Key not found: {}", key_str);
        // TODO: return NvInternalResult::InvalidInput?
        return NvResult::NotAvailableInProduction;
    }

    const auto& value = it->second;
    auto ptr = out_value->data();
    switch (value.type) {
    case settings::SettingDataType::String:
        std::strcpy(reinterpret_cast<char*>(ptr), value.s);
        break;
    case settings::SettingDataType::Integer:
        *reinterpret_cast<i32*>(ptr) = value.i;
        break;
    case settings::SettingDataType::Boolean:
        *reinterpret_cast<bool*>(ptr) = value.b;
        break;
    }

    return NvResult::Success;
}

NvResult NvHostCtrl::SyncptWaitEvent(u32 id, u32 tresh, i32* out_timeout,
                                     u32* out_value) {
    LOG_FUNC_STUBBED(Services);

    // HACK
    *out_value = 0;
    return NvResult::Success;
}

} // namespace hydra::horizon::services::nvdrv::ioctl
