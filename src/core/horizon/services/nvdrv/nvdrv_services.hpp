#pragma once

#include "core/horizon/services/const.hpp"
#include "core/horizon/services/nvdrv/const.hpp"

namespace hydra::horizon::services::nvdrv {

namespace ioctl {
class FdBase;
}

constexpr usize MAX_FD_COUNT = 256;

class INvDrvServices : public ServiceBase {
  protected:
    result_t RequestImpl(RequestContext& context, u32 id) override;

  private:
    // TODO: what should be the max number of fds?
    static StaticPool<ioctl::FdBase*, MAX_FD_COUNT> fd_pool;

    // Commands
    result_t Open(InBuffer<BufferAttr::MapAlias> path_buffer, u32* out_fd_id,
                  u32* out_error);
    result_t Ioctl(handle_id_t fd_id, u32 code,
                   InBuffer<BufferAttr::AutoSelect> in_buffer,
                   NvResult* out_result,
                   OutBuffer<BufferAttr::AutoSelect> out_buffer);
    result_t Close(u32 fd_id, u32* out_err);
    result_t Initialize(u32 transfer_mem_size, NvResult* out_result);
    result_t QueryEvent(handle_id_t fd_id, u32 event_id, NvResult* out_result,
                        OutHandle<HandleAttr::Copy> out_handle);
    STUB_REQUEST_COMMAND(SetAruid);
    result_t Ioctl2(handle_id_t fd_id, u32 code,
                    InBuffer<BufferAttr::AutoSelect> in_buffer1,
                    InBuffer<BufferAttr::AutoSelect> in_buffer2,
                    NvResult* out_result,
                    OutBuffer<BufferAttr::AutoSelect> out_buffer);
    result_t Ioctl3(handle_id_t fd_id, u32 code,
                    InBuffer<BufferAttr::AutoSelect> in_buffer,
                    NvResult* out_result,
                    OutBuffer<BufferAttr::AutoSelect> out_buffer1,
                    OutBuffer<BufferAttr::AutoSelect> out_buffer2);
    STUB_REQUEST_COMMAND(SetGraphicsFirmwareMemoryMarginEnabled);

    result_t IoctlImpl(handle_id_t fd_id, u32 code, Reader* reader,
                       Reader* buffer_reader, Writer* writer,
                       Writer* buffer_writer, NvResult* out_result);
};

} // namespace hydra::horizon::services::nvdrv
