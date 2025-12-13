#pragma once

#include "core/hw/tegra_x1/gpu/renderer/shader_decompiler/codegen/emitter.hpp"

#define WRITE_ARGS fmt::format_string<T...> f, T &&... args
#define FMT fmt::format(f, std::forward<T>(args)...)

#define COMPONENT_STR(component) ("xyzw"[component])

namespace hydra::hw::tegra_x1::gpu::renderer::shader_decomp::ir {
struct Block;
}

namespace hydra::hw::tegra_x1::gpu::renderer::shader_decomp::analyzer {
struct CfgNode;
}

namespace hydra::hw::tegra_x1::gpu::renderer::shader_decomp::codegen::lang {

#define INVALID_VALUE "INVALID"

class LangEmitter : public Emitter {
  public:
    using Emitter::Emitter;

  protected:
    void Start() override;
    void Finish() override;

    // Emit

    virtual void EmitHeader() = 0;
    virtual void EmitTypeAliases() = 0;
    virtual void EmitDeclarations() = 0;
    virtual void EmitStateBindings() = 0;
    virtual void EmitStateBindingAssignments() = 0;
    virtual void EmitMainPrototype() = 0;
    virtual void EmitExitReturn() = 0;

    void EmitMainFunctionPrologue();

    void EmitFunction(const ir::Function& func) override;
    // void EmitNode(const ir::Function& func, const analyzer::CfgNode* node);
    void EmitBlock(const ir::Function& func, const ir::Block& block);

    // Data
    void EmitCopy(const ir::Value& dst, const ir::Value& src) override;
    void EmitCast(const ir::Value& dst, const ir::Value& src,
                  DataType dst_type) override;

    // Arithmetic
    void EmitAbs(const ir::Value& dst, const ir::Value& src) override;
    void EmitNeg(const ir::Value& dst, const ir::Value& src) override;
    void EmitAdd(const ir::Value& dst, const ir::Value& srcA,
                 const ir::Value& srcB) override;
    void EmitMultiply(const ir::Value& dst, const ir::Value& srcA,
                      const ir::Value& srcB) override;
    void EmitFma(const ir::Value& dst, const ir::Value& srcA,
                 const ir::Value& srcB, const ir::Value& srcC) override;
    void EmitMin(const ir::Value& dst, const ir::Value& srcA,
                 const ir::Value& srcB) override;
    void EmitMax(const ir::Value& dst, const ir::Value& srcA,
                 const ir::Value& srcB) override;
    void EmitClamp(const ir::Value& dst, const ir::Value& srcA,
                   const ir::Value& srcB, const ir::Value& srcC) override;

    // Math
    void EmitRound(const ir::Value& dst, const ir::Value& src) override;
    void EmitFloor(const ir::Value& dst, const ir::Value& src) override;
    void EmitCeil(const ir::Value& dst, const ir::Value& src) override;
    void EmitTrunc(const ir::Value& dst, const ir::Value& src) override;

    // Logical & Bitwise
    void EmitNot(const ir::Value& dst, const ir::Value& src) override;
    void EmitBitwiseNot(const ir::Value& dst, const ir::Value& src) override;
    void EmitBitwiseAnd(const ir::Value& dst, const ir::Value& srcA,
                        const ir::Value& srcB) override;
    void EmitBitwiseOr(const ir::Value& dst, const ir::Value& srcA,
                       const ir::Value& srcB) override;
    void EmitBitwiseXor(const ir::Value& dst, const ir::Value& srcA,
                        const ir::Value& srcB) override;
    void EmitShiftLeft(const ir::Value& dst, const ir::Value& src_a,
                       const ir::Value& src_b) override;
    void EmitShiftRight(const ir::Value& dst, const ir::Value& src_a,
                        const ir::Value& src_b) override;

    // Comparison & Selection
    void EmitCompareLess(const ir::Value& dst, const ir::Value& srcA,
                         const ir::Value& srcB) override;
    void EmitCompareLessOrEqual(const ir::Value& dst, const ir::Value& srcA,
                                const ir::Value& srcB) override;
    void EmitCompareGreater(const ir::Value& dst, const ir::Value& srcA,
                            const ir::Value& srcB) override;
    void EmitCompareGreaterOrEqual(const ir::Value& dst, const ir::Value& srcA,
                                   const ir::Value& srcB) override;
    void EmitCompareEqual(const ir::Value& dst, const ir::Value& srcA,
                          const ir::Value& srcB) override;
    void EmitCompareNotEqual(const ir::Value& dst, const ir::Value& srcA,
                             const ir::Value& srcB) override;
    void EmitSelect(const ir::Value& dst, const ir::Value& cond,
                    const ir::Value& src_true,
                    const ir::Value& src_false) override;

