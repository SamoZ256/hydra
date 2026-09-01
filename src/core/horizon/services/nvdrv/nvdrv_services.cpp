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

DEFINE_SERVICE_COMMAND_TABLE(INvDrvServices, 0, Open, 1, Ioctl, 2, Close, 3,
                             Initialize, 4, QueryEvent, 8, SetAruid, 11, Ioctl2,
                             12, Ioctl3, 13,
                             SetGraphicsFirmwareMemoryMarginEnabled)

result_t INvDrvServices::Open(InBuffer<BufferAttr::MapAlias> path_buffer,
                              u32* out_fd_id, u32* out_error) {
    auto path = path_buffer.stream->readNullTerminatedString();
    Handle fd_handle;
    if (path == "/dev/nvhost-ctrl") {
        fd_handle =
            fd_pool.Insert(std::make_unique<ioctl::NvHostCtrl>()).value();
    } else if (path == "/dev/nvmap") {
        fd_handle = fd_pool.Insert(std::make_unique<ioctl::NvMap>()).value();
    } else if (path == "/dev/nvhost-as-gpu") {
        fd_handle =
            fd_pool.Insert(std::make_unique<ioctl::NvHostAsGpu>()).value();
    } else if (path == "/dev/nvhost-ctrl-gpu") {
        fd_handle =
            fd_pool.Insert(std::make_unique<ioctl::NvHostCtrlGpu>()).value();
    } else if (path == "/dev/nvhost-gpu") {
        fd_handle =
            fd_pool.Insert(std::make_unique<ioctl::NvHostGpu>()).value();
    } else if (path == "/dev/nvhost-nvdec") {
        fd_handle =
            fd_pool.Insert(std::make_unique<ioctl::NvHostNvDec>()).value();
    } else if (path == "/dev/nvsched-ctrl") {
        fd_handle =
            fd_pool.Insert(std::make_unique<ioctl::NvSchedCtrl>()).value();
    } else if (path == "/dev/nvdisp-ctrl") {
        fd_handle =
            fd_pool.Insert(std::make_unique<ioctl::NvDispCtrl>()).value();
    } else if (path == "/dev/nvdisp-disp0") {
        fd_handle =
            fd_pool.Insert(std::make_unique<ioctl::NvDispDisp>(0)).value();
    } else if (path == "/dev/nvdisp-disp1") {
        fd_handle =
            fd_pool.Insert(std::make_unique<ioctl::NvDispDisp>(1)).value();
    } else if (path == "/dev/nvhost-vic") {
        fd_handle =
            fd_pool.Insert(std::make_unique<ioctl::NvHostVic>()).value();
    } else if (path == "/dev/nvhost-nvjpg") {
        fd_handle =
            fd_pool.Insert(std::make_unique<ioctl::NvHostNvJpg>()).value();
    } else {
        LOG_WARN(Services, "Unknown path \"{}\"", path);
        *out_error = MAKE_RESULT(Svc, 0); // TODO
        return MAKE_RESULT(Svc, 0);       // TODO
    }

    *out_fd_id = fd_handle.GetRaw();
    *out_error = 0;
    return RESULT_SUCCESS;
}

result_t INvDrvServices::Ioctl(System* system, kernel::Process* process,
                               Handle fd_handle, u32 code,
                               InBuffer<BufferAttr::AutoSelect> in_buffer,
                               NvResult* out_result,
                               OutBuffer<BufferAttr::AutoSelect> out_buffer) {
    return IoctlImpl(&ioctl::FdBase::Ioctl, *system, process, fd_handle, code,
                     in_buffer.stream, std::nullopt, out_buffer.stream,
                     std::nullopt, out_result);
}

result_t INvDrvServices::Close(u32 fd_handle, u32* out_err) {
    if (!fd_pool.Free(fd_handle)) {
        // TODO: what to do?
        return MAKE_RESULT(Svc, 4);
    }

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

result_t INvDrvServices::QueryEvent(kernel::Process* process, Handle fd_handle,
                                    u32 event_id, NvResult* out_result,
                                    OutHandle<HandleAttr::Copy> out_handle) {
    ZTD_ASSIGN_OR_RETURN_VALUE(auto fd, fd_pool.Get(fd_handle),
                               MAKE_RESULT(Svc, 4)); // TODO: result

    // Dispatch
    kernel::Event* event = nullptr;
    NvResult result = fd->get()->QueryEvent(event_id, event);

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
                                Handle fd_handle, u32 code,
                                InBuffer<BufferAttr::AutoSelect> in_buffer1,
                                InBuffer<BufferAttr::AutoSelect> in_buffer2,
                                NvResult* out_result,
                                OutBuffer<BufferAttr::AutoSelect> out_buffer) {
    return IoctlImpl(&ioctl::FdBase::Ioctl2, *system, process, fd_handle, code,
                     in_buffer1.stream, in_buffer2.stream, out_buffer.stream,
                     std::nullopt, out_result);
}

result_t INvDrvServices::Ioctl3(System* system, kernel::Process* process,
                                Handle fd_handle, u32 code,
                                InBuffer<BufferAttr::AutoSelect> in_buffer,
                                NvResult* out_result,
                                OutBuffer<BufferAttr::AutoSelect> out_buffer1,
                                OutBuffer<BufferAttr::AutoSelect> out_buffer2) {
    return IoctlImpl(&ioctl::FdBase::Ioctl3, *system, process, fd_handle, code,
                     in_buffer.stream, std::nullopt, out_buffer1.stream,
                     out_buffer2.stream, out_result);
}

result_t INvDrvServices::IoctlImpl(
    NvResult (ioctl::FdBase::*func)(ioctl::IoctlContext& context, u32 type,
                                    u32 nr),
    System& system, kernel::Process* process, Handle fd_handle, u32 code,
    std::optional<ztd::io::MemoryStream> in_stream,
    std::optional<ztd::io::MemoryStream> in_buffer_stream,
    std::optional<ztd::io::MemoryStream> out_stream,
    std::optional<ztd::io::MemoryStream> out_buffer_stream,
    NvResult* out_result) {
    ZTD_ASSIGN_OR_RETURN_VALUE(auto fd, fd_pool.Get(fd_handle),
                               MAKE_RESULT(Svc, 4)); // TODO: result

    // Dispatch
    u32 type = (code >> 8) & 0xff;
    u32 nr = code & 0xff;

    ioctl::IoctlContext context{
        .system = system,
        .process = process,
        .in_stream = std::move(in_stream),
        .in_buffer_stream = std::move(in_buffer_stream),
        .out_stream = std::move(out_stream),
        .out_buffer_stream = std::move(out_buffer_stream),
    };
    NvResult result = (fd->get()->*func)(context, type, nr);

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
