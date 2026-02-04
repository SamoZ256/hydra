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

enum class TypeError {
    NotAScalar,
    NotAVector,
    NotABoolean,
    NotAnInteger,
    NotAFloatingPoint,
    NotSigned,
    NotUnsigned,
};

inline bool ScalarIsInteger(ScalarType scalar) {
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

inline bool ScalarIsSignedInteger(ScalarType scalar) {
    switch (scalar) {
    case ScalarType::I8:
    case ScalarType::I16:
    case ScalarType::I32:
        return true;
    default:
        return false;
    }
}

inline bool ScalarIsUnsignedInteger(ScalarType scalar) {
    switch (scalar) {
    case ScalarType::U8:
    case ScalarType::U16:
    case ScalarType::U32:
        return true;
    default:
        return false;
    }
}

inline bool ScalarIsFloatingPoint(ScalarType scalar) {
    switch (scalar) {
    case ScalarType::F16:
    case ScalarType::F32:
        return true;
    default:
        return false;
    }
}

ScalarType ScalarSignedEquivalent(ScalarType scalar);
ScalarType ScalarUnsignedEquivalent(ScalarType scalar);

class VectorType {
  public:
    VectorType(ScalarType element_type, u8 size)
        : element_type{element_type}, size{size} {}

    bool operator==(const VectorType& other) const {
        return element_type == other.element_type && size == other.size;
    }

    // Check
    bool IsInteger() const { return ScalarIsInteger(element_type); }
    bool IsSignedInteger() const { return ScalarIsSignedInteger(element_type); }
    bool IsUnsignedInteger() const {
        return ScalarIsUnsignedInteger(element_type);
    }
    bool IsFloatingPoint() const { return ScalarIsFloatingPoint(element_type); }
    VectorType SignedEquivalent() const {
        return VectorType(ScalarSignedEquivalent(element_type), size);
    }
    VectorType UnsignedEquivalent() const {
        return VectorType(ScalarUnsignedEquivalent(element_type), size);
    }

  private:
    ScalarType element_type;
    u8 size;

  public:
    GETTER(element_type, GetElementType);
    GETTER(size, GetSize);
};

class Type {
  public:
    Type() : kind{TypeKind::Undefined} {}
    Type(ScalarType scalar_) : kind{TypeKind::Scalar}, scalar{scalar_} {}
    Type(VectorType vector_) : kind{TypeKind::Vector}, vector{vector_} {}

    static Type Undefined() { return Type(); }
    static Type Scalar(ScalarType scalar) { return Type(scalar); }
    static Type Vector(ScalarType element_type, u8 size) {
        return Type(VectorType(element_type, size));
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
    bool IsUndefined() const { return kind == TypeKind::Undefined; }
    bool IsScalar() const { return kind == TypeKind::Scalar; }
    bool IsVector() const { return kind == TypeKind::Vector; }

    bool IsInteger() const {
        switch (kind) {
        case TypeKind::Scalar:
            return ScalarIsInteger(scalar);
        case TypeKind::Vector:
            return vector.IsInteger();
        default:
            return false;
        }
    }

    bool IsSignedInteger() const {
        switch (kind) {
        case TypeKind::Scalar:
            return ScalarIsSignedInteger(scalar);
        case TypeKind::Vector:
            return vector.IsSignedInteger();
        default:
            return false;
        }
    }

    bool IsUnsignedInteger() const {
        switch (kind) {
        case TypeKind::Scalar:
            return ScalarIsUnsignedInteger(scalar);
        case TypeKind::Vector:
            return vector.IsUnsignedInteger();
        default:
            return false;
        }
    }

    bool IsFloatingPoint() const {
        switch (kind) {
        case TypeKind::Scalar:
            return ScalarIsFloatingPoint(scalar);
        case TypeKind::Vector:
            return vector.IsFloatingPoint();
        default:
            return false;
        }
    }

    // Get
    ScalarType GetScalarType() const;
    VectorType GetVectorType() const;

    // Type creation
    Type SignedEquivalent() const;
    Type UnsignedEquivalent() const;

  private:
    TypeKind kind;
    union {
        ScalarType scalar;
        VectorType vector;
    };

  public:
    GETTER(kind, GetKind);
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
            vector_type,
        FormatContext& ctx) const {
        return formatter<string_view>::format(
            fmt::format("{}x{}", vector_type.GetSize(),
                        vector_type.GetElementType()),
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
        switch (type.GetKind()) {
        case hydra::hw::tegra_x1::gpu::renderer::shader_decomp::ir::TypeKind::
            Undefined:
            str = "undefined";
            break;
        case hydra::hw::tegra_x1::gpu::renderer::shader_decomp::ir::TypeKind::
            Scalar:
            str = fmt::format("{}", type.GetScalarType());
            break;
        case hydra::hw::tegra_x1::gpu::renderer::shader_decomp::ir::TypeKind::
            Vector:
            str = fmt::format("{}", type.GetVectorType());
            break;
        }

        return formatter<string_view>::format(str, ctx);
    }
};
