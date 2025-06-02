#include "core/hw/tegra_x1/gpu/renderer/shader_decompiler/const.hpp"

namespace hydra::hw::tegra_x1::gpu::renderer::shader_decomp {

DataType to_data_type(engines::VertexAttribType vertex_attrib_type) {
    switch (vertex_attrib_type) {
    case engines::VertexAttribType::Snorm:
        return DataType::F32;
    case engines::VertexAttribType::Unorm:
        return DataType::F32;
    case engines::VertexAttribType::Sint:
        return DataType::I32;
    case engines::VertexAttribType::Uint:
        return DataType::U32;
    case engines::VertexAttribType::Sscaled:
        return DataType::U32; // TODO: use float if the Rendered backend
                              // supports scaled attributes
    case engines::VertexAttribType::Uscaled:
        return DataType::U32; // TODO: use float if the Rendered backend
                              // supports scaled attributes
    case engines::VertexAttribType::Float:
        return DataType::F32;
    default:
        LOG_WARN(ShaderDecompiler, "Unknown vertex attrib type {}",
                 vertex_attrib_type);
        return DataType::Invalid;
    }
}

const SvAccess get_sv_access_from_addr(u64 addr) {
    ASSERT_ALIGNMENT_DEBUG(addr, 4, ShaderDecompiler, "Address");

    struct SvBase {
        SvSemantic semantic;
        u64 base_addr;
    };

    static constexpr SvBase bases[] = {
        {SvSemantic::UserInOut, 0x80},
        {SvSemantic::Position, 0x70},
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

u32 get_load_store_count(LoadStoreMode mode) {
    switch (mode) {
    case LoadStoreMode::B32:
        return 1;
    case LoadStoreMode::B64:
        return 2;
    case LoadStoreMode::B96:
        return 3;
    case LoadStoreMode::B128:
        return 4;
    default:
        LOG_ERROR(ShaderDecompiler, "Unknown load store mode {}", mode);
        return 0;
    }
}

} // namespace hydra::hw::tegra_x1::gpu::renderer::shader_decomp
