#include "core/horizon/services/hosbinder/hos_binder_driver.hpp"

#include "core/horizon/kernel/kernel.hpp"
#include "core/horizon/os.hpp"
#include "core/horizon/services/hosbinder/parcel.hpp"
#include "core/hw/tegra_x1/gpu/const.hpp"

namespace hydra::horizon::services::hosbinder {

namespace {

// From Ryujinx
enum class NativeWindowAttribute : u32 {
    Width = 0,
    Height = 1,
    Format = 2,
    MinUnqueuedBuffers = 3,
    ConsumerRunningBehind = 9,
    ConsumerUsageBits = 10,
    MaxBufferCountAsync = 12,
};

} // namespace

} // namespace hydra::horizon::services::hosbinder

ENABLE_ENUM_FORMATTING(
    hydra::horizon::services::hosbinder::NativeWindowAttribute, Width, "width",
    Height, "height", Format, "format", MinUnqueuedBuffers,
    "min unqueued buffers", ConsumerRunningBehind, "consumer running behind",
    ConsumerUsageBits, "consumer usage bits", MaxBufferCountAsync,
    "max buffer count async")

namespace hydra::horizon::services::hosbinder {

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

enum class PixelFormat : u32 {
    Unknown,
    RGBA8888,
    RGBX8888,
    RGB888,
    RGB565,
    BGRA8888,
    RGBA5551,
    RGBA4444,
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
    u32 transform_hint;
    u32 num_pending_buffers;
};

} // namespace

DEFINE_SERVICE_COMMAND_TABLE(IHOSBinderDriver, 0, TransactParcel, 1,
                             AdjustRefcount, 2, GetNativeHandle, 3,
                             TransactParcelAuto)

result_t IHOSBinderDriver::TransactParcel(
    i32 binder_id, TransactCode code, u32 flags,
    InBuffer<BufferAttr::MapAlias> in_parcel_buffer,
    OutBuffer<BufferAttr::MapAlias> out_parcel_buffer) {
    LOG_DEBUG(Services, "Code: {}", code);

    ParcelReader parcel_reader(*in_parcel_buffer.reader);
    ParcelWriter parcel_writer(*out_parcel_buffer.writer);

    TransactParcelImpl(binder_id, code, flags, parcel_reader, parcel_writer);

    parcel_writer.Finalize();
    return RESULT_SUCCESS;
}

result_t IHOSBinderDriver::AdjustRefcount(i32 binder_id, i32 add_value,
                                          BinderType type) {
    auto& binder = OS::GetInstance().GetDisplayDriver().GetBinder(binder_id);
    switch (type) {
    case BinderType::Weak:
        binder.weak_ref_count += add_value;
        break;
    case BinderType::Strong:
        binder.strong_ref_count += add_value;
        break;
    }

    return RESULT_SUCCESS;
}

result_t
IHOSBinderDriver::GetNativeHandle(i32 binder_id, u32 code,
                                  OutHandle<HandleAttr::Copy> out_handle) {
    out_handle =
        OS::GetInstance().GetDisplayDriver().GetBinder(binder_id).GetEvent().id;
    return RESULT_SUCCESS;
}

result_t IHOSBinderDriver::TransactParcelAuto(
    i32 binder_id, TransactCode code, u32 flags,
    InBuffer<BufferAttr::AutoSelect> in_parcel_buffer,
    OutBuffer<BufferAttr::AutoSelect> out_parcel_buffer) {
    LOG_DEBUG(Services, "Code: {}", code);

    ParcelReader parcel_reader(*in_parcel_buffer.reader);
    ParcelWriter parcel_writer(*out_parcel_buffer.writer);

    TransactParcelImpl(binder_id, code, flags, parcel_reader, parcel_writer);

    parcel_writer.Finalize();
    return RESULT_SUCCESS;
}

