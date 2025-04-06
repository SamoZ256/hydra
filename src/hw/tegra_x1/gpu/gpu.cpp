#include "hw/tegra_x1/gpu/gpu.hpp"

#include "hw/tegra_x1/cpu/mmu_base.hpp"
#include "hw/tegra_x1/gpu/const.hpp"
#include "hw/tegra_x1/gpu/engines/2d.hpp"
#include "hw/tegra_x1/gpu/engines/3d.hpp"
#include "hw/tegra_x1/gpu/engines/compute.hpp"
#include "hw/tegra_x1/gpu/engines/copy.hpp"
#include "hw/tegra_x1/gpu/engines/inline.hpp"
#include "hw/tegra_x1/gpu/renderer/metal/renderer.hpp"

namespace Hydra::HW::TegraX1::GPU {

namespace {

struct SetObjectArg {
    u32 class_id : 16;
    u32 engine_id : 5;
};

} // namespace

SINGLETON_DEFINE_GET_INSTANCE(GPU, GPU, "GPU")

GPU::GPU(CPU::MMUBase* mmu_) : mmu{mmu_}, gpu_mmu(mmu), pfifo(gpu_mmu) {
    SINGLETON_SET_INSTANCE(GPU, "GPU");

    // TODO: choose based on the Renderer backend
    {
        renderer = new Renderer::Metal::Renderer();
    }
}

GPU::~GPU() {
    for (u32 i = 0; i < SUBCHANNEL_COUNT; i++) {
        if (subchannels[i])
            delete subchannels[i];
    }

    delete renderer;

    SINGLETON_UNSET_INSTANCE();
}

void GPU::SubchannelMethod(u32 subchannel, u32 method, u32 arg) {
    if (method == 0x0) { // SetEngine
        ASSERT_DEBUG(subchannel <= SUBCHANNEL_COUNT, GPU,
                     "Invalid subchannel {}", subchannel);

        const auto set_object_arg = bit_cast<SetObjectArg>(arg);
        // TODO: what is engine ID?
        Engines::EngineBase* engine = nullptr;
        switch (set_object_arg.class_id) {
        case 0xb197:
            engine = new Engines::ThreeD();
            break;
        case 0xb1c0:
            engine = new Engines::Compute();
            break;
        case 0xa140:
            engine = new Engines::Inline();
            break;
        case 0x902d:
            engine = new Engines::TwoD();
            break;
        case 0xb0b5:
            engine = new Engines::Copy();
            break;
        case 0xb06f:
            // TODO: implement
            LOG_NOT_IMPLEMENTED(GPU, "GPFIFO engine");
            break;
        default:
            LOG_ERROR(GPU, "Unknown engine class ID 0x{:08x}",
                      set_object_arg.class_id);
            break;
        }

        subchannels[subchannel] = engine;

        return;
    }

    GetEngineAtSubchannel(subchannel)->Method(method, arg);
}

Renderer::TextureBase* GPU::GetTexture(const NvGraphicsBuffer& buff) {
    LOG_DEBUG(GPU,
              "Map id: {}, width: {}, "
              "height: {}",
              buff.nvmap_id, buff.planes[0].width, buff.planes[0].height);

    // TODO: why are there more planes?
    Renderer::TextureDescriptor descriptor{
        .ptr = mmu->UnmapAddr(GetMapById(buff.nvmap_id).addr +
                              buff.planes[0].offset),
        .format = Renderer::to_texture_format(buff.planes[0].color_format),
        .kind = buff.planes[0].kind,
        .width = buff.planes[0].width,
        .height = buff.planes[0].height,
        //.stride = buff.stride,
        .block_height_log2 = buff.planes[0].block_height_log2,
        .stride = buff.planes[0].pitch,
    };

    return renderer->GetTextureCache().GetTextureView(descriptor);
}

} // namespace Hydra::HW::TegraX1::GPU
