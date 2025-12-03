#include "core/horizon/display/layer.hpp"

#include "core/horizon/kernel/process.hpp"
#include "core/horizon/os.hpp"
#include "core/hw/tegra_x1/gpu/gpu.hpp"
#include "core/hw/tegra_x1/gpu/renderer/texture_base.hpp"

namespace hydra::horizon::display {

bool Layer::AcquirePresentTexture() {
    // Get the buffer to present
    auto& binder = OS_INSTANCE.GetDisplayDriver().GetBinder(binder_id);

    BqBufferInput input;
    i32 slot = binder.ConsumeBuffer(input);
    if (slot == -1)
        return false;
    const auto& buffer = binder.GetBuffer(slot);

    // Texture
    present_texture =
        GPU_INSTANCE.GetTexture(process->GetMmu(), buffer.nv_buffer);

    // Rect
    src_rect = {};
    src_rect.origin.x() = input.rect.left;
    src_rect.origin.y() =
        input.rect.top; // Convert from top left to bottom left origin
    src_rect.size.x() = input.rect.right - input.rect.left;
    src_rect.size.y() = input.rect.bottom - input.rect.top;

    // HACK
    if (src_rect.size.x() == 0) {
        src_rect.size.x() =
            static_cast<i32>(present_texture->GetDescriptor().width);
        ONCE(LOG_WARN(Other, "Invalid src width"));
    }
    if (src_rect.size.y() == 0) {
        src_rect.size.y() =
            static_cast<i32>(present_texture->GetDescriptor().height);
        ONCE(LOG_WARN(Other, "Invalid src height"));
    }

    if (any(input.transform_flags & horizon::display::TransformFlags::FlipH)) {
        src_rect.origin.x() += src_rect.size.x();
        src_rect.size.x() = -src_rect.size.x();
    }
    if (any(input.transform_flags & horizon::display::TransformFlags::FlipV)) {
        src_rect.origin.y() += src_rect.size.y();
        src_rect.size.y() = -src_rect.size.y();
    }
    if (any(input.transform_flags & horizon::display::TransformFlags::Rot90)) {
        // TODO: how does this work? Is the aspect ratio kept intact?
        ONCE(LOG_NOT_IMPLEMENTED(Other, "Rotating by 90 degrees"));
    }

    return true;
}

void Layer::Present(FloatRect2D dst_rect, f32 dst_scale, bool transparent) {
    if (!present_texture)
        return;

    // Size
    if (size != LAYER_SIZE_AUTO)
        dst_rect.size = float2(size) * dst_scale;

    // Draw
    RENDERER_INSTANCE.DrawTextureToSurface(present_texture, src_rect, dst_rect,
                                           transparent);
}

AccumulatedTime Layer::GetAccumulatedDT() {
    return OS_INSTANCE.GetDisplayDriver()
        .GetBinder(binder_id)
        .GetAccumulatedDT();
}

} // namespace hydra::horizon::display
