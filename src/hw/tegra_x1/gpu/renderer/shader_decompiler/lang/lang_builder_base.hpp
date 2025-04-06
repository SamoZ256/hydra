#pragma once

#include "hw/tegra_x1/gpu/renderer/shader_decompiler/builder_base.hpp"

#define WRITE_ARGS fmt::format_string<T...> fmt, T &&... args
#define FMT fmt::format(fmt, std::forward<T>(args)...)

#define COMPONENT_STR(component) ("xyzw"[component])

namespace Hydra::HW::TegraX1::GPU::Renderer::ShaderDecompiler::Lang {

#define INVALID_VALUE "INVALID"

class LangBuilderBase : public BuilderBase {
  public:
    LangBuilderBase(const Analyzer& analyzer, const ShaderType type,
                    const GuestShaderState& state, std::vector<u8>& out_code,
                    ResourceMapping& out_resource_mapping)
        : BuilderBase(analyzer, type, state, out_code, out_resource_mapping) {}

    void Start() override;
    void Finish() override;

    // Operations
    void OpExit() override;
    void OpMove(reg_t dst, Operand src) override;
    void OpFloatAdd(reg_t dst, reg_t src1, Operand src2) override;
    void OpFloatMultiply(reg_t dst, reg_t src1, Operand src2) override;
    void OpFloatFma(reg_t dst, reg_t src1, Operand src2, reg_t src3) override;
    void OpShiftLeft(reg_t dst, reg_t src, u32 shift) override;
    void OpMathFunction(MathFunc func, reg_t dst, reg_t src) override;
    void OpLoad(reg_t dst, Operand src) override;
    void OpStore(AMem dst, reg_t src) override;
    void OpInterpolate(reg_t dst, AMem src) override;
    void OpTextureSample(reg_t dst, u32 index, reg_t coords) override;

  protected:
    virtual void InitializeResourceMapping() = 0;

    virtual void EmitHeader() = 0;
    virtual void EmitTypeAliases() = 0;
    virtual void EmitDeclarations() = 0;
    virtual void EmitMainPrototype() = 0;

    virtual std::string EmitTextureSample(u32 index,
                                          const std::string& coords) = 0;

    template <typename... T> void Write(WRITE_ARGS) {
        // TODO: handle indentation differently
        std::string indent_str;
        for (u32 i = 0; i < indent; i++) {
            indent_str += "    ";
        }
        WriteRaw("{}{}\n", indent_str, FMT);
    }

    template <typename... T> void WriteRaw(WRITE_ARGS) { code_str += FMT; }

    void WriteNewline() { code_str += "\n"; }
    template <typename... T> void WriteStatement(WRITE_ARGS) {
        Write("{};", FMT);
    }

    void EnterScopeEmpty() { EnterScopeImpl(""); }

    template <typename... T> void EnterScope(WRITE_ARGS) {
        EnterScopeImpl("{} ", FMT);
    }

    void ExitScopeEmpty(bool semicolon = false) {
        ExitScopeImpl(semicolon ? ";" : "");
    }

    template <typename... T> void ExitScope(WRITE_ARGS) {
        ExitScopeImpl(" {};", FMT);
    }

    // Helpers
    std::string GetReg(reg_t reg, bool write = false,
                       DataType data_type = DataType::UInt) {
        if (reg == RZ && !write)
            return GetImmediate(0, data_type);

        return fmt::format("r[{}].{}", reg, GetTypePrefix(data_type));
    }

    std::string GetImm(u32 imm, DataType data_type = DataType::UInt) {
        return fmt::format("as_type<{}>(uint(0x{:08x}u))", data_type, imm);
    }

    std::string GetA(const AMem amem, DataType data_type = DataType::UInt) {
        // TODO: support indexing with reg
        return fmt::format("a[0x{:08x}].{}", amem.imm / sizeof(u32),
                           GetTypePrefix(data_type));
    }

    std::string GetC(const CMem cmem, DataType data_type = DataType::UInt) {
        return fmt::format("c[{}][{} + 0x{:08x}].{}", cmem.idx,
                           GetReg(cmem.reg), cmem.imm / sizeof(u32),
                           GetTypePrefix(data_type));
    }

    std::string GetOperand(Operand operand, bool write = false,
                           DataType data_type = DataType::UInt) {
        switch (operand.type) {
        case OperandType::Register:
            return GetReg(operand.reg, write, data_type);
        case OperandType::Immediate:
            return GetImm(operand.imm, data_type);
        case OperandType::AttributeMemory:
            return GetA(operand.amem, data_type);
        case OperandType::ConstMemory:
            return GetC(operand.cmem, data_type);
        }
    }

    const char GetComponentFromIndex(u8 component_index) {
        ASSERT_DEBUG(component_index < 4, ShaderDecompiler,
                     "Invalid component index {}", component_index);

        return "xyzw"[component_index];
    }

    const char GetTypePrefix(DataType data_type) {
        switch (data_type) {
        case DataType::Int:
            return 'i';
        case DataType::UInt:
            return 'u';
        case DataType::Float:
            return 'f';
        default:
            LOG_ERROR(ShaderDecompiler, "Invalid data type {}", data_type);
            return 'X';
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
            return "1.0 / ";
        case MathFunc::Rsq:
            return "1.0 / sqrt"; // TODO: isn's there a better way?
        case MathFunc::Rcp64h:
            return "TODO"; // TODO
        case MathFunc::Rsq64h:
            return "TODO"; // TODO
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
        else
            LOG_ERROR(ShaderDecompiler, "Invalid immediate type {}",
                      typeid(T).name());
    }

    std::string GetImmediate(const u32 imm, DataType data_type) {
        switch (data_type) {
        case DataType::Int:
            return GetImmediate(bit_cast<i32>(imm));
        case DataType::UInt:
            return GetImmediate(imm);
        case DataType::Float:
            return GetImmediate(bit_cast<f32>(imm));
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

    std::string GetSVName(const SV sv) {
        switch (sv.semantic) {
        case SVSemantic::Position:
            return "position";
        case SVSemantic::UserInOut:
            return fmt::format("user{}", sv.index);
        default:
            LOG_NOT_IMPLEMENTED(ShaderDecompiler, "SV {} (index: {})",
                                sv.semantic, sv.index);
            return INVALID_VALUE;
        }
    }

    std::string GetSVNameQualified(const SV sv, bool output) {
        // TODO: is it okay to access components just like this?
        return fmt::format("{}.{}.{}", GetInOutName(output), GetSVName(sv),
                           GetComponentFromIndex(sv.component_index));
    }

    std::string GetInOutName(bool output) {
        return (output ? "__out" : "__in");
    }

  private:
    std::string code_str;

    u32 indent = 0;

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
    void EmitReadToTemp(reg_t src, u32 count = 4);
    void EmitWriteFromTemp(reg_t dst, u32 count = 4);
};

} // namespace Hydra::HW::TegraX1::GPU::Renderer::ShaderDecompiler::Lang

#undef FMT
#undef WRITE_ARGS
