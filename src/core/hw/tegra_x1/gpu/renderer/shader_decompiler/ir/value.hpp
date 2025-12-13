#pragma once

#include "core/hw/tegra_x1/gpu/renderer/shader_decompiler/const.hpp"

namespace hydra::hw::tegra_x1::gpu::renderer::shader_decomp::ir {

enum class ValueType {
    Undefined,
    RawValue,
    Immediate,
    Local,
    Register,
    Predicate,
    AttrMemory,
    ConstMemory,
    Label,
};

class Value {
  public:
    static Value Undefined() { return Value{ValueType::Undefined}; }
    template <typename T>
    static Value RawValue(const T raw_value) {
        return Value{ValueType::RawValue,
                     .raw_value = static_cast<u64>(raw_value)};
    }
    static Value Immediate(const u32 imm,
                           const DataType data_type = DataType::U32) {
        return Value{ValueType::Immediate, data_type, .imm = imm};
    }
    static Value Local(const local_t local) {
        return Value{ValueType::Local, .local = local};
    }
    static Value Register(const reg_t reg,
                          const DataType data_type = DataType::U32) {
        return Value{ValueType::Register, data_type, .reg = reg};
    }
    static Value Predicate(const pred_t pred) {
        return Value{ValueType::Predicate, .pred = pred};
    }
    static Value AttrMemory(const AMem& amem,
                            const DataType data_type = DataType::U32) {
        return Value{ValueType::AttrMemory, data_type, .amem = amem};
    }
    static Value ConstMemory(const CMem& cmem,
                             const DataType data_type = DataType::U32) {
        return Value{ValueType::ConstMemory, data_type, .cmem = cmem};
    }
    static Value Label(const label_t label) {
        return Value{ValueType::Label, .label = label};
    }

    bool operator==(const Value& other) const {
        if (type != other.type)
            return false;

        switch (type) {
        case ValueType::Undefined:
            return true;
        case ValueType::RawValue:
            return raw_value == other.raw_value;
        case ValueType::Immediate:
            return imm == other.imm;
        case ValueType::Local:
            return local == other.local;
        case ValueType::Register:
            return reg == other.reg;
        case ValueType::Predicate:
            return pred == other.pred;
        case ValueType::AttrMemory:
            return amem == other.amem;
        case ValueType::ConstMemory:
            return cmem == other.cmem;
        case ValueType::Label:
            return label == other.label;
        }
    }

  public: // TODO: private?
    ValueType type;
    DataType data_type{DataType::Invalid};

    union {
        u64 raw_value;
        u32 imm;
        local_t local;
        reg_t reg;
        pred_t pred;
        AMem amem;
        CMem cmem;
        label_t label;
    };

    template <ValueType type_>
    void AssertType() const {
        ASSERT_DEBUG(type == type_, ShaderDecompiler,
                     "Invalid value type (expected {}, got {})", type_, type);
    }

  public:
    GETTER(type, GetType);
    GETTER(data_type, GetDataType);

    template <typename T>
    T GetRawValue() const {
        AssertType<ValueType::RawValue>();
        return static_cast<T>(raw_value);
    }
    u32 GetImmediate() const {
        AssertType<ValueType::Immediate>();
        return imm;
    }
    local_t GetLocal() const {
        AssertType<ValueType::Local>();
        return local;
    }
    reg_t GetRegister() const {
        AssertType<ValueType::Register>();
        return reg;
    }
    pred_t GetPredicate() const {
        AssertType<ValueType::Predicate>();
        return pred;
    }
    AMem GetAttrMemory() const {
        AssertType<ValueType::AttrMemory>();
        return amem;
    }
    CMem GetConstMemory() const {
        AssertType<ValueType::ConstMemory>();
        return cmem;
    }
    label_t GetLabel() const {
        AssertType<ValueType::Label>();
        return label;
    }
};

} // namespace hydra::hw::tegra_x1::gpu::renderer::shader_decomp::ir

ENABLE_ENUM_FORMATTING(
    hydra::hw::tegra_x1::gpu::renderer::shader_decomp::ir::ValueType, Undefined,
    "undefined", RawValue, "raw value", Immediate, "immediate", Register,
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
        switch (value.GetType()) {
        case hydra::hw::tegra_x1::gpu::renderer::shader_decomp::ir::ValueType::
            Undefined:
            str = "undefined";
            break;
        case hydra::hw::tegra_x1::gpu::renderer::shader_decomp::ir::ValueType::
            RawValue:
            // TODO: figure out a better way to print this
            str = fmt::format("0x{:x}", value.GetRawValue<hydra::u64>());
            break;
        case hydra::hw::tegra_x1::gpu::renderer::shader_decomp::ir::ValueType::
            Immediate:
            str = fmt::format("0x{:08x}", value.GetImmediate());
            break;
        case hydra::hw::tegra_x1::gpu::renderer::shader_decomp::ir::ValueType::
            Local:
            str = fmt::format("{}", value.GetLocal());
            break;
        case hydra::hw::tegra_x1::gpu::renderer::shader_decomp::ir::ValueType::
            Register:
            str = fmt::format("{}", value.GetRegister());
            break;
        case hydra::hw::tegra_x1::gpu::renderer::shader_decomp::ir::ValueType::
            Predicate:
            str = fmt::format("{}", value.GetPredicate());
            break;
        case hydra::hw::tegra_x1::gpu::renderer::shader_decomp::ir::ValueType::
            AttrMemory:
            str = fmt::format("{}", value.GetAttrMemory());
            break;
        case hydra::hw::tegra_x1::gpu::renderer::shader_decomp::ir::ValueType::
            ConstMemory:
            str = fmt::format("{}", value.GetConstMemory());
            break;
        case hydra::hw::tegra_x1::gpu::renderer::shader_decomp::ir::ValueType::
            Label:
            str = fmt::format("{}", value.GetLabel());
            break;
        }

        return formatter<string_view>::format(str, ctx);
    }
};
