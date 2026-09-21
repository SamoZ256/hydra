#pragma once

#include "core/hw/tegra_x1/gpu/renderer/shader_decompiler/const.hpp"

namespace hydra::hw::tegra_x1::gpu::renderer::shader_decomp::ir {

enum class TypeKind {
    Undefined,
    Scalar,
    Vector,
};

// TODO: 64-bit types
enum class ScalarType {
    Bool,
    U8,
    U16,
    U32,
    I8,
    I16,
    I32,
    F16,
    F32,
};

inline bool scalarIsInteger(ScalarType scalar) {
    switch (scalar) {
    case ScalarType::U8:
    case ScalarType::U16:
    case ScalarType::U32:
    case ScalarType::I8:
    case ScalarType::I16:
    case ScalarType::I32:
        return true;
    default:
        return false;
    }
}

inline bool scalarIsSignedInteger(ScalarType scalar) {
    switch (scalar) {
    case ScalarType::I8:
    case ScalarType::I16:
    case ScalarType::I32:
        return true;
    default:
        return false;
    }
}

inline bool scalarIsUnsignedInteger(ScalarType scalar) {
    switch (scalar) {
    case ScalarType::U8:
    case ScalarType::U16:
    case ScalarType::U32:
        return true;
    default:
        return false;
    }
}

inline bool scalarIsFloatingPoint(ScalarType scalar) {
    switch (scalar) {
    case ScalarType::F16:
    case ScalarType::F32:
        return true;
    default:
        return false;
    }
}

ScalarType scalarSignedEquivalent(ScalarType scalar);
ScalarType scalarUnsignedEquivalent(ScalarType scalar);

class VectorType {
  public:
    VectorType(ScalarType element_type, u8 size)
        : element_type{element_type}, size{size} {}

    bool operator==(const VectorType& other) const {
        return element_type == other.element_type && size == other.size;
    }

    // Check
    bool isInteger() const { return scalarIsInteger(element_type); }
    bool isSignedInteger() const { return scalarIsSignedInteger(element_type); }
    bool isUnsignedInteger() const {
        return scalarIsUnsignedInteger(element_type);
    }
    bool isFloatingPoint() const { return scalarIsFloatingPoint(element_type); }
    VectorType signedEquivalent() const {
        return {scalarSignedEquivalent(element_type), size};
    }
    VectorType unsignedEquivalent() const {
        return {scalarUnsignedEquivalent(element_type), size};
    }

  private:
    ScalarType element_type;
    u8 size;

  public:
    GETTER(element_type, getElementType);
    GETTER(size, getSize);
};

class Type {
  public:
    Type() : kind{TypeKind::Undefined} {}
    // NOLINTBEGIN(cppcoreguidelines-explicit-constructor)
    Type(ScalarType scalar_) : kind{TypeKind::Scalar}, scalar{scalar_} {}
    Type(VectorType vector_) : kind{TypeKind::Vector}, vector{vector_} {}
    // NOLINTEND(cppcoreguidelines-explicit-constructor)

    static Type createUndefined() { return {}; }
    static Type createScalar(ScalarType scalar) { return {scalar}; }
    static Type createVector(ScalarType element_type, u8 size) {
        return {VectorType(element_type, size)};
    }

    bool operator==(const Type& other) const {
        if (kind != other.kind)
            return false;

        switch (kind) {
        case TypeKind::Scalar:
            return scalar == other.scalar;
        case TypeKind::Vector:
            return vector == other.vector;
        default:
            return true;
        }
    }

    // Check kind
    bool isUndefined() const { return kind == TypeKind::Undefined; }
    bool isScalar() const { return kind == TypeKind::Scalar; }
    bool isVector() const { return kind == TypeKind::Vector; }

    bool isInteger() const {
        switch (kind) {
        case TypeKind::Scalar:
            return scalarIsInteger(scalar);
        case TypeKind::Vector:
            return vector.isInteger();
        default:
            return false;
        }
    }

    bool isSignedInteger() const {
        switch (kind) {
        case TypeKind::Scalar:
            return scalarIsSignedInteger(scalar);
        case TypeKind::Vector:
            return vector.isSignedInteger();
        default:
            return false;
        }
    }

    bool isUnsignedInteger() const {
        switch (kind) {
        case TypeKind::Scalar:
            return scalarIsUnsignedInteger(scalar);
        case TypeKind::Vector:
            return vector.isUnsignedInteger();
        default:
            return false;
        }
    }

    bool isFloatingPoint() const {
        switch (kind) {
        case TypeKind::Scalar:
            return scalarIsFloatingPoint(scalar);
        case TypeKind::Vector:
            return vector.isFloatingPoint();
        default:
            return false;
        }
    }

    // Get
    ScalarType getScalarType() const;
    VectorType getVectorType() const;

    // Type creation
    Type signedEquivalent() const;
    Type unsignedEquivalent() const;

  private:
    TypeKind kind;
    union {
        ScalarType scalar;
        VectorType vector;
    };

  public:
    GETTER(kind, getKind);
};

} // namespace hydra::hw::tegra_x1::gpu::renderer::shader_decomp::ir

ENABLE_ENUM_FORMATTING(
    hydra::hw::tegra_x1::gpu::renderer::shader_decomp::ir::TypeKind, Undefined,
    "undefined", Scalar, "scalar", Vector, "vector")

ENABLE_ENUM_FORMATTING(
    hydra::hw::tegra_x1::gpu::renderer::shader_decomp::ir::ScalarType, Bool,
    "bool", U8, "u8", U16, "u16", U32, "u32", I8, "i8", I16, "i16", I32, "i32",
    F16, "f16", F32, "f32")

template <>
struct fmt::formatter<
    hydra::hw::tegra_x1::gpu::renderer::shader_decomp::ir::VectorType>
    : formatter<string_view> {
    template <typename FormatContext>
    auto format(
        const hydra::hw::tegra_x1::gpu::renderer::shader_decomp::ir::VectorType&
            vector,
        FormatContext& ctx) const {
        return formatter<string_view>::format(
            fmt::format("{}x{}", vector.getSize(), vector.getElementType()),
            ctx);
    }
};

template <>
struct fmt::formatter<
    hydra::hw::tegra_x1::gpu::renderer::shader_decomp::ir::Type>
    : formatter<string_view> {
    template <typename FormatContext>
    auto format(
        const hydra::hw::tegra_x1::gpu::renderer::shader_decomp::ir::Type& type,
        FormatContext& ctx) const {
        std::string str;
        switch (type.getKind()) {
        case hydra::hw::tegra_x1::gpu::renderer::shader_decomp::ir::TypeKind::
            Undefined:
            str = "undefined";
            break;
        case hydra::hw::tegra_x1::gpu::renderer::shader_decomp::ir::TypeKind::
            Scalar:
            str = fmt::format("{}", type.getScalarType());
            break;
        case hydra::hw::tegra_x1::gpu::renderer::shader_decomp::ir::TypeKind::
            Vector:
            str = fmt::format("{}", type.getVectorType());
            break;
        }

        return formatter<string_view>::format(str, ctx);
    }
};
