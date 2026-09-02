#include "core/horizon/services/audio/audio_device.hpp"

#include "core/horizon/kernel/process.hpp"

namespace hydra::horizon::services::audio {

namespace {

struct DeviceName {
    char name[0x100];
};

} // namespace

DEFINE_SERVICE_COMMAND_TABLE(
    IAudioDevice, 0, ListAudioDeviceName, 1, SetAudioDeviceOutputVolume, 2,
    GetAudioDeviceOutputVolume, 3, GetActiveAudioDeviceName, 4,
    QueryAudioDeviceSystemEvent, 5, GetActiveChannelCount, 6,
    ListAudioDeviceNameAuto, 7, SetAudioDeviceOutputVolumeAuto, 8,
    GetAudioDeviceOutputVolumeAuto, 10, GetActiveAudioDeviceNameAuto)

result_t
IAudioDevice::ListAudioDeviceName(i32* out_count,
                                  OutBuffer<BufferAttr::MapAlias> out_buffer) {
    return ListAudioDeviceNameImpl(out_count, out_buffer.stream);
}

result_t IAudioDevice::SetAudioDeviceOutputVolume(
    f32 volume, InBuffer<BufferAttr::MapAlias> in_name_buffer) {
    return SetAudioDeviceOutputVolumeImpl(volume, in_name_buffer.stream);
}

result_t IAudioDevice::GetAudioDeviceOutputVolume(
    InBuffer<BufferAttr::MapAlias> in_name_buffer, f32* out_volume) {
    return GetAudioDeviceOutputVolumeImpl(in_name_buffer.stream, out_volume);
}

result_t IAudioDevice::GetActiveAudioDeviceName(
    OutBuffer<BufferAttr::MapAlias> out_buffer) {
    return GetActiveAudioDeviceNameImpl(out_buffer.stream);
}

result_t IAudioDevice::QueryAudioDeviceSystemEvent(
    kernel::Process* process, OutHandle<HandleAttr::Copy> out_handle) {
    LOG_FUNC_STUBBED(Services);

    // TODO: choose based on name?
    out_handle = process->AddHandle(event);
    return RESULT_SUCCESS;
}

result_t IAudioDevice::GetActiveChannelCount(i32* out_count) {
    LOG_FUNC_STUBBED(Services);

    // HACK
    *out_count = 0;
    return RESULT_SUCCESS;
}

result_t IAudioDevice::ListAudioDeviceNameAuto(
    i32* out_count, OutBuffer<BufferAttr::AutoSelect> out_buffer) {
    return ListAudioDeviceNameImpl(out_count, out_buffer.stream);
}

result_t IAudioDevice::SetAudioDeviceOutputVolumeAuto(
    f32 volume, InBuffer<BufferAttr::AutoSelect> in_name_buffer) {
    return SetAudioDeviceOutputVolumeImpl(volume, in_name_buffer.stream);
}

result_t IAudioDevice::GetAudioDeviceOutputVolumeAuto(
    InBuffer<BufferAttr::AutoSelect> in_name_buffer, f32* out_volume) {
    return GetAudioDeviceOutputVolumeImpl(in_name_buffer.stream, out_volume);
}

result_t IAudioDevice::GetActiveAudioDeviceNameAuto(
    OutBuffer<BufferAttr::AutoSelect> out_buffer) {
    return GetActiveAudioDeviceNameImpl(out_buffer.stream);
}

result_t IAudioDevice::ListAudioDeviceNameImpl(
    i32* out_count, std::optional<ztd::io::MemoryStream> out_stream) {
    LOG_FUNC_STUBBED(Services);

    // HACK
    *out_count = 1;
    out_stream->write<DeviceName>({"Hydra audio device"});
    return RESULT_SUCCESS;
}

result_t IAudioDevice::SetAudioDeviceOutputVolumeImpl(
    f32 volume, std::optional<ztd::io::MemoryStream> in_name_stream) {
    const auto device_name_raw = in_name_stream->readPtr<DeviceName>();
    const std::string device_name(device_name_raw->name);
    LOG_FUNC_WITH_ARGS_STUBBED(Services, "name: {}, volume: {}", device_name,
                               volume);

    return RESULT_SUCCESS;
}

result_t IAudioDevice::GetAudioDeviceOutputVolumeImpl(
    std::optional<ztd::io::MemoryStream> in_name_stream, f32* out_volume) {
    const auto device_name_raw = in_name_stream->readPtr<DeviceName>();
    const std::string device_name(device_name_raw->name);
    LOG_FUNC_WITH_ARGS_STUBBED(Services, "name: {}", device_name);

    // HACK
    *out_volume = 1.0f;
    return RESULT_SUCCESS;
}

result_t IAudioDevice::GetActiveAudioDeviceNameImpl(
    std::optional<ztd::io::MemoryStream> out_stream) {
    LOG_FUNC_STUBBED(Services);

    // HACK
    out_stream->write<DeviceName>({"Hydra audio device"});
    return RESULT_SUCCESS;
}

} // namespace hydra::horizon::services::audio
