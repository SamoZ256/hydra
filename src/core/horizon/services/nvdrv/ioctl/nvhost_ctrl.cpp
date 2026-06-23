#include "core/horizon/services/nvdrv/ioctl/nvhost_ctrl.hpp"

#include "core/horizon/kernel/event.hpp"
#include "core/horizon/services/settings/nx_settings.hpp"

namespace hydra::horizon::services::nvdrv::ioctl {

DEFINE_IOCTL_TABLE(NvHostCtrl,
                   DEFINE_IOCTL_TABLE_ENTRY(NvHostCtrl, 0x00, 0x1b, GetConfig,
                                            0x1d, SyncptWaitEvent, 0x1e,
                                            SyncptWaitEventEx, 0x1f,
                                            SyncptAllocEvent, 0x20,
                                            SyncptFreeEvent))

NvResult NvHostCtrl::QueryEvent(u32 event_id_u32, kernel::Event*& out_event) {
    u32 slot;
    u32 syncpoint_id;
    if (extract_bits(event_id_u32, 28, 1) != 0u) { // New format
        slot = extract_bits(event_id_u32, 0, 16);
        syncpoint_id = extract_bits(event_id_u32, 16, 12);
    } else { // Old format
        slot = extract_bits(event_id_u32, 0, 8);
        syncpoint_id = extract_bits(event_id_u32, 4, 28);
    }

    if (slot >= EVENT_COUNT) {
        LOG_WARN(Services, "Invalid event at slot {}", slot);
        return NvResult::NotSupported; // TODO: NvInternalResult::InvalidInput
    }

    const auto& event = events[slot];
    // TODO: check if syncpoint IDs are equal
    (void)syncpoint_id;

    out_event = event.event;
    return NvResult::Success;
}

NvResult NvHostCtrl::GetConfig(std::array<char, 0x41> name,
                               std::array<char, 0x41> key,
                               std::array<u8, 0x101>* out_value) {
    const auto key_str =
        to_lower(fmt::format("{}!{}", name.data(), key.data()));
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
        std::memcpy(ptr, value.s.data(), value.s.size());
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

NvResult NvHostCtrl::SyncptWaitEvent(u32 id, u32 tresh, i32 timeout,
                                     u32* out_value) {
    LOG_FUNC_WITH_ARGS_STUBBED(Services, "ID: {}, treshold: {}, timeout: {}",
                               id, tresh, timeout);

    // HACK
    *out_value = 0;
    return NvResult::Success;
}

NvResult NvHostCtrl::SyncptWaitEventEx(u32 id, u32 tresh, i32 timeout,
                                       InOutSingle<u32> out_value) {
    LOG_FUNC_WITH_ARGS_STUBBED(Services, "ID: {}, treshold: {}, timeout: {}",
                               id, tresh, timeout);

    // HACK
    out_value = 0;
    return NvResult::Success;
}

NvResult NvHostCtrl::SyncptAllocEvent(u32 slot) {
    auto& event = events[slot];

    // Check if event is already allocated
    // TODO: correct?
    if (event.event != nullptr)
        event.event->Release();

    event.event = new kernel::Event(false, fmt::format("NvHostEvent {}", slot));

    return NvResult::Success;
}

NvResult NvHostCtrl::SyncptFreeEvent(u32 slot) {
    auto& event = events[slot];
    event.event->Release();
    event.event = nullptr;

    return NvResult::Success;
}

} // namespace hydra::horizon::services::nvdrv::ioctl
