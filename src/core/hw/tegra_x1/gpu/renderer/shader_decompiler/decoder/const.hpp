#pragma once

#include "core/hw/tegra_x1/gpu/renderer/shader_decompiler/ir/builder.hpp"

// TODO: ignore on release
#define COMMENT_IMPL(log_level, f_comment, f_log, ...)                         \
    {                                                                          \
        /* TODO: comments */                                                   \
        /*BUILDER.OpDebugComment(fmt::format(f_comment                         \
         * PASS_VA_ARGS(__VA_ARGS__)));*/                                      \
        LOG_##log_level(ShaderDecompiler, f_log PASS_VA_ARGS(__VA_ARGS__));    \
    }
#define COMMENT(f, ...) COMMENT_IMPL(DEBUG, f, f, __VA_ARGS__)
#define COMMENT_NOT_IMPLEMENTED(f, ...)                                        \
    COMMENT_IMPL(NOT_IMPLEMENTED, f " (NOT IMPLEMENTED)", f, __VA_ARGS__)

namespace hydra::hw::tegra_x1::gpu::renderer::shader_decomp::decoder {

struct DecoderContext {
    const DecompilerContext& decomp_context;
    Reader code_reader;
    ir::Builder& builder;
};

enum class RoundMode {
    Rn = 0,
    Rm = 1,
    Rp = 2,
    Rz = 3,
};

inline ir::Value NegIf(ir::Builder& builder, const ir::Value& value, bool neg) {
    return neg ? builder.OpNeg(value) : value;
}

inline ir::Value AbsIf(ir::Builder& builder, const ir::Value& value, bool abs) {
    return abs ? builder.OpAbs(value) : value;
}

inline ir::Value AbsNegIf(ir::Builder& builder, const ir::Value& value,
                          bool abs, bool neg) {
    return NegIf(builder, AbsIf(builder, value, abs), neg);
}

inline ir::Value NotIf(ir::Builder& builder, const ir::Value& value,
                       bool not_) {
    return not_ ? builder.OpNot(value) : value;
}

inline ir::Value SaturateIf(ir::Builder& builder, const ir::Value& value,
                            bool sat) {
    return sat ? builder.OpClamp(value,
                                 ir::Value::Immediate(std::bit_cast<u32>(0.0f),
                                                      DataType::F32),
                                 ir::Value::Immediate(std::bit_cast<u32>(1.0f),
                                                      DataType::F32))
               : value;
}

inline bool HandlePredCond(ir::Builder& builder, pred_t pred, bool pred_inv) {
    if (pred == PT) {
        if (!pred_inv) { // Always
        } else {         // Never
            // TODO: implement
            LOG_FATAL(ShaderDecompiler, "Never condition encountered");
        }

        return false;
    } else { // Conditional
        builder.OpBeginIf(
            {NotIf(builder, ir::Value::Predicate(pred), pred_inv)});
        return true;
    }
}

} // namespace hydra::hw::tegra_x1::gpu::renderer::shader_decomp::decoder
