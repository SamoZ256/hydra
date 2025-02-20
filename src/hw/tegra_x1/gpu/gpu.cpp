#include "hw/tegra_x1/gpu/gpu.hpp"

#include "hw/tegra_x1/cpu/mmu_base.hpp"
#include "hw/tegra_x1/gpu/const.hpp"
#include "hw/tegra_x1/gpu/renderer/metal/renderer.hpp"

namespace Hydra::HW::TegraX1::GPU {

static GPU* s_instance = nullptr;

GPU& GPU::GetInstance() { return *s_instance; }

GPU::GPU(CPU::MMUBase* mmu_) : mmu{mmu_} {
    ASSERT(s_instance == nullptr, GPU, "GPU already exists");
    s_instance = this;

    // TODO: choose based on the Renderer backend
    {
        renderer = new Renderer::Metal::Renderer();
    }
}

GPU::~GPU() {
    delete renderer;

    s_instance = nullptr;
}

TextureDescriptor GPU::CreateTextureDescriptor(const NvGraphicsBuffer& buff) {
    TextureDescriptor descriptor;
    descriptor.ptr =
        mmu->UnmapAddr(GetMapById(buff.nvmap_id).addr + buff.planes[0].offset);
    // TODO: why are there more planes?
    descriptor.width = buff.planes[0].width;
    descriptor.height = buff.planes[0].height;

    return descriptor;
}

} // namespace Hydra::HW::TegraX1::GPU
