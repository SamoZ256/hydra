#include "core/horizon/services/hosbinder/hos_binder_driver.hpp"

#include "core/horizon/kernel/process.hpp"
#include "core/horizon/services/hosbinder/parcel.hpp"
#include "core/hw/tegra_x1/gpu/const.hpp"
#include "core/system.hpp"

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

} // namespace

DEFINE_SERVICE_COMMAND_TABLE(IHOSBinderDriver, 0, TransactParcel, 1,
                             AdjustRefcount, 2, GetNativeHandle, 3,
                             TransactParcelAuto)

result_t IHOSBinderDriver::TransactParcel(
    System* system, i32 binder_id, TransactCode code, u32 flags,
    InBuffer<BufferAttr::MapAlias> in_parcel_buffer,
    OutBuffer<BufferAttr::MapAlias> out_parcel_buffer) {
    LOG_DEBUG(Services, "Code: {}", code);

    TransactParcelImpl(*system, binder_id, code, flags, in_parcel_buffer.stream,
                       out_parcel_buffer.stream);
    return RESULT_SUCCESS;
}

result_t IHOSBinderDriver::AdjustRefcount(System* system, i32 binder_id,
                                          i32 add_value, BinderType type) {
    auto& binder = system->GetOS().GetDisplayDriver().GetBinder(
        static_cast<u32>(binder_id));
    switch (type) {
    case BinderType::Weak:
        binder.weak_ref_count = static_cast<u32>(
            static_cast<i32>(binder.weak_ref_count) + add_value);
        break;
    case BinderType::Strong:
        binder.strong_ref_count = static_cast<u32>(
            static_cast<i32>(binder.strong_ref_count) + add_value);
        break;
    }

    return RESULT_SUCCESS;
}

// TODO: code
result_t
IHOSBinderDriver::GetNativeHandle(System* system, kernel::Process* process,
                                  i32 binder_id, u32 code,
                                  OutHandle<HandleAttr::Copy> out_handle) {
    (void)code;

    out_handle = process->AddHandle(system->GetOS()
                                        .GetDisplayDriver()
                                        .GetBinder(static_cast<u32>(binder_id))
                                        .GetEvent());
    return RESULT_SUCCESS;
}

result_t IHOSBinderDriver::TransactParcelAuto(
    System* system, i32 binder_id, TransactCode code, u32 flags,
    InBuffer<BufferAttr::AutoSelect> in_parcel_buffer,
    OutBuffer<BufferAttr::AutoSelect> out_parcel_buffer) {
    LOG_DEBUG(Services, "Code: {}", code);

    TransactParcelImpl(*system, binder_id, code, flags, in_parcel_buffer.stream,
                       out_parcel_buffer.stream);
    return RESULT_SUCCESS;
}

// TODO: flags
void IHOSBinderDriver::TransactParcelImpl(System& system, i32 binder_id,
                                          TransactCode code, u32 flags,
                                          io::MemoryStream* in_stream,
                                          io::MemoryStream* out_stream) {
    (void)flags;

    ParcelReader parcel_reader(in_stream);
    ParcelWriter parcel_writer(out_stream);

    // Binder
    auto& binder = system.GetOS().GetDisplayDriver().GetBinder(
        static_cast<u32>(binder_id));

    // Interface token
    [[maybe_unused]] const auto interface_token =
        parcel_reader.ReadInterfaceToken();
    LOG_DEBUG(Services, "Interface token: {}", interface_token);

    // Dispatch
    BinderResult b_result = BinderResult::Success;
    switch (code) {
    case TransactCode::RequestBuffer: {
        i32 slot = parcel_reader.Read<i32>();
        if (slot > static_cast<i32>(display::MAX_BINDER_BUFFER_COUNT)) {
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
        display::NvMultiFence fence = {
            .num_fences = 0,
        };
        parcel_writer.WriteStrongPointer(&fence);

        break;
    }
    case TransactCode::QueueBuffer: {
        // Slot
        i32 slot = parcel_reader.Read<i32>();
        const auto& input =
            *parcel_reader.ReadFlattenedObject<display::BqBufferInput>();

        binder.QueueBuffer(system, slot, input);

        // Buffer output
        const auto res = system.GetOS().GetDisplayResolution();
        parcel_writer.Write<display::BqBufferOutput>({
            .width = res.x(),
            .height = res.y(),
            .transform_hint = 0,                                     // HACK
            .num_pending_buffers = display::MAX_BINDER_BUFFER_COUNT, // HACK
        });

        break;
    }
    case TransactCode::Query: {
        const auto what = parcel_reader.Read<NativeWindowAttribute>();
        LOG_DEBUG(Services, "what: {}", what);

        u32 value = 0;
        switch (what) {
        case NativeWindowAttribute::Width:
            value = system.GetOS().GetDisplayResolution().x();
            break;
        case NativeWindowAttribute::Height:
            value = system.GetOS().GetDisplayResolution().y();
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
        const auto res = system.GetOS().GetDisplayResolution();
        parcel_writer.Write<display::BqBufferOutput>({
            .width = res.x(),
            .height = res.y(),
            .transform_hint = 0,                                     // HACK
            .num_pending_buffers = display::MAX_BINDER_BUFFER_COUNT, // HACK
        });

        break;
    }
    case TransactCode::Disconnect: {
        [[maybe_unused]] auto api = parcel_reader.Read<i32>(); // TODO: enum
        LOG_DEBUG(Services, "API: {}", api);

        binder.UnqueueAllBuffers();

        break;
    }
    case TransactCode::SetPreallocatedBuffer: {
        // Slot
        i32 slot = parcel_reader.Read<i32>();

        // Input buffer
        auto buffer = parcel_reader.ReadStrongPointer<display::GraphicBuffer>();
        if (!buffer) {
            LOG_ERROR(Services, "No graphic buffer");
            break;
        }

        // Debug
#ifdef HYDRA_DEBUG
        const auto& plane = buffer->nv_buffer.planes[0];
        LOG_DEBUG(Services,
                  "width: {}, height: {}, color_format: {}, "
                  "layout: {}, pitch: 0x{:08x}, "
                  "unused: 0x{:08x}, offset: 0x{:08x}, kind: "
                  "{}, size: 0x{:08x}",
                  plane.width, plane.height, plane.color_format, plane.layout,
                  plane.pitch, plane.unused, plane.offset, plane.kind,
                  plane.size);
#endif

        binder.AddBuffer(slot, *buffer);

        break;
    }
    default:
        LOG_WARN(Services, "Unknown code {}", code);
        break;
    }

    parcel_writer.Write(b_result);
    parcel_writer.Finish();
}

} // namespace hydra::horizon::services::hosbinder
