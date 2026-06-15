#pragma once

#include "core/horizon/kernel/kernel.hpp"
#include "core/horizon/services/audio/const.hpp"
#include "core/horizon/services/const.hpp"

namespace hydra::horizon::services::audio {

class IAudioDevice : public IService {
  public:
    IAudioDevice() : event{new kernel::Event(false, "IAudioDevice event")} {}

  protected:
    result_t RequestImpl([[maybe_unused]] RequestContext& context,
                         u32 id) override;

  private:
    kernel::Event* event;

    // Commands
    result_t ListAudioDeviceName(i32* out_count,
                                 OutBuffer<BufferAttr::MapAlias> out_buffer);
    result_t
    SetAudioDeviceOutputVolume(f32 volume,
                               InBuffer<BufferAttr::MapAlias> in_name_buffer);
    result_t
    GetAudioDeviceOutputVolume(InBuffer<BufferAttr::MapAlias> in_name_buffer,
                               f32* out_volume);
    result_t
    GetActiveAudioDeviceName(OutBuffer<BufferAttr::MapAlias> out_buffer);
    // TODO: takes a name?
    // TODO: check handle attrs
    result_t
    QueryAudioDeviceSystemEvent(kernel::Process* process,
                                OutHandle<HandleAttr::Copy> out_handle);
    // TODO: params
    result_t GetActiveChannelCount(i32* out_count);
    result_t
    ListAudioDeviceNameAuto(i32* out_count,
                            OutBuffer<BufferAttr::AutoSelect> out_buffer);
    result_t SetAudioDeviceOutputVolumeAuto(
        f32 volume, InBuffer<BufferAttr::AutoSelect> in_name_buffer);
    result_t GetAudioDeviceOutputVolumeAuto(
        InBuffer<BufferAttr::AutoSelect> in_name_buffer, f32* out_volume);
    result_t
    GetActiveAudioDeviceNameAuto(OutBuffer<BufferAttr::AutoSelect> out_buffer);

    // Impl
    result_t
    ListAudioDeviceNameImpl(i32* out_count,
                            std::optional<io::MemoryStream> out_stream);
    result_t SetAudioDeviceOutputVolumeImpl(
        f32 volume, std::optional<io::MemoryStream> in_name_stream);
    result_t GetAudioDeviceOutputVolumeImpl(
        std::optional<io::MemoryStream> in_name_stream, f32* out_volume);
    result_t
    GetActiveAudioDeviceNameImpl(std::optional<io::MemoryStream> out_stream);
};

} // namespace hydra::horizon::services::audio
