#include "core/horizon/services/audio/audio_renderer.hpp"

#include "core/debugger/debugger_manager.hpp"
#include "core/horizon/kernel/process.hpp"

namespace hydra::horizon::services::audio {

DEFINE_SERVICE_COMMAND_TABLE(IAudioRenderer, 4, RequestUpdate, 5, Start, 6,
                             Stop, 7, QuerySystemEvent, 8,
                             SetRenderingTimeLimit, 9, GetRenderingTimeLimit,
                             10, RequestUpdateAuto)

IAudioRenderer::IAudioRenderer(const AudioRendererParameters& params,
                               std::span<u8> work_buffer)
    : renderer(params, work_buffer), event{new kernel::Event(
                                         false, "IAudioRenderer event")} {
    // HACK: create a thread that signals the handle every so often
    new std::thread([&]() {
        GET_CURRENT_PROCESS_DEBUGGER().RegisterThisThread("Audren signal");
        while (true) {
            event->Signal();
            std::this_thread::sleep_for(std::chrono::microseconds(2));
        }
        GET_CURRENT_PROCESS_DEBUGGER().UnregisterThisThread();
    });
}

result_t
IAudioRenderer::RequestUpdate(kernel::Process* process,
                              InBuffer<BufferAttr::MapAlias> in_buffer,
                              OutBuffer<BufferAttr::MapAlias> out_buffer,
                              OutBuffer<BufferAttr::MapAlias> out_perf_buffer) {
    return RequestUpdateImpl(process, in_buffer.stream, out_buffer.stream,
                             out_perf_buffer.stream);
}

result_t
IAudioRenderer::QuerySystemEvent(kernel::Process* process,
                                 OutHandle<HandleAttr::Copy> out_handle) {
    out_handle = process->AddHandle(event);
    return RESULT_SUCCESS;
}

result_t IAudioRenderer::SetRenderingTimeLimit(u32 time_limit) {
    rendering_time_limit = time_limit;
    return RESULT_SUCCESS;
}

result_t IAudioRenderer::GetRenderingTimeLimit(u32* out_time_limit) {
    *out_time_limit = rendering_time_limit;
    return RESULT_SUCCESS;
}

result_t IAudioRenderer::RequestUpdateAuto(
    kernel::Process* process, InBuffer<BufferAttr::AutoSelect> in_buffer,
    OutBuffer<BufferAttr::AutoSelect> out_buffer,
    OutBuffer<BufferAttr::AutoSelect> out_perf_buffer) {
    return RequestUpdateImpl(process, in_buffer.stream, out_buffer.stream,
                             out_perf_buffer.stream);
}

result_t IAudioRenderer::RequestUpdateImpl(kernel::Process* process,
                                           io::MemoryStream* in_stream,
                                           io::MemoryStream* out_stream,
                                           io::MemoryStream* out_perf_stream) {
    // Header
    const auto in_header = in_stream->Read<UpdateDataHeader>();

    // TODO: correct?
    auto& header = out_stream->WriteReturningRef<UpdateDataHeader>();
    header.revision = in_header.revision; // make_magic4('R', 'E', 'V', '4');
    header.total_size = sizeof(UpdateDataHeader);

    // Behavior info in
    auto res = renderer.UpdateBehaviorInfoIn(in_stream);
    if (res != RESULT_SUCCESS)
        return res;

    // Mempools
    res = renderer.UpdateMemoryPools(in_stream, process->GetMmu(), header,
                                     out_stream);
    if (res != RESULT_SUCCESS)
        return res;

    // Channels
    res = renderer.UpdateChannels(in_stream, header, out_stream);
    if (res != RESULT_SUCCESS)
        return res;

    // Voices
    res = renderer.UpdateVoices(in_stream, header, out_stream);
    if (res != RESULT_SUCCESS)
        return res;

    // Effects
    res = renderer.UpdateEffects(in_stream, header, out_stream);
    if (res != RESULT_SUCCESS)
        return res;

    // Splitter
    // TODO

    // Mixes
    res = renderer.UpdateMixes(in_stream, header, out_stream);
    if (res != RESULT_SUCCESS)
        return res;

    // Sinks
    res = renderer.UpdateSinks(in_stream, header, out_stream);
    if (res != RESULT_SUCCESS)
        return res;

    // Performance buffer
    res = renderer.UpdatePerformanceBuffer(in_stream, header, out_stream,
                                           out_perf_stream);
    if (res != RESULT_SUCCESS)
        return res;

    // Behavior info out
    res = renderer.UpdateBehaviorInfoOut(header, out_stream);
    if (res != RESULT_SUCCESS)
        return res;

    // Renderer info
    // TODO

    return RESULT_SUCCESS;
}

} // namespace hydra::horizon::services::audio
