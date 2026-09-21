#pragma once

#include "core/hw/tegra_x1/gpu/renderer/shader_decompiler/decoder/const.hpp"

namespace hydra::hw::tegra_x1::gpu::renderer::shader_decomp::decoder {

enum class BoolOp {
    And = 0,
    Or = 1,
    Xor = 2,
};

inline ir::Value getLogical(DecoderContext& context, BoolOp op, ir::Value a,
                            ir::Value b) {
    switch (op) {
    case BoolOp::And:
        return context.builder.opBitwiseAnd(a, b);
    case BoolOp::Or:
        return context.builder.opBitwiseOr(a, b);
    case BoolOp::Xor:
        return context.builder.opBitwiseXor(a, b);
    }
}

} // namespace hydra::hw::tegra_x1::gpu::renderer::shader_decomp::decoder
