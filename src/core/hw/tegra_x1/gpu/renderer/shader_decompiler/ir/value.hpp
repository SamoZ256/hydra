#pragma once

#include "core/hw/tegra_x1/gpu/renderer/shader_decompiler/ir/type.hpp"

namespace hydra::hw::tegra_x1::gpu::renderer::shader_decomp::ir {

enum class ValueKind {
    Undefined,
    RawValue,
    Constant,
    Local,
    Register,
    Predicate,
    AttrMemory,
    ConstMemory,
    Label,
};

class Value {
  public:
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmissing-field-initializers"
    static Value Undefined() { return Value{ValueKind::Undefined}; }
#pragma GCC diagnostic pop
    template <typename T>
    static Value RawValue(const T raw_value) {
        return Value{.kind = ValueKind::RawValue,
                     .type = {},
                     .raw_value = static_cast<u64>(raw_value)};
    }
    static Value Constant(const u32 constant, const ScalarType type) {
        return Value{
            .kind = ValueKind::Constant, .type = type, .constant = constant};
    }
    static Value ConstantB(const bool constant) {
        return Constant(constant, ScalarType::Bool);
    }
    static Value ConstantU(const u32 constant,
                           const ScalarType type = ScalarType::U32) {
        return Constant(constant, type);
    }
    static Value ConstantI(const i32 constant,
                           const ScalarType type = ScalarType::I32) {
        return Constant(std::bit_cast<u32>(constant), type);
    }
    static Value ConstantF(const f32 constant,
                           const ScalarType type = ScalarType::F32) {
        return Constant(std::bit_cast<u32>(constant), type);
    }
    template <typename T>
    static Value Constant(const T constant) {
        if constexpr (std::is_same_v<T, bool>)
            return ConstantB(constant);
        else if constexpr (std::is_same_v<T, u8>)
            return ConstantU(constant, ScalarType::U8);
        else if constexpr (std::is_same_v<T, u16>)
            return ConstantU(constant, ScalarType::U16);
        else if constexpr (std::is_same_v<T, u32>)
            return ConstantU(constant, ScalarType::U32);
        else if constexpr (std::is_same_v<T, i8>)
            return ConstantI(constant, ScalarType::I8);
        else if constexpr (std::is_same_v<T, i16>)
            return ConstantI(constant, ScalarType::I16);
        else if constexpr (std::is_same_v<T, i32>)
            return ConstantI(constant, ScalarType::I32);
        else if constexpr (std::is_same_v<T, f32>)
            return ConstantF(constant, ScalarType::F32);
        else
            static_assert(always_false<T>::value, "Unsupported type");
    }
    static Value Local(const local_t local, const Type type = ScalarType::U32) {
        return Value{.kind = ValueKind::Local, .type = type, .local = local};
    }
    static Value Register(const reg_t reg, const Type type = ScalarType::U32) {
        return Value{.kind = ValueKind::Register, .type = type, .reg = reg};
    }
    static Value Predicate(const pred_t pred) {
        return Value{.kind = ValueKind::Predicate,
                     .type = ScalarType::Bool,
                     .pred = pred};
    }
    static Value AttrMemory(const AMem& amem,
                            const Type type = ScalarType::U32) {
        return Value{.kind = ValueKind::AttrMemory, .type = type, .amem = amem};
    }
    static Value ConstMemory(const CMem& cmem,
                             const Type type = ScalarType::U32) {
        return Value{
            .kind = ValueKind::ConstMemory, .type = type, .cmem = cmem};
    }
    static Value Label(const label_t label) {
        return Value{.kind = ValueKind::Label, .type = {}, .label = label};
    }

    bool operator==(const Value& other) const {
        if (kind != other.kind)
            return false;

        switch (kind) {
        case ValueKind::Undefined:
            return true;
        case ValueKind::RawValue:
            return raw_value == other.raw_value;
        case ValueKind::Constant:
            return constant == other.constant;
        case ValueKind::Local:
            return local == other.local;
        case ValueKind::Register:
            return reg == other.reg;
        case ValueKind::Predicate:
            return pred == other.pred;
        case ValueKind::AttrMemory:
            return amem == other.amem;
        case ValueKind::ConstMemory:
            return cmem == other.cmem;
        case ValueKind::Label:
            return label == other.label;
        }
    }

