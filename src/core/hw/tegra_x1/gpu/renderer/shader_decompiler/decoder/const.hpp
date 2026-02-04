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

#define DEFINE_INST_VARIANTS_IMPL(inst_name, reg_name)                         \
    union Inst##inst_name##R {                                                 \
        Inst##inst_name##Base base;                                            \
        BitField64<reg_t, 20, 8> reg_name;                                     \
    };                                                                         \
    void Emit##inst_name##R(DecoderContext& context, Inst##inst_name##R inst); \
    union Inst##inst_name##C {                                                 \
        Inst##inst_name##Base base;                                            \
        BitField64<u32, 20, 14> cbuf_offset;                                   \
        BitField64<u32, 34, 5> cbuf_slot;                                      \
    };                                                                         \
    void Emit##inst_name##C(DecoderContext& context, Inst##inst_name##C inst); \
    union Inst##inst_name##I {                                                 \
        Inst##inst_name##Base base;                                            \
        BitField64<u32, 20, 19> imm20_0;                                       \
        BitField64<u32, 56, 1> imm20_19;                                       \
    };                                                                         \
    void Emit##inst_name##I(DecoderContext& context, Inst##inst_name##I inst);

#define DEFINE_INST_SRC1_VARIANTS(inst_name)                                   \
    DEFINE_INST_VARIANTS_IMPL(inst_name, src)
#define DEFINE_INST_SRC2_VARIANTS(inst_name)                                   \
    DEFINE_INST_VARIANTS_IMPL(inst_name, src_b)

namespace hydra::hw::tegra_x1::gpu::renderer::shader_decomp::decoder {

struct DecoderContext {
    const DecompilerContext& decomp_context;
    io::IStream* code_stream;
    ir::Builder& builder;
};

enum class RoundMode {
    Rn = 0,
    Rm = 1,
    Rp = 2,
    Rz = 3,
};

inline u32 GetIntImm20(u32 imm20_0, u32 imm20_19, bool extend) {
    const auto imm20 = imm20_0 | (imm20_19 << 19);
    if (extend)
        return sign_extend<u32, 20>(imm20);
    else
        return imm20;
}

inline f32 GetFloatImm20(u32 imm20_0, u32 imm20_19) {
    return std::bit_cast<f32>((imm20_0 | (imm20_19 << 19)) << 12);
}

inline ir::Value GetHalf2Const(ir::Builder& builder, u32 h0, u32 h1) {
    return builder.OpVectorConstruct(
        ir::ScalarType::F16, {ir::Value::Constant(h0, ir::ScalarType::F16),
                              ir::Value::Constant(h1, ir::ScalarType::F16)});
}

inline ir::Value GetHalf2Const20(ir::Builder& builder, u32 h0_imm10,
                                 u32 h1_imm10) {
    return GetHalf2Const(builder, h0_imm10 << 6, h1_imm10 << 6);
}

inline ir::Value GetHalf2Const32(ir::Builder& builder, u32 imm32) {
    return GetHalf2Const(builder, extract_bits(imm32, 0, 16),
                         extract_bits(imm32, 16, 16));
}

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
    return sat ? builder.OpClamp(value, ir::Value::ConstantF(0.0f),
                                 ir::Value::ConstantF(1.0f))
               : value;
}

inline ir::Value Saturate2xF16If(ir::Builder& builder, const ir::Value& value,
                                 bool sat) {
    return sat ? builder.OpClamp(value, GetHalf2Const(builder, 0x0, 0x0),
                                 GetHalf2Const(builder, 0x3c00, 0x3c00))
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

enum class EnsureIntegerSignednessError {
    NotAnInteger,
};

template <bool signed_>
ir::Value EnsureIntegerSignedness(ir::Builder& builder,
                                  const ir::Value& value) {
    const auto type = value.GetType();
    if (signed_) {
        if (type.IsUnsignedInteger())
            return builder.OpCast(value, type.SignedEquivalent());
    } else {
        if (type.IsSignedInteger())
            return builder.OpCast(value, type.UnsignedEquivalent());
    }

    return value;
}

} // namespace hydra::hw::tegra_x1::gpu::renderer::shader_decomp::decoder
