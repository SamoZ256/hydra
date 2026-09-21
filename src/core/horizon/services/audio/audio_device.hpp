#pragma once

#include "core/horizon/kernel/kernel.hpp"
#include "core/horizon/services/audio/const.hpp"
#include "core/horizon/services/const.hpp"

namespace hydra::horizon::services::audio {

class IAudioDevice : public IService {
  public:
    IAudioDevice() : event{new kernel::Event(false, "IAudioDevice event")} {}

  protected:
    result_t requestImpl([[maybe_unused]] RequestContext& context,
                         u32 id) override;

  private:
    kernel::Event* event;

    // Commands
    result_t listAudioDeviceName(i32* out_count,
                                 OutBuffer<BufferAttr::MapAlias> out_buffer);
    result_t
    setAudioDeviceOutputVolume(f32 volume,
                               InBuffer<BufferAttr::MapAlias> in_name_buffer);
    result_t
    getAudioDeviceOutputVolume(InBuffer<BufferAttr::MapAlias> in_name_buffer,
                               f32* out_volume);
    result_t
    getActiveAudioDeviceName(OutBuffer<BufferAttr::MapAlias> out_buffer);
    // TODO: takes a name?
    // TODO: check handle attrs
    result_t
    queryAudioDeviceSystemEvent(kernel::Process* process,
                                OutHandle<HandleAttr::Copy> out_handle);
    // TODO: params
    result_t getActiveChannelCount(i32* out_count);
    result_t
    listAudioDeviceNameAuto(i32* out_count,
                            OutBuffer<BufferAttr::AutoSelect> out_buffer);
    result_t setAudioDeviceOutputVolumeAuto(
        f32 volume, InBuffer<BufferAttr::AutoSelect> in_name_buffer);
    result_t getAudioDeviceOutputVolumeAuto(
        InBuffer<BufferAttr::AutoSelect> in_name_buffer, f32* out_volume);
    result_t
    getActiveAudioDeviceNameAuto(OutBuffer<BufferAttr::AutoSelect> out_buffer);

    // Impl
    result_t
    listAudioDeviceNameImpl(i32* out_count,
                            std::optional<ztd::io::MemoryStream> out_stream);
    result_t setAudioDeviceOutputVolumeImpl(
        f32 volume, std::optional<ztd::io::MemoryStream> in_name_stream);
    result_t getAudioDeviceOutputVolumeImpl(
        std::optional<ztd::io::MemoryStream> in_name_stream, f32* out_volume);
    result_t getActiveAudioDeviceNameImpl(
        std::optional<ztd::io::MemoryStream> out_stream);
};

} // namespace hydra::horizon::services::audio