void IHOSBinderDriver::TransactParcelImpl(i32 binder_id, TransactCode code,
                                          u32 flags,
                                          ParcelReader& parcel_reader,
                                          ParcelWriter& parcel_writer) {
    // Binder
    auto& binder = OS::GetInstance().GetDisplayDriver().GetBinder(binder_id);

    // Dispatch
    BinderResult b_result = BinderResult::Success;
    switch (code) {
    case TransactCode::RequestBuffer: {
        auto interface_token = parcel_reader.ReadInterfaceToken();
        LOG_DEBUG(Services, "Interface token: {}", interface_token);

        i32 slot = parcel_reader.Read<i32>();
        if (slot > MAX_BINDER_BUFFER_COUNT) {
            LOG_WARN(Services, "Invalid slot: {}", slot);
            parcel_writer.Write<u32>(0x0);
            break;
        }

        // Buffer
        const auto& buffer = binder.GetBuffer(slot);
        parcel_writer.WriteStrongPointer(&buffer);

        break;
    }
    case TransactCode::DequeueBuffer: {
        i32 slot = binder.GetAvailableSlot();

        parcel_writer.Write(slot);

        // NvMultiFence
        NvMultiFence fence = {
            .num_fences = 0,
        };
        parcel_writer.WriteStrongPointer(&fence);

        break;
    }
    case TransactCode::QueueBuffer: {
        auto interface_token = parcel_reader.ReadInterfaceToken();
        LOG_DEBUG(Services, "Interface token: {}", interface_token);

        // Slot
        i32 slot = parcel_reader.Read<i32>();

        // TODO: input

        binder.QueueBuffer(slot);

        // Buffer output
        // TODO
        parcel_writer.Write<BqBufferOutput>({
            .width = 1280,       // TODO: don't hardcode
            .height = 720,       // TODO: don't hardcode
            .transform_hint = 0, // HACK
            .num_pending_buffers = MAX_BINDER_BUFFER_COUNT, // HACK
        });

        break;
    }
    case TransactCode::Query: {
        auto interface_token = parcel_reader.ReadInterfaceToken();
        LOG_DEBUG(Services, "Interface token: {}", interface_token);

        const auto what = parcel_reader.Read<NativeWindowAttribute>();
        LOG_DEBUG(Services, "what: {}", what);

        u32 value = 0;
        switch (what) {
        case NativeWindowAttribute::Width:
            value = 1280; // TODO: don't hardcode
            break;
        case NativeWindowAttribute::Height:
            value = 720; // TODO: don't hardcode
            break;
        case NativeWindowAttribute::Format:
            value = static_cast<u32>(PixelFormat::RGBA8888); // RGBA8888
            break;
        default:
            LOG_NOT_IMPLEMENTED(Services, "Native window attribute {}", what);
            break;
        }

        parcel_writer.Write(value);

        break;
    }
    case TransactCode::Connect: {
        auto interface_token = parcel_reader.ReadInterfaceToken();
        LOG_DEBUG(Services, "Interface token: {}", interface_token);

        parcel_writer.Write<BqBufferOutput>({
            .width = 1280,       // TODO: don't hardcode
            .height = 720,       // TODO: don't hardcode
            .transform_hint = 0, // HACK
            .num_pending_buffers = MAX_BINDER_BUFFER_COUNT, // HACK
        });

        break;
    }
    case TransactCode::Disconnect: {
        auto interface_token = parcel_reader.ReadInterfaceToken();
        auto api = parcel_reader.Read<i32>(); // TODO: enum
        LOG_DEBUG(Services, "Interface token: {}, API: {}", interface_token,
                  api);

        binder.UnqueueAllBuffers();

        break;
    }
    case TransactCode::SetPreallocatedBuffer: {
        auto interface_token = parcel_reader.ReadInterfaceToken();
        LOG_DEBUG(Services, "Interface token: {}", interface_token);

        // Slot
        i32 slot = parcel_reader.Read<i32>();

        // Input buffer
        auto buffer = parcel_reader.ReadStrongPointer<GraphicBuffer>();
        if (!buffer) {
            LOG_ERROR(Services, "No graphic buffer");
            break;
        }

        // Debug
        const auto& plane = buffer->nv_buffer.planes[0];
        LOG_DEBUG(Services,
                  "width: {}, height: {}, color_format: {}, "
                  "layout: {}, pitch: 0x{:08x}, "
                  "unused: 0x{:08x}, offset: 0x{:08x}, kind: "
                  "{}, size: 0x{:08x}",
                  plane.width, plane.height, plane.color_format, plane.layout,
                  plane.pitch, plane.unused, plane.offset, plane.kind,
                  plane.size);

        binder.AddBuffer(slot, *buffer);

        break;
    }
    default:
        LOG_WARN(Services, "Unknown code {}", code);
        break;
    }

    // result_t
    parcel_writer.Write(b_result);
}

} // namespace hydra::horizon::services::hosbinder
