#include "core/horizon/services/nvdrv/nvdrv_services.hpp"

#include "core/horizon/const.hpp"
#include "core/horizon/kernel/process.hpp"
#include "core/horizon/services/nvdrv/const.hpp"
#include "core/horizon/services/nvdrv/ioctl/nvdisp_ctrl.hpp"
#include "core/horizon/services/nvdrv/ioctl/nvdisp_disp.hpp"
#include "core/horizon/services/nvdrv/ioctl/nvhost_as_gpu.hpp"
#include "core/horizon/services/nvdrv/ioctl/nvhost_ctrl.hpp"
#include "core/horizon/services/nvdrv/ioctl/nvhost_ctrl_gpu.hpp"
#include "core/horizon/services/nvdrv/ioctl/nvhost_gpu.hpp"
#include "core/horizon/services/nvdrv/ioctl/nvhost_nvdec.hpp"
#include "core/horizon/services/nvdrv/ioctl/nvhost_nvjpg.hpp"
#include "core/horizon/services/nvdrv/ioctl/nvhost_vic.hpp"
#include "core/horizon/services/nvdrv/ioctl/nvmap.hpp"
#include "core/horizon/services/nvdrv/ioctl/nvsched_ctrl.hpp"

namespace hydra::horizon::services::nvdrv {

StaticPool<ioctl::FdBase*, MAX_FD_COUNT> INvDrvServices::fd_pool;

DEFINE_SERVICE_COMMAND_TABLE(INvDrvServices, 0, Open, 1, Ioctl, 2, Close, 3,
                             Initialize, 4, QueryEvent, 8, SetAruid, 11, Ioctl2,
                             12, Ioctl3, 13,
                             SetGraphicsFirmwareMemoryMarginEnabled)

result_t INvDrvServices::Open(InBuffer<BufferAttr::MapAlias> path_buffer,
                              u32* out_fd_id, u32* out_error) {
    auto path = path_buffer.stream->ReadNullTerminatedString();
    handle_id_t fd_id = fd_pool.AllocateHandle();
    if (path == "/dev/nvhost-ctrl") {
        fd_pool.Get(fd_id) = new ioctl::NvHostCtrl();
    } else if (path == "/dev/nvmap") {
        fd_pool.Get(fd_id) = new ioctl::NvMap();
    } else if (path == "/dev/nvhost-as-gpu") {
        fd_pool.Get(fd_id) = new ioctl::NvHostAsGpu();
    } else if (path == "/dev/nvhost-ctrl-gpu") {
        fd_pool.Get(fd_id) = new ioctl::NvHostCtrlGpu();
    } else if (path == "/dev/nvhost-gpu") {
        fd_pool.Get(fd_id) = new ioctl::NvHostGpu();
    } else if (path == "/dev/nvhost-nvdec") {
        fd_pool.Get(fd_id) = new ioctl::NvHostNvDec();
    } else if (path == "/dev/nvsched-ctrl") {
        fd_pool.Get(fd_id) = new ioctl::NvSchedCtrl();
    } else if (path == "/dev/nvdisp-ctrl") {
        fd_pool.Get(fd_id) = new ioctl::NvDispCtrl();
    } else if (path == "/dev/nvdisp-disp0") {
        fd_pool.Get(fd_id) = new ioctl::NvDispDisp(0);
    } else if (path == "/dev/nvdisp-disp1") {
        fd_pool.Get(fd_id) = new ioctl::NvDispDisp(1);
    } else if (path == "/dev/nvhost-vic") {
        fd_pool.Get(fd_id) = new ioctl::NvHostVic();
    } else if (path == "/dev/nvhost-nvjpg") {
        fd_pool.Get(fd_id) = new ioctl::NvHostNvJpg();
    } else {
        LOG_WARN(Services, "Unknown path \"{}\"", path);
        *out_error = MAKE_RESULT(Svc, 0); // TODO
        return MAKE_RESULT(Svc, 0);       // TODO
    }

    *out_fd_id = fd_id;
    *out_error = 0;
    return RESULT_SUCCESS;
}

result_t INvDrvServices::Ioctl(System* system, kernel::Process* process,
                               handle_id_t fd_id, u32 code,
                               InBuffer<BufferAttr::AutoSelect> in_buffer,
                               NvResult* out_result,
                               OutBuffer<BufferAttr::AutoSelect> out_buffer) {
    return IoctlImpl(&ioctl::FdBase::Ioctl, *system, process, fd_id, code,
                     in_buffer.stream, nullptr, out_buffer.stream, nullptr,
                     out_result);
}

result_t INvDrvServices::Close(u32 fd_id, u32* out_err) {
    // TODO: check if exists
    auto fd = fd_pool.Get(fd_id);
    delete fd;
    fd_pool.Free(fd_id);

    *out_err = 0;
    return RESULT_SUCCESS;
}

result_t INvDrvServices::Initialize(u32 transfer_mem_size,
                                    NvResult* out_result) {
    LOG_FUNC_WITH_ARGS_STUBBED(Services, "transfer_mem_size: {:x}",
                               transfer_mem_size);

    // TODO: read process and transfer mem handle IDs

    *out_result = NvResult::Success;
    return RESULT_SUCCESS;
}

result_t INvDrvServices::QueryEvent(kernel::Process* process, handle_id_t fd_id,
                                    u32 event_id, NvResult* out_result,
                                    OutHandle<HandleAttr::Copy> out_handle) {
    auto fd = fd_pool.Get(fd_id);

    // Dispatch
    kernel::Event* event = nullptr;
    NvResult result = fd->QueryEvent(event_id, event);

    // Write result
    *out_result = result;
    if (result == NvResult::Success) {
        out_handle = process->AddHandle(event);
        return RESULT_SUCCESS;
    } else {
        return MAKE_RESULT(
            Svc,
            kernel::Error::NotFound); // TODO: what should this be?
    }
}

result_t INvDrvServices::Ioctl2(System* system, kernel::Process* process,
                                handle_id_t fd_id, u32 code,
                                InBuffer<BufferAttr::AutoSelect> in_buffer1,
                                InBuffer<BufferAttr::AutoSelect> in_buffer2,
                                NvResult* out_result,
                                OutBuffer<BufferAttr::AutoSelect> out_buffer) {
    return IoctlImpl(&ioctl::FdBase::Ioctl2, *system, process, fd_id, code,
                     in_buffer1.stream, in_buffer2.stream, out_buffer.stream,
                     nullptr, out_result);
}

result_t INvDrvServices::Ioctl3(System* system, kernel::Process* process,
                                handle_id_t fd_id, u32 code,
                                InBuffer<BufferAttr::AutoSelect> in_buffer,
                                NvResult* out_result,
                                OutBuffer<BufferAttr::AutoSelect> out_buffer1,
                                OutBuffer<BufferAttr::AutoSelect> out_buffer2) {
    return IoctlImpl(&ioctl::FdBase::Ioctl3, *system, process, fd_id, code,
                     in_buffer.stream, nullptr, out_buffer1.stream,
                     out_buffer2.stream, out_result);
}

result_t INvDrvServices::IoctlImpl(
    NvResult (ioctl::FdBase::*func)(ioctl::IoctlContext& context, u32 type,
                                    u32 nr),
    System& system, kernel::Process* process, handle_id_t fd_id, u32 code,
    io::MemoryStream* in_stream, io::MemoryStream* in_buffer_stream,
    io::MemoryStream* out_stream, io::MemoryStream* out_buffer_stream,
    NvResult* out_result) {
    auto fd = fd_pool.Get(fd_id);

    // Dispatch
    u32 type = (code >> 8) & 0xff;
    u32 nr = code & 0xff;

    ioctl::IoctlContext context{
        system,           process,    in_stream,
        in_buffer_stream, out_stream, out_buffer_stream,
    };
    NvResult result = (fd->*func)(context, type, nr);

    // Write result
    *out_result = result;

    if (result != NvResult::Success)
        return MAKE_RESULT(
            Svc,
            kernel::Error::NotFound); // TODO: what should this be?
    else
        return RESULT_SUCCESS;
}

} // namespace hydra::horizon::services::nvdrv
