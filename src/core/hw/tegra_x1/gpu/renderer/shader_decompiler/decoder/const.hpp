#pragma once

#include "core/hw/tegra_x1/gpu/renderer/shader_decompiler/ir/builder.hpp"

// TODO: ignore on release
#define COMMENT_IMPL(log_level, f_comment, f_log, ...)                         \
    {                                                                          \
        /* TODO: comments */                                                   \
        /*BUILDER.OpDebugComment(fmt::format(f_comment                         \
         * ZTD_PASS_VA_ARGS(__VA_ARGS__)));*/                                  \
        LOG_##log_level(ShaderDecompiler,                                      \
                        f_log ZTD_PASS_VA_ARGS(__VA_ARGS__));                  \
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

enum class FloatCmpOp {
    F = 0,
    Lt = 1,
    Eq = 2,
    Le = 3,
    Gt = 4,
    Ne = 5,
    Ge = 6,
    Num = 7,
    Nan = 8,
    Ltu = 9,
    Equ = 10,
    Leu = 11,
    Gtu = 12,
    Neu = 13,
    Geu = 14,
    T = 15,
};

enum class HalfSwizzle {
    F16 = 0,
    F32 = 1,
    H0H0 = 2,
    H1H1 = 3,
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

inline ir::Value GetSwizzledHalf(ir::Builder& builder, HalfSwizzle swizzle,
                                 reg_t src) {
    if (swizzle == HalfSwizzle::F16)
        return ir::Value::Register(src, ir::VectorType(ir::ScalarType::F16, 2));

    ir::Value src_v = ir::Value::Undefined();
    switch (swizzle) {
    case HalfSwizzle::F32: {
        src_v = builder.OpCast(ir::Value::Register(src, ir::ScalarType::F32),
                               ir::ScalarType::F16);
        break;
    }
    case HalfSwizzle::H0H0: {
        src_v = ir::Value::Register(src, ir::ScalarType::F16);
        break;
    }
    case HalfSwizzle::H1H1: {
        src_v = builder.OpBitfieldExtract(ir::Value::Register(src),
                                          ir::Value::ConstantU(16),
                                          ir::Value::ConstantU(16));
        src_v = builder.OpCast(src_v, ir::ScalarType::U16);
        src_v = builder.OpBitCast(src_v, ir::ScalarType::F16);
        break;
    }
    default:
        unreachable();
    }

    return builder.OpVectorConstruct(ir::ScalarType::F16, {src_v, src_v});
}

inline ir::Value GetFloatCmp(DecoderContext& context, FloatCmpOp op,
                             ir::Value a, ir::Value b) {
    // TODO: handle U versions differently?
    switch (op) {
    case FloatCmpOp::F:
        return ir::Value::ConstantB(false);
    case FloatCmpOp::T:
        return ir::Value::ConstantB(true);
    case FloatCmpOp::Lt:
    case FloatCmpOp::Ltu:
        return context.builder.OpCompareLess(a, b);
    case FloatCmpOp::Le:
    case FloatCmpOp::Leu:
        return context.builder.OpCompareLessOrEqual(a, b);
    case FloatCmpOp::Gt:
    case FloatCmpOp::Gtu:
        return context.builder.OpCompareGreater(a, b);
    case FloatCmpOp::Ge:
    case FloatCmpOp::Geu:
        return context.builder.OpCompareGreaterOrEqual(a, b);
    case FloatCmpOp::Eq:
    case FloatCmpOp::Equ:
        return context.builder.OpCompareEqual(a, b);
    case FloatCmpOp::Ne:
    case FloatCmpOp::Neu:
        return context.builder.OpCompareNotEqual(a, b);
    case FloatCmpOp::Num:
    case FloatCmpOp::Nan: {
        const auto res = context.builder.OpBitwiseOr(a, b);
        if (op == FloatCmpOp::Num)
            return context.builder.OpNot(res);
        else
            return res;
    }
    }
}

} // namespace hydra::hw::tegra_x1::gpu::renderer::shader_decomp::decoder
