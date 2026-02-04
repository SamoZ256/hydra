#include "core/hw/tegra_x1/gpu/renderer/shader_decompiler/ir/type.hpp"

namespace hydra::hw::tegra_x1::gpu::renderer::shader_decomp::ir {

ScalarType ScalarSignedEquivalent(ScalarType scalar) {
    ASSERT_THROWING_DEBUG(ScalarIsUnsignedInteger(scalar), ShaderDecompiler,
                          TypeError::NotUnsigned,
                          "Type {} is not an unsigned integer", scalar);
    switch (scalar) {
    case ScalarType::U8:
        return ScalarType::I8;
    case ScalarType::U16:
        return ScalarType::I16;
    case ScalarType::U32:
        return ScalarType::I32;
    default:
        unreachable();
    }
}

ScalarType ScalarUnsignedEquivalent(ScalarType scalar) {
    ASSERT_THROWING_DEBUG(ScalarIsSignedInteger(scalar), ShaderDecompiler,
                          TypeError::NotSigned,
                          "Type {} is not a signed integer", scalar);
    switch (scalar) {
    case ScalarType::I8:
        return ScalarType::U8;
    case ScalarType::I16:
        return ScalarType::U16;
    case ScalarType::I32:
        return ScalarType::U32;
    default:
        unreachable();
    }
}

ScalarType Type::GetScalarType() const {
    ASSERT_THROWING_DEBUG(IsScalar(), ShaderDecompiler, TypeError::NotAScalar,
                          "Type {} is not a scalar", *this);
    return scalar;
}

VectorType Type::GetVectorType() const {
    ASSERT_THROWING_DEBUG(IsVector(), ShaderDecompiler, TypeError::NotAVector,
                          "Type {} is not a vector", *this);
    return vector;
}

// Type creation
Type Type::SignedEquivalent() const {
    ASSERT_THROWING_DEBUG(IsInteger(), ShaderDecompiler,
                          TypeError::NotAnInteger, "Type {} is not an integer",
                          *this);
    switch (kind) {
    case TypeKind::Scalar:
        return ScalarSignedEquivalent(scalar);
    case TypeKind::Vector:
        return vector.SignedEquivalent();
    default:
        unreachable();
    }
}

Type Type::UnsignedEquivalent() const {
    ASSERT_THROWING_DEBUG(IsInteger(), ShaderDecompiler,
                          TypeError::NotAnInteger, "Type {} is not an integer",
                          *this);
    switch (kind) {
    case TypeKind::Scalar:
        return ScalarUnsignedEquivalent(scalar);
    case TypeKind::Vector:
        return vector.UnsignedEquivalent();
    default:
        unreachable();
    }
}

} // namespace hydra::hw::tegra_x1::gpu::renderer::shader_decomp::ir
