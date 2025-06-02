#include "core/horizon/services/audio/audio_device.hpp"

namespace hydra::horizon::services::audio {

namespace {

struct DeviceName {
    char name[0x100];
};

} // namespace

DEFINE_SERVICE_COMMAND_TABLE(IAudioDevice, 0, ListAudioDeviceName, 1,
                             SetAudioDeviceOutputVolume, 3,
                             GetActiveAudioDeviceName, 4,
                             QueryAudioDeviceSystemEvent, 5,
                             GetActiveChannelCount, 6, ListAudioDeviceNameAuto,
                             7, SetAudioDeviceOutputVolumeAuto, 10,
                             GetActiveAudioDeviceNameAuto)

// TODO: autoclear event?
IAudioDevice::IAudioDevice() : event(new kernel::Event()) {}

result_t
IAudioDevice::ListAudioDeviceName(i32* out_count,
                                  OutBuffer<BufferAttr::MapAlias> out_buffer) {
    return ListAudioDeviceNameImpl(out_count, *out_buffer.writer);
}

result_t IAudioDevice::SetAudioDeviceOutputVolume(
    f32 volume, InBuffer<BufferAttr::MapAlias> in_name_buffer) {
    return SetAudioDeviceOutputVolumeImpl(volume, *in_name_buffer.reader);
}

result_t IAudioDevice::GetActiveAudioDeviceName(
    OutBuffer<BufferAttr::MapAlias> out_buffer) {
    return GetActiveAudioDeviceNameImpl(*out_buffer.writer);
}

result_t IAudioDevice::QueryAudioDeviceSystemEvent(
    OutHandle<HandleAttr::Copy> out_handle) {
    LOG_FUNC_STUBBED(Services);

    // TODO: choose based on name?
    out_handle = event.id;
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
    return ListAudioDeviceNameImpl(out_count, *out_buffer.writer);
}

result_t IAudioDevice::SetAudioDeviceOutputVolumeAuto(
    f32 volume, InBuffer<BufferAttr::AutoSelect> in_name_buffer) {
    return SetAudioDeviceOutputVolumeImpl(volume, *in_name_buffer.reader);
}

result_t IAudioDevice::GetActiveAudioDeviceNameAuto(
    OutBuffer<BufferAttr::AutoSelect> out_buffer) {
    return GetActiveAudioDeviceNameImpl(*out_buffer.writer);
}

result_t IAudioDevice::ListAudioDeviceNameImpl(i32* out_count,
                                               Writer& out_writer) {
    LOG_FUNC_STUBBED(Services);

    // HACK
    *out_count = 1;
    out_writer.Write<DeviceName>({"Hydra audio device"});
    return RESULT_SUCCESS;
}

result_t IAudioDevice::SetAudioDeviceOutputVolumeImpl(f32 volume,
                                                      Reader& in_name_reader) {
    LOG_FUNC_STUBBED(Services);

    const auto device_name_raw = in_name_reader.ReadPtr<DeviceName>();
    const std::string device_name(device_name_raw->name);
    LOG_DEBUG(Services, "Name: {}, volume: {}", device_name, volume);

    return RESULT_SUCCESS;
}

result_t IAudioDevice::GetActiveAudioDeviceNameImpl(Writer& out_writer) {
    LOG_FUNC_STUBBED(Services);

    // HACK
    out_writer.Write<DeviceName>({"Hydra audio device"});
    return RESULT_SUCCESS;
}

} // namespace hydra::horizon::services::audio
