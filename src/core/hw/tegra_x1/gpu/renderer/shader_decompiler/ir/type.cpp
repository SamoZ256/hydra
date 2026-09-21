#include "core/hw/tegra_x1/gpu/renderer/shader_decompiler/ir/type.hpp"

namespace hydra::hw::tegra_x1::gpu::renderer::shader_decomp::ir {

ScalarType scalarSignedEquivalent(ScalarType scalar) {
    ASSERT_DEBUG(scalarIsUnsignedInteger(scalar), ShaderDecompiler,
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

ScalarType scalarUnsignedEquivalent(ScalarType scalar) {
    ASSERT_DEBUG(scalarIsSignedInteger(scalar), ShaderDecompiler,
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

ScalarType Type::getScalarType() const {
    ASSERT_DEBUG(isScalar(), ShaderDecompiler, "Type {} is not a scalar",
                 *this);
    return scalar;
}

VectorType Type::getVectorType() const {
    ASSERT_DEBUG(isVector(), ShaderDecompiler, "Type {} is not a vector",
                 *this);
    return vector;
}

// Type creation
Type Type::signedEquivalent() const {
    ASSERT_DEBUG(isInteger(), ShaderDecompiler, "Type {} is not an integer",
                 *this);
    switch (kind) {
    case TypeKind::Scalar:
        return scalarSignedEquivalent(scalar);
    case TypeKind::Vector:
        return vector.signedEquivalent();
    default:
        unreachable();
    }
}

Type Type::unsignedEquivalent() const {
    ASSERT_DEBUG(isInteger(), ShaderDecompiler, "Type {} is not an integer",
                 *this);
    switch (kind) {
    case TypeKind::Scalar:
        return scalarUnsignedEquivalent(scalar);
    case TypeKind::Vector:
        return vector.unsignedEquivalent();
    default:
        unreachable();
    }
}

} // namespace hydra::hw::tegra_x1::gpu::renderer::shader_decomp::ir
