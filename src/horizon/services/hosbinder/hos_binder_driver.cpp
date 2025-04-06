#include "horizon/services/hosbinder/hos_binder_driver.hpp"

#include "horizon/kernel.hpp"
#include "horizon/os.hpp"
#include "hw/tegra_x1/gpu/const.hpp"

namespace Hydra::Horizon::Services::HosBinder {

namespace {

enum class BinderResult : i32 {
    Success = 0,
    PermissionDenied = -1,
    NameNotFound = -2,
    WouldBlock = -11,
    NoMemory = -12,
    AlreadyExists = -17,
    NoInit = -19,
    BadValue = -22,
    DeadObject = -32,
    InvalidOperation = -38,
    NotEnoughData = -61,
    UnknownTransaction = -74,
    BadIndex = -75,
    TimedOut = -110,
    FdsNotAllowed = INT32_MIN + 7,
    FailedTransaction = INT32_MIN + 2,
    BadType = INT32_MIN + 1,
};

// TODO: define these somewhere else
struct NvFence {
    u32 id;
    u32 value;
};

struct NvMultiFence {
    u32 num_fences;
    NvFence fences[4];
};

struct BqBufferOutput {
    u32 width;
    u32 height;
    u32 transformHint;
    u32 numPendingBuffers;
};

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

struct TransactParcelIn {
    i32 binder_id;
    TransactCode code;
    u32 flags;
};

struct InputBuffer {
    u32 magic;
    u32 width;
    u32 height;
    u32 stride;
    u32 format;
    u32 usage;

    u32 pid;
    u32 refcount;

    u32 num_fds;
    u32 num_words;
};

enum class BinderType : i32 {
    Weak = 0,
    Strong = 1,
};

struct AdjustRefcountIn {
    i32 binder_id;
    i32 addval;
    BinderType type;
};

} // namespace

DEFINE_SERVICE_COMMAND_TABLE(IHOSBinderDriver, 0, TransactParcel, 1,
                             AdjustRefcount, 2, GetNativeHandle)

void IHOSBinderDriver::TransactParcel(REQUEST_COMMAND_PARAMS) {
    auto& reader = readers.send_buffers_readers[0];
    auto& writer = writers.recv_buffers_writers[0];

    auto in = readers.reader.Read<TransactParcelIn>();
    auto parcel_in = reader.Read<Parcel>();
    reader.Seek(parcel_in.data_offset);

    auto parcel_out = writer.Write<Parcel>({.data_offset = sizeof(Parcel)});
    usize written_begin = writer.GetWrittenSize();

    // Binder
    auto& binder =
        OS::GetInstance().GetDisplayBinderManager().GetBinder(in.binder_id);

    // Dispatch
    BinderResult b_result = BinderResult::Success;
    switch (in.code) {
    case TransactCode::DequeueBuffer: {
        i32 slot = binder.GetAvailableSlot();

        writer.Write(slot);
        // HACK
        writer.Write((i32)1);

        // Flattened object
        ParcelFlattenedObject flattened_obj = {
            .size = sizeof(NvMultiFence),
            .fd_count = 0,
        };
        writer.Write(flattened_obj);

        // NvMultiFence
        NvMultiFence nv_multi_fence = {
            .num_fences = 0,
        };
        writer.Write(nv_multi_fence);

        break;
    }
    case TransactCode::QueueBuffer: {
        // Interface name
        reader.Read<i32>(); // 0x100

        i32 interface_name_len = reader.Read<i32>() + 1;
        char interface_name[interface_name_len];
        for (i32 i = 0; i < interface_name_len; i++) {
            interface_name[i] = reader.Read<u16>();
        }

        // Alignment
        if (interface_name_len % 2 != 0)
            reader.Read<u16>();

        LOG_DEBUG(HorizonServices, "Interface name: {}",
                  reinterpret_cast<const char*>(interface_name));

        // Slot
        i32 slot = reader.Read<i32>();

        // Input buffer
        auto input_buffer = reader.Read<InputBuffer>();

        binder.QueueBuffer(slot);

        // Buffer output
        // TODO
        writer.Write<BqBufferOutput>({
            .width = 0,
            .height = 0,
            .transformHint = 0,
            .numPendingBuffers = 0,
        });

        break;
    }
    case TransactCode::Connect: {
        LOG_WARNING(HorizonServices, "Connect not implemented");

        // HACK
        u64 arr[16] = {0};
        writer.Write(arr, 16);

        break;
    }
    case TransactCode::SetPreallocatedBuffer: {
        // Interface name
        reader.Read<i32>(); // 0x100

        i32 interface_name_len = reader.Read<i32>() + 1;
        char interface_name[interface_name_len];
        for (i32 i = 0; i < interface_name_len; i++) {
            interface_name[i] = reader.Read<u16>();
        }

        // Alignment
        if (interface_name_len % 2 != 0)
            reader.Read<u16>();

        LOG_DEBUG(HorizonServices, "Interface name: {}",
                  reinterpret_cast<const char*>(interface_name));

        // Slot
        i32 slot = reader.Read<i32>();

        // Input buffer
        bool has_input_buffer = reader.Read<i32>();
        if (!has_input_buffer) {
            LOG_WARNING(HorizonServices, "No input buffer");
            break;
        }

        reader.Read<i32>(); // len
        reader.Read<i32>(); // fd_count

        auto input_buffer = reader.Read<InputBuffer>();
        auto buffer = reader.ReadPtr<HW::TegraX1::GPU::NvGraphicsBuffer>();

        // pitch, size and offset seem to have completely wrong values
        LOG_DEBUG(HorizonServices,
                  "width: 0x{:08x}, height: 0x{:08x}, color_format: {}, "
                  "layout: {}, pitch: 0x{:08x}, "
                  "unused: 0x{:08x}, offset: 0x{:08x}, kind: "
                  "{}, size: 0x{:08x}",
                  buffer->planes[0].width, buffer->planes[0].height,
                  buffer->planes[0].color_format, buffer->planes[0].layout,
                  buffer->planes[0].pitch, buffer->planes[0].unused,
                  buffer->planes[0].offset, buffer->planes[0].kind,
                  buffer->planes[0].size);

        binder.AddBuffer(slot, *buffer);

        break;
    }
    default:
        LOG_WARNING(HorizonServices, "Unknown code {}", in.code);
        break;
    }

    // Result
    writer.Write(b_result);

    parcel_out->data_size = writer.GetWrittenSize() - written_begin;
}

void IHOSBinderDriver::AdjustRefcount(REQUEST_COMMAND_PARAMS) {
    auto in = readers.reader.Read<AdjustRefcountIn>();

    auto& binder =
        OS::GetInstance().GetDisplayBinderManager().GetBinder(in.binder_id);

    switch (in.type) {
    case BinderType::Weak:
        binder.weak_ref_count += in.addval;
        break;
    case BinderType::Strong:
        binder.strong_ref_count += in.addval;
        break;
    }
}

void IHOSBinderDriver::GetNativeHandle(REQUEST_COMMAND_PARAMS) {
    const i32 id = readers.reader.Read<i32>();
    const u32 code =
        readers.reader.Read<u32>(); // TODO: should this be TransactCode?

    writers.copy_handles_writer.Write(
        OS::GetInstance().GetDisplayBinderManager().GetEventHandle().id);
}

} // namespace Hydra::Horizon::Services::HosBinder

ENABLE_ENUM_FORMATTING(Hydra::Horizon::Services::HosBinder::TransactCode,
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