    // Control flow
    void EmitBeginIf(const ir::Value& cond) override;
    void EmitEndIf() override;
    void EmitBranch(label_t target) override;
    void EmitBranchConditional(const ir::Value& cond, label_t target_true,
                               label_t target_false) override;

    // Vector
    void EmitVectorExtract(const ir::Value& dst, const ir::Value& src,
                           u32 index) override;
    void EmitVectorInsert(const ir::Value& dst, const ir::Value& src,
                          u32 index) override;
    void EmitVectorConstruct(const ir::Value& dst, DataType data_type,
                             const std::vector<ir::Value>& elements) override;

    // Exit
    void EmitExit() override;

    // Helpers
    template <typename... T>
    void WriteRaw(WRITE_ARGS) {
        code_str += FMT;
    }

    template <typename... T>
    void WriteWithIndent(WRITE_ARGS) {
        WriteRaw("{:{}}{}", "", indent * 4, FMT);
    }

    template <typename... T>
    void Write(WRITE_ARGS) {
        WriteWithIndent("{}\n", FMT);
    }

    void WriteNewline() { code_str += "\n"; }

    template <typename... T>
    void WriteStatement(WRITE_ARGS) {
        Write("{};", FMT);
    }

    template <typename... T>
    void EnterScope(WRITE_ARGS) {
        EnterScopeImpl("{} ", FMT);
    }

    void EnterScopeEmpty() { EnterScopeImpl(""); }

    template <typename... T>
    void ExitScope(WRITE_ARGS) {
        ExitScopeImpl(" {};", FMT);
    }

    void ExitScopeEmpty(bool semicolon = false) {
        if (semicolon)
            ExitScopeImpl(";");
        else
            ExitScopeImpl("");
    }

    template <typename T>
    std::string GetImmediateStr(const T imm) {
        if constexpr (std::is_same_v<T, i32>)
            return fmt::format("({:#x})", imm);
        if constexpr (std::is_same_v<T, i64>)
            return fmt::format("({:#x}ll)", imm);
        else if constexpr (std::is_same_v<T, u32>)
            return fmt::format("({:#x}u)", imm);
        else if constexpr (std::is_same_v<T, u64>)
            return fmt::format("({:#x}ull)", imm);
        else if constexpr (std::is_same_v<T, f32>)
            return fmt::format("({:#}f)", imm);
        else if constexpr (std::is_same_v<T, f64>)
            return fmt::format("({:#}f)", imm);
        else if constexpr (std::is_same_v<T, bool>)
            return fmt::format("{}", imm);
        else {
            LOG_ERROR(ShaderDecompiler, "Invalid immediate type {}",
                      typeid(T).name());
            return INVALID_VALUE;
        }
    }

    std::string GetImmediateStr(const u32 imm, DataType data_type) {
        switch (data_type) {
        case DataType::U8:
            return GetImmediateStr<u8>(imm & 0xff);
        case DataType::U16:
            return GetImmediateStr<u16>(imm & 0xffff);
        case DataType::U32:
            return GetImmediateStr<u32>(imm);
        case DataType::I8:
            return GetImmediateStr<i8>(std::bit_cast<i8>((u8)(imm & 0xff)));
        case DataType::I16:
            return GetImmediateStr<i16>(
                std::bit_cast<i16>((u16)(imm & 0xffff)));
        case DataType::I32:
            return GetImmediateStr<i32>(std::bit_cast<i32>(imm));
        case DataType::F16:
            return fmt::format("as_type<f16>((u16)0x{:04x})",
                               u16(imm & 0xffff));
        case DataType::F32:
            return GetImmediateStr<f32>(std::bit_cast<f32>(imm));
        case DataType::F16X2:
            return fmt::format("half2(as_type<f16>((u16)0x{:04x}), "
                               "as_type<f16>((u16)0x{:04x}))",
                               u16(imm & 0xffff), u16((imm >> 16) & 0xffff));
        default:
            return INVALID_VALUE;
        }
    }

    std::string GetLocalStr(local_t local) {
        return fmt::format("local0x{:x}_{}", u32(local.label), local.id);
    }

    template <bool load = true>
    std::string GetRegisterStr(reg_t reg, DataType data_type = DataType::U32) {
        if (load && reg == RZ)
            return GetImmediateStr(0, data_type);

        return fmt::format("state.r[{}].{}", u32(reg),
                           GetTypeSuffixStr(data_type));
    }

