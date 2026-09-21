#include "core/horizon/services/audio/audio_device.hpp"

#include "core/horizon/kernel/process.hpp"

namespace hydra::horizon::services::audio {

namespace {

struct DeviceName {
    char name[0x100];
};

} // namespace

DEFINE_SERVICE_COMMAND_TABLE(
    IAudioDevice, 0, listAudioDeviceName, 1, setAudioDeviceOutputVolume, 2,
    getAudioDeviceOutputVolume, 3, getActiveAudioDeviceName, 4,
    queryAudioDeviceSystemEvent, 5, getActiveChannelCount, 6,
    listAudioDeviceNameAuto, 7, setAudioDeviceOutputVolumeAuto, 8,
    getAudioDeviceOutputVolumeAuto, 10, getActiveAudioDeviceNameAuto)

result_t
IAudioDevice::listAudioDeviceName(i32* out_count,
                                  OutBuffer<BufferAttr::MapAlias> out_buffer) {
    return listAudioDeviceNameImpl(out_count, out_buffer.stream);
}

result_t IAudioDevice::setAudioDeviceOutputVolume(
    f32 volume, InBuffer<BufferAttr::MapAlias> in_name_buffer) {
    return setAudioDeviceOutputVolumeImpl(volume, in_name_buffer.stream);
}

result_t IAudioDevice::getAudioDeviceOutputVolume(
    InBuffer<BufferAttr::MapAlias> in_name_buffer, f32* out_volume) {
    return getAudioDeviceOutputVolumeImpl(in_name_buffer.stream, out_volume);
}

result_t IAudioDevice::getActiveAudioDeviceName(
    OutBuffer<BufferAttr::MapAlias> out_buffer) {
    return getActiveAudioDeviceNameImpl(out_buffer.stream);
}

result_t IAudioDevice::queryAudioDeviceSystemEvent(
    kernel::Process* process, OutHandle<HandleAttr::Copy> out_handle) {
    LOG_FUNC_STUBBED(Services);

    // TODO: choose based on name?
    out_handle = process->addHandle(event);
    return RESULT_SUCCESS;
}

result_t IAudioDevice::getActiveChannelCount(i32* out_count) {
    LOG_FUNC_STUBBED(Services);

    // HACK
    *out_count = 0;
    return RESULT_SUCCESS;
}

result_t IAudioDevice::listAudioDeviceNameAuto(
    i32* out_count, OutBuffer<BufferAttr::AutoSelect> out_buffer) {
    return listAudioDeviceNameImpl(out_count, out_buffer.stream);
}

result_t IAudioDevice::setAudioDeviceOutputVolumeAuto(
    f32 volume, InBuffer<BufferAttr::AutoSelect> in_name_buffer) {
    return setAudioDeviceOutputVolumeImpl(volume, in_name_buffer.stream);
}

result_t IAudioDevice::getAudioDeviceOutputVolumeAuto(
    InBuffer<BufferAttr::AutoSelect> in_name_buffer, f32* out_volume) {
    return getAudioDeviceOutputVolumeImpl(in_name_buffer.stream, out_volume);
}

result_t IAudioDevice::getActiveAudioDeviceNameAuto(
    OutBuffer<BufferAttr::AutoSelect> out_buffer) {
    return getActiveAudioDeviceNameImpl(out_buffer.stream);
}

result_t IAudioDevice::listAudioDeviceNameImpl(
    i32* out_count, std::optional<ztd::io::MemoryStream> out_stream) {
    LOG_FUNC_STUBBED(Services);

    // HACK
    *out_count = 1;
    out_stream->write<DeviceName>({"Hydra audio device"});
    return RESULT_SUCCESS;
}

result_t IAudioDevice::setAudioDeviceOutputVolumeImpl(
    f32 volume, std::optional<ztd::io::MemoryStream> in_name_stream) {
    const auto device_name_raw = in_name_stream->readPtr<DeviceName>();
    const std::string device_name(device_name_raw->name);
    LOG_FUNC_WITH_ARGS_STUBBED(Services, "name: {}, volume: {}", device_name,
                               volume);

    return RESULT_SUCCESS;
}

result_t IAudioDevice::getAudioDeviceOutputVolumeImpl(
    std::optional<ztd::io::MemoryStream> in_name_stream, f32* out_volume) {
    const auto device_name_raw = in_name_stream->readPtr<DeviceName>();
    const std::string device_name(device_name_raw->name);
    LOG_FUNC_WITH_ARGS_STUBBED(Services, "name: {}", device_name);

    // HACK
    *out_volume = 1.0f;
    return RESULT_SUCCESS;
}

result_t IAudioDevice::getActiveAudioDeviceNameImpl(
    std::optional<ztd::io::MemoryStream> out_stream) {
    LOG_FUNC_STUBBED(Services);

    // HACK
    out_stream->write<DeviceName>({"Hydra audio device"});
    return RESULT_SUCCESS;
}

} // namespace hydra::horizon::services::audio
