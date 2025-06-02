#pragma once

#include "core/hw/tegra_x1/gpu/renderer/shader_decompiler/builder_base.hpp"

#define WRITE_ARGS fmt::format_string<T...> f, T &&... args
#define FMT fmt::format(f, std::forward<T>(args)...)

#define COMPONENT_STR(component) ("xyzw"[component])

namespace hydra::hw::tegra_x1::gpu::renderer::shader_decomp::Lang {

#define INVALID_VALUE "INVALID"

class Value : public ValueBase {
    friend class LangBuilderBase;

  private:
    const std::string str;

    Value(const std::string_view str_) : str{str_} {}
};

// TODO: keep track of created values
class LangBuilderBase : public BuilderBase {
    friend class StructuredIterator;

  public:
    using BuilderBase::BuilderBase;

    void Start() override;
    void Finish() override;

    void SetPredCond(const PredCond pred_cond) override;

    // Operations

    // Value
    ValueBase* OpImmediateL(u32 imm, DataType data_type = DataType::U32,
                            bool neg = false) override;
    ValueBase* OpRegister(bool load, reg_t reg,
                          DataType data_type = DataType::U32,
                          bool neg = false) override;
    ValueBase* OpPredicate(bool load, pred_t pred, bool not_ = false) override;
    ValueBase* OpAttributeMemory(bool load, const AMem& amem,
                                 DataType data_type = DataType::U32,
                                 bool neg = false) override;
    ValueBase* OpConstMemoryL(const CMem& cmem,
                              DataType data_type = DataType::U32,
                              bool neg = false) override;

    // Basic
    void OpMove(ValueBase* dst, ValueBase* src) override;
    ValueBase* OpAdd(ValueBase* srcA, ValueBase* srcB) override;
    ValueBase* OpMultiply(ValueBase* srcA, ValueBase* srcB) override;
    ValueBase* OpFloatFma(ValueBase* srcA, ValueBase* srcB,
                          ValueBase* srcC) override;
    ValueBase* OpShiftLeft(ValueBase* src, u32 shift) override;
    ValueBase* OpCast(ValueBase* src, DataType dst_type) override;
    ValueBase* OpCompare(ComparisonOperator cmp, ValueBase* srcA,
                         ValueBase* srcB) override;
    ValueBase* OpBinary(BinaryOperator bin, ValueBase* srcA,
                        ValueBase* srcB) override;
    ValueBase* OpSelect(ValueBase* cond, ValueBase* src_true,
                        ValueBase* src_false) override;

    // Control flow
    void OpExit() override;

    // Math
    ValueBase* OpMin(ValueBase* srcA, ValueBase* srcB) override;
    ValueBase* OpMax(ValueBase* srcA, ValueBase* srcB) override;
    ValueBase* OpMathFunction(MathFunc func, ValueBase* src) override;

    // Special
    ValueBase* OpInterpolate(ValueBase* src) override;
    void OpTextureSample(ValueBase* dstA, ValueBase* dstB, ValueBase* dstC,
                         ValueBase* dstD, u32 const_buffer_index,
                         ValueBase* coords_x, ValueBase* coords_y) override;

  protected:
    virtual void EmitHeader() = 0;
    virtual void EmitTypeAliases() = 0;
    virtual void EmitDeclarations() = 0;
    virtual void EmitMainPrototype() = 0;
    virtual void EmitExit() = 0;

    virtual std::string EmitTextureSample(u32 const_buffer_index,
                                          const std::string_view coords) = 0;

    template <typename... T> void WriteRaw(WRITE_ARGS) { code_str += FMT; }

    template <typename... T> void WriteWithIndent(WRITE_ARGS) {
        // TODO: handle indentation differently
        std::string indent_str;
        if (!skip_indent) {
            for (u32 i = 0; i < indent; i++) {
                indent_str += "    ";
            }
        } else {
            skip_indent = false;
        }
        WriteRaw("{}{}", indent_str, FMT);
    }

    template <typename... T> void Write(WRITE_ARGS) {
        WriteWithIndent("{}\n", FMT);
    }

    void WriteNewline() { code_str += "\n"; }

    template <typename... T> void WriteStatement(WRITE_ARGS) {
        Write("{};", FMT);
    }

    template <typename... T> void EnterScopeTemp(WRITE_ARGS) {
        WriteWithIndent("{} ", FMT);
        skip_indent = true;
    }

    void EnterScopeEmpty() { EnterScopeImpl(""); }

    template <typename... T> void EnterScope(WRITE_ARGS) {
        EnterScopeImpl("{} ", FMT);
    }

    void ExitScopeEmpty(bool semicolon = false) {
        if (semicolon)
            ExitScopeImpl(";");
        else
            ExitScopeImpl("");
    }

    template <typename... T> void ExitScope(WRITE_ARGS) {
        ExitScopeImpl(" {};", FMT);
    }

    // Helpers
    std::string GetImmediateL(u32 imm, DataType data_type = DataType::U32) {
        return fmt::format("as_type<{}>(uint(0x{:08x}u))", data_type, imm);
    }

    std::string GetRegister(bool load, reg_t reg,
                            DataType data_type = DataType::U32) {
        if (load && reg == RZ)
            return GetImmediate(0, data_type);

        return fmt::format("r[{}].{}", reg, GetTypePrefix(data_type));
    }

    std::string GetPredicate(bool load, pred_t pred) {
        if (load && pred == PT)
            return GetImmediate(true);

        return fmt::format("p[{}]", pred);
    }

    std::string GetAttributeMemory(bool load, const AMem amem,
                                   DataType data_type = DataType::U32) {
        // TODO: support indexing with reg
        return fmt::format("a[0x{:08x}].{}", amem.imm / sizeof(u32),
                           GetTypePrefix(data_type));
    }

