#include "hw/tegra_x1/gpu/engines/inline_base.hpp"

#include "hw/tegra_x1/gpu/gpu.hpp"
#include "hw/tegra_x1/gpu/renderer/buffer_base.hpp"

namespace Hydra::HW::TegraX1::GPU::Engines {

void InlineBase::LaunchDMAImpl(RegsInline& regs, const u32 index,
                               const u32 data) {
    // TODO: args
    LOG_FUNC_STUBBED(Engines);
}

void InlineBase::LoadInlineDataImpl(RegsInline& regs, const u32 index,
                                    const u32 data) {
    inline_data.push_back(data);
    // TODO: correct?
    if (inline_data.size() * sizeof(u32) ==
        regs.line_length_in * regs.line_count) {
        // Flush
        // TODO: determine what type of copy this is based on launch DMA args

        // Buffer to buffer
        uptr dst_ptr = UNMAP_ADDR(regs.offset_out);
        // TODO: do a GPU copy instead?
        memcpy(reinterpret_cast<void*>(dst_ptr), inline_data.data(),
               inline_data.size() * sizeof(u32));
        /*
        auto dst = RENDERER->GetBufferCache().Find(
            {dst_ptr, inline_data.size() * sizeof(u32)});

        dst->CopyFrom(inline_data.data());
        */
        inline_data.clear();
    }
}

} // namespace Hydra::HW::TegraX1::GPU::Engines
