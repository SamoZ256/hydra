#include "core/hw/tegra_x1/gpu/gpu.hpp"

#include "core/hw/tegra_x1/cpu/mmu.hpp"
#include "core/hw/tegra_x1/gpu/const.hpp"
#include "core/hw/tegra_x1/gpu/renderer/metal/renderer.hpp"

namespace hydra::hw::tegra_x1::gpu {

namespace {

renderer::IRenderer* CreateRenderer() {
    const auto renderer_type = CONFIG_INSTANCE.GetGpuRenderer();
    switch (renderer_type) {
    case GpuRenderer::Metal:
        return new renderer::metal::Renderer();
    default:
        LOG_FATAL(Gpu, "Unknown Gpu renderer {}", renderer_type);
    }
}

struct SetObjectArg {
    u32 class_id : 16;
    u32 engine_id : 5;
};

} // namespace

Gpu::Gpu()
    : pfifo(*this), three_d_engine(*this), compute_engine(*this),
      inline_engine(*this), two_d_engine(*this),
      copy_engine(*this), renderer{CreateRenderer()} {}

void Gpu::SubchannelMethod(u32 subchannel, u32 method, u32 arg) {
    if (method == 0x0) { // SetEngine
        ASSERT_DEBUG(subchannel <= SUBCHANNEL_COUNT, Gpu,
                     "Invalid subchannel {}", subchannel);

        const auto set_object_arg = std::bit_cast<SetObjectArg>(arg);
        // TODO: what is engine ID?
        engines::EngineBase* engine = nullptr;
        switch (set_object_arg.class_id) {
        case 0xb197:
            engine = &three_d_engine;
            break;
        case 0xb1c0:
            engine = &compute_engine;
            break;
        case 0xa140:
            engine = &inline_engine;
            break;
        case 0x902d:
            engine = &two_d_engine;
            break;
        case 0xb0b5:
            engine = &copy_engine;
            break;
        case 0xb06f:
            // TODO: implement
            LOG_NOT_IMPLEMENTED(Gpu, "GPFIFO engine");
            break;
        default:
            LOG_ERROR(Gpu, "Unknown engine class ID 0x{:08x}",
                      set_object_arg.class_id);
            break;
        }

        subchannels[subchannel] = engine;

        return;
    }

    GetEngineAtSubchannel(subchannel)->Method(method, arg);
}

renderer::ITextureView*
Gpu::GetTexture(renderer::ICommandBuffer* command_buffer, cpu::IMmu* mmu,
                const NvGraphicsBuffer& buff) {
    std::lock_guard texture_cache_lock(renderer->GetTextureCache().GetMutex());

    const auto& plane = buff.planes[0];

    LOG_DEBUG(Gpu,
              "Map id: {}, width: {}, "
              "height: {}",
              buff.nvmap_id, plane.width, plane.height);

    const bool is_linear =
        (plane.kind == NvKind::Pitch || plane.kind == NvKind::PitchNoSwizzle);

    // TODO: why are there more planes?
    const auto descriptor = renderer::TextureDescriptor::CreateWithLayerSize(
        mmu->UnmapAddr(GetMap(static_cast<u32>(buff.nvmap_id)).addr +
                       plane.offset),
        renderer::TextureType::_2D,
        renderer::to_texture_format(plane.color_format), is_linear, plane.pitch,
        plane.width, plane.height, 1, 1, 0x0, plane.block_height_gobs_log2, 0x0,
        static_cast<u32>(plane.size));

    return renderer->GetTextureCache().Find(command_buffer, descriptor,
                                            renderer::TextureUsage::Present);
}

} // namespace hydra::hw::tegra_x1::gpu
