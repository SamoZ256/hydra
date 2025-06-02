#pragma once

#include "core/horizon/services/const.hpp"

namespace hydra::horizon::services::hosbinder {

class ParcelReader;
class ParcelWriter;

enum class TransactCode : u32 {
    RequestBuffer = 1,
    SetBufferCount,
    DequeueBuffer,
    DetachBuffer,
    DetachNextBuffer,
    AttachBuffer,
    QueueBuffer,
    CancelBuffer,
    Query,
    Connect,
    Disconnect,
    SetSidebandStream,
    AllocateBuffers,
    SetPreallocatedBuffer,
};

enum class BinderType : i32 {
    Weak = 0,
    Strong = 1,
};

class IHOSBinderDriver : public ServiceBase {
  protected:
    result_t RequestImpl(RequestContext& context, u32 id) override;

  private:
    // Commands
    result_t TransactParcel(i32 binder_id, TransactCode code, u32 flags,
                            InBuffer<BufferAttr::MapAlias> in_parcel_buffer,
                            OutBuffer<BufferAttr::MapAlias> out_parcel_buffer);
    result_t AdjustRefcount(i32 binder_id, i32 add_value, BinderType type);
    result_t
    GetNativeHandle(i32 binder_id, u32 code,
                    OutHandle<HandleAttr::Copy>
                        out_handle); // TODO: should code be TransactCode?
    result_t
    TransactParcelAuto(i32 binder_id, TransactCode code, u32 flags,
                       InBuffer<BufferAttr::AutoSelect> in_parcel_buffer,
                       OutBuffer<BufferAttr::AutoSelect> out_parcel_buffer);

    void TransactParcelImpl(i32 binder_id, TransactCode code, u32 flags,
                            ParcelReader& parcel_reader,
                            ParcelWriter& parcel_writer);
};

} // namespace hydra::horizon::services::hosbinder

ENABLE_ENUM_FORMATTING(hydra::horizon::services::hosbinder::TransactCode,
                       RequestBuffer, "request buffer", SetBufferCount,
                       "set buffer count", DequeueBuffer, "dequeue buffer",
                       DetachBuffer, "detach buffer", DetachNextBuffer,
                       "detach next buffer", AttachBuffer, "attach buffer",
                       QueueBuffer, "queue buffer", CancelBuffer,
                       "cancel buffer", Query, "query", Connect, "connect",
                       Disconnect, "disconnect", SetSidebandStream,
                       "set sideband stream", AllocateBuffers,
                       "allocate buffers", SetPreallocatedBuffer,
                       "set preallocated buffer")
