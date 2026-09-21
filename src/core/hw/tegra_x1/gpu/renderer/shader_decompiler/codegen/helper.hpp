#pragma once

#include "core/hw/tegra_x1/gpu/renderer/shader_decompiler/ir/value.hpp"

namespace hydra::hw::tegra_x1::gpu::renderer::shader_decomp::codegen {

inline ir::ScalarType toType(engines::VertexAttribType vertex_attrib_type) {
    switch (vertex_attrib_type) {
    case engines::VertexAttribType::Snorm:
        return ir::ScalarType::F32;
    case engines::VertexAttribType::Unorm:
        return ir::ScalarType::F32;
    case engines::VertexAttribType::Sint:
        return ir::ScalarType::I32;
    case engines::VertexAttribType::Uint:
        return ir::ScalarType::U32;
    case engines::VertexAttribType::Sscaled:
        return ir::ScalarType::U32; // TODO: use float if the Renderer backend
                                    // supports scaled attributes
    case engines::VertexAttribType::Uscaled:
        return ir::ScalarType::U32; // TODO: use float if the Renderer backend
                                    // supports scaled attributes
    case engines::VertexAttribType::Float:
        return ir::ScalarType::F32;
    default:
        unreachable();
    }
}

inline ir::ScalarType toType(ColorDataType color_data_type) {
    switch (color_data_type) {
    case ColorDataType::Float:
        return ir::ScalarType::F32;
    case ColorDataType::Int:
        return ir::ScalarType::I32;
    case ColorDataType::UInt:
        return ir::ScalarType::U32;
    default:
        unreachable();
    }
}

} // namespace hydra::hw::tegra_x1::gpu::renderer::shader_decomp::codegen