    std::string GetConstMemoryL(const CMem cmem,
                                DataType data_type = DataType::U32) {
        return fmt::format("c[{}][{} + 0x{:08x}].{}", cmem.idx,
                           GetRegister(true, cmem.reg), cmem.imm / sizeof(u32),
                           GetTypePrefix(data_type));
    }

    const char GetComponentFromIndex(u8 component_index) {
        ASSERT_DEBUG(component_index < 4, ShaderDecompiler,
                     "Invalid component index {}", component_index);

        return "xyzw"[component_index];
    }

    const std::string GetTypePrefix(DataType data_type) {
        switch (data_type) {
        case DataType::U8:
            return "_u8";
        case DataType::U16:
            return "_u16";
        case DataType::U32:
            return "_u32";
        case DataType::I8:
            return "_i8";
        case DataType::I16:
            return "_i16";
        case DataType::I32:
            return "_i32";
        case DataType::F16:
            return "_f16";
        case DataType::F32:
            return "_f32";
        default:
            LOG_ERROR(ShaderDecompiler, "Invalid data type {}", data_type);
            return INVALID_VALUE;
        }
    }

    std::string GetMathFunc(MathFunc func) {
        // TODO: check
        switch (func) {
        case MathFunc::Cos:
            return "cos";
        case MathFunc::Sin:
            return "sin";
        case MathFunc::Ex2:
            return "exp2";
        case MathFunc::Lg2:
            return "log2";
        case MathFunc::Rcp:
            return "1.0f / ";
        case MathFunc::Rsq:
            return "1.0f / sqrt"; // TODO: isn's there a better way?
        case MathFunc::Rcp64h:
            return "1.0 / "; // TODO: correct?
        case MathFunc::Rsq64h:
            return "1.0 / sqrt"; // TODO: correct?
        case MathFunc::Sqrt:
            return "sqrt";
        default:
            return INVALID_VALUE;
        }
    }

    template <typename T> std::string GetImmediate(const T imm) {
        if constexpr (std::is_same_v<T, i32>)
            return fmt::format("0x{:08x}i", imm);
        if constexpr (std::is_same_v<T, i64>)
            return fmt::format("0x{:08x}ill", imm);
        else if constexpr (std::is_same_v<T, u32>)
            return fmt::format("0x{:08x}u", imm);
        else if constexpr (std::is_same_v<T, u64>)
            return fmt::format("0x{:08x}ull", imm);
        else if constexpr (std::is_same_v<T, f32>)
            return fmt::format("{:#}f", imm);
        else if constexpr (std::is_same_v<T, f64>)
            return fmt::format("{:#}f", imm);
        else if constexpr (std::is_same_v<T, bool>)
            return fmt::format("{}", imm);
        else {
            LOG_ERROR(ShaderDecompiler, "Invalid immediate type {}",
                      typeid(T).name());
            return INVALID_VALUE;
        }
    }

    std::string GetImmediate(const u32 imm, DataType data_type) {
        switch (data_type) {
        case DataType::U8:
            return GetImmediate<u8>(imm & 0xff);
        case DataType::U16:
            return GetImmediate<u16>(imm & 0xffff);
        case DataType::U32:
            return GetImmediate<u32>(imm);
        case DataType::I8:
            return GetImmediate<i8>(std::bit_cast<i8>((u8)(imm & 0xff)));
        case DataType::I16:
            return GetImmediate<i16>(std::bit_cast<i16>((u16)(imm & 0xffff)));
        case DataType::I32:
            return GetImmediate<i32>(std::bit_cast<i32>(imm));
        // TODO: F16
        case DataType::F32:
            return GetImmediate<f32>(std::bit_cast<f32>(imm));
        default:
            return INVALID_VALUE;
        }
    }

    // template <typename... T>
    // std::string GetQualifiedSVName(const SV sv, bool output, WRITE_ARGS)
    // {
    //     // TODO: support qualifiers before the name as well
    //     return fmt::format("{} {}", FMT, GetSVQualifierName(sv, output));
    // }

    std::string GetSvName(const Sv& sv) {
        switch (sv.semantic) {
        case SvSemantic::Position:
            return "position";
        case SvSemantic::UserInOut:
            return fmt::format("user{}", sv.index);
        default:
            LOG_NOT_IMPLEMENTED(ShaderDecompiler, "SV {} (index: {})",
                                sv.semantic, sv.index);
            return INVALID_VALUE;
        }
    }

    std::string GetSvAccessNameQualified(const SvAccess& sv_access,
                                         bool output) {
        // TODO: is it okay to access components just like this?
        return fmt::format("{}.{}.{}", GetInOutName(output),
                           GetSvName(sv_access.sv),
                           GetComponentFromIndex(sv_access.component_index));
    }

    std::string GetInOutName(bool output) {
        return (output ? "__out" : "__in");
    }

  private:
    std::string code_str;

    u32 indent{0};
    bool skip_indent{false};

    template <typename... T> void EnterScopeImpl(WRITE_ARGS) {
        Write("{}{{", FMT);
        indent++;
    }

    template <typename... T> void ExitScopeImpl(WRITE_ARGS) {
        ASSERT_DEBUG(indent != 0, ShaderDecompiler,
                     "Cannot exit scope when indentation is 0");
        indent--;
        Write("}}{}", FMT);
    }

    // Helpers
    void EmitReadToTemp(ValueBase* src, u32 offset);
    void EmitWriteFromTemp(ValueBase* dst, u32 offset);
};

} // namespace hydra::hw::tegra_x1::gpu::renderer::shader_decomp::Lang

#undef FMT
#undef WRITE_ARGS
