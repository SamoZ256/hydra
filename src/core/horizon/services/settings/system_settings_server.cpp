#include "core/horizon/services/settings/system_settings_server.hpp"

#include "core/horizon/const.hpp"
#include "core/horizon/services/settings/const.hpp"
#include "core/horizon/services/settings/nx_settings.hpp"

namespace hydra::horizon::services::settings {

DEFINE_SERVICE_COMMAND_TABLE(ISystemSettingsServer, 3, GetFirmwareVersion, 4,
                             GetFirmwareVersion2, 23, GetColorSetId, 37,
                             GetSettingsItemValueSize, 38, GetSettingsItemValue,
                             39, GetTvSettings, 62, GetDebugModeFlag, 77,
                             GetDeviceNickName)

result_t ISystemSettingsServer::GetFirmwareVersion(
    OutBuffer<BufferAttr::HipcPointer> out_buffer) {
    auto firmware_version = FIRMWARE_VERSION;

    // Zero out the revision numbers
    firmware_version.revision_major = 0;
    firmware_version.revision_minor = 0;

    out_buffer.stream->Write(firmware_version);
    return RESULT_SUCCESS;
}

result_t ISystemSettingsServer::GetFirmwareVersion2(
    OutBuffer<BufferAttr::HipcPointer> out_buffer) {
    out_buffer.stream->Write(FIRMWARE_VERSION);
    return RESULT_SUCCESS;
}

result_t ISystemSettingsServer::GetColorSetId(ColorSetId* out_id) {
    // TODO: make this configurable
    *out_id = ColorSetId::BasicWhite;
    return RESULT_SUCCESS;
}

result_t ISystemSettingsServer::GetSettingsItemValueSize(
    InBuffer<BufferAttr::HipcPointer> in_name_buffer,
    InBuffer<BufferAttr::HipcPointer> in_item_key_buffer, u64* out_size) {
    auto name = in_name_buffer.stream->ReadNullTerminatedString();
    auto item_key = in_item_key_buffer.stream->ReadNullTerminatedString();
    const auto* value = GetSettingsValue(name, item_key);
    if (!value) {
        // TODO: error
        return RESULT_SUCCESS;
    }

    switch (value->type) {
    case settings::SettingDataType::String:
        *out_size = value->s.size();
        break;
    case settings::SettingDataType::Integer:
        *out_size = sizeof(value->i);
        break;
    case settings::SettingDataType::Boolean:
        *out_size = sizeof(value->b);
        break;
    }

    return RESULT_SUCCESS;
}

result_t ISystemSettingsServer::GetSettingsItemValue(
    InBuffer<BufferAttr::HipcPointer> in_name_buffer,
    InBuffer<BufferAttr::HipcPointer> in_item_key_buffer, u64* out_size,
    OutBuffer<BufferAttr::MapAlias> out_buffer) {
    auto name = in_name_buffer.stream->ReadNullTerminatedString();
    auto item_key = in_item_key_buffer.stream->ReadNullTerminatedString();
    const auto* value = GetSettingsValue(name, item_key);
    if (!value) {
        // TODO: error
        return RESULT_SUCCESS;
    }

    switch (value->type) {
    case settings::SettingDataType::String:
        out_buffer.stream->WriteSpan(std::span(value->s));
        *out_size = value->s.size();
        break;
    case settings::SettingDataType::Integer:
        out_buffer.stream->Write(value->i);
        *out_size = sizeof(value->i);
        break;
    case settings::SettingDataType::Boolean:
        out_buffer.stream->Write(value->b);
        *out_size = sizeof(value->b);
        break;
    }

    return RESULT_SUCCESS;
}

result_t ISystemSettingsServer::GetTvSettings(TvSettings* out_settings) {
    LOG_FUNC_STUBBED(Services);

    // HACK
    *out_settings = {
        .flags = TvFlags::None,
        .resolution = TvResolution::Auto,
        .hdmi_content_type = HdmiContentType::Graphics,
        .rgb_range = RgbRange::Auto,
        .cmu_mode = CmuMode::None,
        .tv_underscan = 0,
        .tv_gamma = 0,
        .contrast_ratio = 0,
    };
    return RESULT_SUCCESS;
}

result_t ISystemSettingsServer::GetDebugModeFlag(bool* out_flag) {
    auto value = GetSettingsValue("settings_debug", "is_debug_mode_enabled");
    if (!value) {
        *out_flag = true;
        return RESULT_SUCCESS;
    }

    ASSERT_DEBUG(value->type == settings::SettingDataType::Boolean, Services,
                 "Invalid type for debug mode flag");
    *out_flag = value->b;
    return RESULT_SUCCESS;
}

result_t ISystemSettingsServer::GetDeviceNickName(
    OutBuffer<BufferAttr::MapAlias> out_buffer) {
    // TODO: make this configurable
    out_buffer.stream->Write<DeviceNickName>({
        .name = "Hydra device",
    });
    return RESULT_SUCCESS;
}

} // namespace hydra::horizon::services::settings
