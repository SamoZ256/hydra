#include "core/hw/tegra_x1/gpu/renderer/shader_decompiler/const.hpp"

namespace hydra::hw::tegra_x1::gpu::renderer::shader_decomp {

SvAccess get_sv_access_from_addr(u64 addr) {
    ASSERT_ALIGNMENT_DEBUG(addr, 4, ShaderDecompiler, "Address");

    struct SvBase {
        SvSemantic semantic;
        u64 base_addr;
    };

    static constexpr SvBase bases[] = {
        {.semantic=SvSemantic::VertexID, .base_addr=SV_VERTEX_ID_BASE},
        {.semantic=SvSemantic::InstanceID, .base_addr=SV_INSTANCE_ID_BASE},
        {.semantic=SvSemantic::UserInOut, .base_addr=SV_USER_IN_OUT_BASE},
        {.semantic=SvSemantic::Position, .base_addr=SV_POSITION_BASE},
    };

    for (const auto& base : bases) {
        if (addr >= base.base_addr) {
            return SvAccess(Sv(base.semantic,
                               static_cast<u8>((addr - base.base_addr) >> 4)),
                            static_cast<u8>((addr >> 2) & 0x3));
        }
    }

    LOG_NOT_IMPLEMENTED(ShaderDecompiler, "SV address 0x{:02x}", addr);

    return SvAccess(Sv(SvSemantic::Invalid), invalid<u8>());
}

} // namespace hydra::hw::tegra_x1::gpu::renderer::shader_decomp
