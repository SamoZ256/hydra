#pragma once

#include "core/horizon/services/const.hpp"
#include "core/horizon/services/nvdrv/const.hpp"
#include "core/horizon/services/nvdrv/ioctl/fd_base.hpp"

namespace hydra::horizon::services::nvdrv {

constexpr usize MAX_FD_COUNT = 256;

class INvDrvServices : public IService {
  protected:
    result_t requestImpl([[maybe_unused]] RequestContext& context,
                         u32 id) override;

  private:
    // TODO: what should be the max number of fds?
    StaticHandlePool<std::unique_ptr<ioctl::FdBase>, MAX_FD_COUNT> fd_pool;

    // Commands
    result_t open(InBuffer<BufferAttr::MapAlias> path_buffer, u32* out_fd_id,
                  u32* out_error);
    result_t ioctl(System* system, kernel::Process* process, Handle fd_handle,
                   u32 code, InBuffer<BufferAttr::AutoSelect> in_buffer,
                   NvResult* out_result,
                   OutBuffer<BufferAttr::AutoSelect> out_buffer);
    result_t close(u32 fd_handle, u32* out_err);
    result_t initialize(u32 transfer_mem_size, NvResult* out_result);
    result_t queryEvent(kernel::Process* process, Handle fd_handle,
                        u32 event_id, NvResult* out_result,
                        OutHandle<HandleAttr::Copy> out_handle);
    STUB_REQUEST_COMMAND(setAruid);
    result_t ioctl2(System* system, kernel::Process* process, Handle fd_handle,
                    u32 code, InBuffer<BufferAttr::AutoSelect> in_buffer1,
                    InBuffer<BufferAttr::AutoSelect> in_buffer2,
                    NvResult* out_result,
                    OutBuffer<BufferAttr::AutoSelect> out_buffer);
    result_t ioctl3(System* system, kernel::Process* process, Handle fd_handle,
                    u32 code, InBuffer<BufferAttr::AutoSelect> in_buffer,
                    NvResult* out_result,
                    OutBuffer<BufferAttr::AutoSelect> out_buffer1,
                    OutBuffer<BufferAttr::AutoSelect> out_buffer2);
    STUB_REQUEST_COMMAND(setGraphicsFirmwareMemoryMarginEnabled);

    result_t
    ioctlImpl(NvResult (ioctl::FdBase::*func)(ioctl::IoctlContext& context,
                                              u32 type, u32 nr),
              System& system, kernel::Process* process, Handle fd_handle,
              u32 code, std::optional<ztd::io::MemoryStream> in_stream,
              std::optional<ztd::io::MemoryStream> in_buffer_stream,
              std::optional<ztd::io::MemoryStream> out_stream,
              std::optional<ztd::io::MemoryStream> out_buffer_stream,
              NvResult* out_result);
};

} // namespace hydra::horizon::services::nvdrv