  public: // TODO: private?
    ValueKind kind;
    Type type;

    union {
        u64 raw_value;
        u32 constant;
        local_t local;
        reg_t reg;
        pred_t pred;
        AMem amem;
        CMem cmem;
        label_t label;
    };

    template <ValueKind kind_>
    void AssertKind() const {
        ASSERT_DEBUG(kind == kind_, ShaderDecompiler,
                     "Invalid value kind (expected {}, got {})", kind_, kind);
    }

  public:
    GETTER(kind, GetKind);
    GETTER(type, GetType);

    template <typename T>
    T GetRawValue() const {
        AssertKind<ValueKind::RawValue>();
        return static_cast<T>(raw_value);
    }
    u32 GetConstant() const {
        AssertKind<ValueKind::Constant>();
        return constant;
    }
    local_t GetLocal() const {
        AssertKind<ValueKind::Local>();
        return local;
    }
    reg_t GetRegister() const {
        AssertKind<ValueKind::Register>();
        return reg;
    }
    pred_t GetPredicate() const {
        AssertKind<ValueKind::Predicate>();
        return pred;
    }
    AMem GetAttrMemory() const {
        AssertKind<ValueKind::AttrMemory>();
        return amem;
    }
    CMem GetConstMemory() const {
        AssertKind<ValueKind::ConstMemory>();
        return cmem;
    }
    label_t GetLabel() const {
        AssertKind<ValueKind::Label>();
        return label;
    }
};

} // namespace hydra::hw::tegra_x1::gpu::renderer::shader_decomp::ir

ENABLE_ENUM_FORMATTING(
    hydra::hw::tegra_x1::gpu::renderer::shader_decomp::ir::ValueKind, Undefined,
    "undefined", RawValue, "raw value", Constant, "constant", Register,
    "register", Predicate, "predicate", AttrMemory, "attribute memory",
    ConstMemory, "constant memory", Label, "label")

template <>
struct fmt::formatter<
    hydra::hw::tegra_x1::gpu::renderer::shader_decomp::ir::Value>
    : formatter<string_view> {
    template <typename FormatContext>
    auto
    format(const hydra::hw::tegra_x1::gpu::renderer::shader_decomp::ir::Value&
               value,
           FormatContext& ctx) const {
        std::string str;
        switch (value.GetKind()) {
        case hydra::hw::tegra_x1::gpu::renderer::shader_decomp::ir::ValueKind::
            Undefined:
            str = "undefined";
            break;
        case hydra::hw::tegra_x1::gpu::renderer::shader_decomp::ir::ValueKind::
            RawValue:
            // TODO: figure out a better way to print this
            str = fmt::format("0x{:x}", value.GetRawValue<hydra::u64>());
            break;
        case hydra::hw::tegra_x1::gpu::renderer::shader_decomp::ir::ValueKind::
            Constant:
            str = fmt::format("0x{:08x}", value.GetConstant());
            break;
        case hydra::hw::tegra_x1::gpu::renderer::shader_decomp::ir::ValueKind::
            Local:
            str = fmt::format("{}", value.GetLocal());
            break;
        case hydra::hw::tegra_x1::gpu::renderer::shader_decomp::ir::ValueKind::
            Register:
            str = fmt::format("{}", value.GetRegister());
            break;
        case hydra::hw::tegra_x1::gpu::renderer::shader_decomp::ir::ValueKind::
            Predicate:
            str = fmt::format("{}", value.GetPredicate());
            break;
        case hydra::hw::tegra_x1::gpu::renderer::shader_decomp::ir::ValueKind::
            AttrMemory:
            str = fmt::format("{}", value.GetAttrMemory());
            break;
        case hydra::hw::tegra_x1::gpu::renderer::shader_decomp::ir::ValueKind::
            ConstMemory:
            str = fmt::format("{}", value.GetConstMemory());
            break;
        case hydra::hw::tegra_x1::gpu::renderer::shader_decomp::ir::ValueKind::
            Label:
            str = fmt::format("{}", value.GetLabel());
            break;
        }

        return formatter<string_view>::format(str, ctx);
    }
};