    template <bool load = true>
    std::string GetPredicateStr(pred_t pred) {
        if (load && pred == PT)
            return GetImmediateStr(true);

        return fmt::format("state.p[{}]", u32(pred));
    }

    std::string GetAttrMemoryStr(const AMem amem,
                                 DataType data_type = DataType::U32) {
        // TODO: what about unaligned access?
        return fmt::format("state.a_{}[({} + 0x{:08x}) >> 2].{}",
                           (amem.is_input ? "in" : "out"),
                           GetRegisterStr(amem.reg), amem.imm,
                           GetTypeSuffixStr(data_type));
    }

    std::string GetConstMemoryStr(const CMem cmem,
                                  DataType data_type = DataType::U32) {
        // TODO: what about unaligned access?
        return fmt::format("state.c{}[({} + 0x{:08x}) >> 2].{}", cmem.idx,
                           GetRegisterStr(cmem.reg), cmem.imm,
                           GetTypeSuffixStr(data_type));
    }

    std::string GetValueStr(const ir::Value& value) {
        switch (value.GetType()) {
        case ir::ValueType::Immediate:
            return GetImmediateStr(value.GetImmediate(), value.GetDataType());
        case ir::ValueType::Local:
            return GetLocalStr(value.GetLocal());
        case ir::ValueType::Register:
            return GetRegisterStr(value.GetRegister(), value.GetDataType());
        case ir::ValueType::Predicate:
            return GetPredicateStr(value.GetPredicate());
        case ir::ValueType::AttrMemory:
            return GetAttrMemoryStr(value.GetAttrMemory(), value.GetDataType());
        case ir::ValueType::ConstMemory:
            return GetConstMemoryStr(value.GetConstMemory(),
                                     value.GetDataType());
        default:
            LOG_FATAL(ShaderDecompiler, "Invalid value type {} for src",
                      value.GetType());
        }
    }

    template <typename... T>
    void StoreValue(const ir::Value& dst, WRITE_ARGS) {
        switch (dst.GetType()) {
        case ir::ValueType::Local:
            // TODO: don't use auto
            WriteStatement("auto {} = {}", GetLocalStr(dst.GetLocal()), FMT);
            break;
        case ir::ValueType::Register:
            WriteStatement(
                "{} = {}",
                GetRegisterStr<false>(dst.GetRegister(), dst.GetDataType()),
                FMT);
            break;
        case ir::ValueType::Predicate:
            WriteStatement("{} = {}",
                           GetPredicateStr<false>(dst.GetPredicate()), FMT);
            break;
        case ir::ValueType::AttrMemory:
            WriteStatement(
                "{} = {}",
                GetAttrMemoryStr(dst.GetAttrMemory(), dst.GetDataType()), FMT);
            break;
        default:
            LOG_FATAL(ShaderDecompiler, "Invalid value type {} for dst",
                      dst.GetType());
        }
    }

    const char GetComponentStrFromIndex(u8 component_index) {
        ASSERT_DEBUG(component_index < 4, ShaderDecompiler,
                     "Invalid component index {}", component_index);

        return "xyzw"[component_index];
    }

    const std::string GetTypeSuffixStr(DataType data_type) {
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
        case DataType::F16X2:
            return "_f16x2";
        default:
            LOG_ERROR(ShaderDecompiler, "Invalid data type {}", data_type);
            return INVALID_VALUE;
        }
    }

    // template <typename... T>
    // std::string GetQualifiedSVName(const SV sv, bool output, WRITE_ARGS)
    // {
    //     // TODO: support qualifiers before the name as well
    //     return fmt::format("{} {}", FMT, GetSVQualifierName(sv, output));
    // }

    virtual std::string GetSvAccessQualifiedStr(const SvAccess& sv_access,
                                                bool output) = 0;

  private:
    std::string code_str;

    u32 indent{0};

    template <typename... T>
    void EnterScopeImpl(WRITE_ARGS) {
        Write("{}{{", FMT);
        indent++;
    }

    template <typename... T>
    void ExitScopeImpl(WRITE_ARGS) {
        ASSERT_DEBUG(indent != 0, ShaderDecompiler,
                     "Cannot exit scope when indentation is 0");
        indent--;
        Write("}}{}", FMT);
    }
};

} // namespace hydra::hw::tegra_x1::gpu::renderer::shader_decomp::codegen::lang

#undef FMT
#undef WRITE_ARGS
