#include "core/hw/tegra_x1/gpu/gpu.hpp"

#include "core/hw/tegra_x1/cpu/mmu.hpp"
#include "core/hw/tegra_x1/gpu/const.hpp"
#include "core/hw/tegra_x1/gpu/renderer/metal/renderer.hpp"

namespace hydra::hw::tegra_x1::gpu {

namespace {

struct SetObjectArg {
    u32 class_id : 16;
    u32 engine_id : 5;
};

} // namespace

SINGLETON_DEFINE_GET_INSTANCE(Gpu, Gpu)

Gpu::Gpu() {
    SINGLETON_SET_INSTANCE(Gpu, Gpu);

    const auto renderer_type = CONFIG_INSTANCE.GetGpuRenderer();
    switch (renderer_type) {
    case GpuRenderer::Metal:
        renderer = new renderer::metal::Renderer();
        break;
    default:
        LOG_FATAL(Gpu, "Unknown Gpu renderer {}", renderer_type);
        break;
    }
}

Gpu::~Gpu() {
    delete renderer;

    SINGLETON_UNSET_INSTANCE();
}

void Gpu::SubchannelMethod(GMmu& gmmu, u32 subchannel, u32 method, u32 arg) {
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

    GetEngineAtSubchannel(subchannel)->Method(gmmu, method, arg);
}

renderer::TextureBase* Gpu::GetTexture(cpu::IMmu* mmu,
                                       const NvGraphicsBuffer& buff) {
    LOG_DEBUG(Gpu,
              "Map id: {}, width: {}, "
              "height: {}",
              buff.nvmap_id, buff.planes[0].width, buff.planes[0].height);

    // TODO: why are there more planes?
    renderer::TextureDescriptor descriptor(
        mmu->UnmapAddr(GetMap(static_cast<u32>(buff.nvmap_id)).addr +
                       buff.planes[0].offset),
        renderer::TextureType::_2D,
        renderer::to_texture_format(buff.planes[0].color_format),
        buff.planes[0].kind, buff.planes[0].width, buff.planes[0].height, 1,
        buff.planes[0].block_height_log2, buff.planes[0].pitch);

    return renderer->GetTextureCache().Find(descriptor,
                                            renderer::TextureUsage::Present);
}

} // namespace hydra::hw::tegra_x1::gpu
