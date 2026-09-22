#pragma once

#include "core/hw/tegra_x1/gpu/renderer/shader_decompiler/codegen/emitter.hpp"

#define WRITE_ARGS fmt::format_string<T...> f, T &&... args
#define FMT fmt::format(f, std::forward<T>(args)...)

#define COMPONENT_STR(component) ("xyzw"[component])

namespace hydra::hw::tegra_x1::gpu::renderer::shader_decomp::ir {
class Block;
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
    void start() override;
    void finish() override;

    // Emit

    virtual void emitHeader() = 0;
    virtual void emitTypeAliases() = 0;
    virtual void emitDeclarations() = 0;
    virtual void emitStateBindings() = 0;
    virtual void emitStateBindingAssignments() = 0;
    virtual void emitMainPrototype() = 0;
    virtual void emitExitReturn() = 0;

    void emitMainFunctionPrologue();

    void emitFunction(const ir::Function& func) override;
    // void EmitNode(const ir::Function& func, const analyzer::CfgNode* node);
    void emitBlock(const ir::Function& func, const ir::Block& block);

    // Data
    void emitCopy(const ir::Value& dst, const ir::Value& src) override;
    void emitCast(const ir::Value& dst, const ir::Value& src) override;

    // Arithmetic
    void emitAbs(const ir::Value& dst, const ir::Value& src) override;
    void emitNeg(const ir::Value& dst, const ir::Value& src) override;
    void emitAdd(const ir::Value& dst, const ir::Value& srcA,
                 const ir::Value& srcB) override;
    void emitMultiply(const ir::Value& dst, const ir::Value& srcA,
                      const ir::Value& srcB) override;
    void emitFma(const ir::Value& dst, const ir::Value& srcA,
                 const ir::Value& srcB, const ir::Value& srcC) override;
    void emitMin(const ir::Value& dst, const ir::Value& srcA,
                 const ir::Value& srcB) override;
    void emitMax(const ir::Value& dst, const ir::Value& srcA,
                 const ir::Value& srcB) override;
    void emitClamp(const ir::Value& dst, const ir::Value& srcA,
                   const ir::Value& srcB, const ir::Value& srcC) override;

    // Math
    void emitRound(const ir::Value& dst, const ir::Value& src) override;
    void emitFloor(const ir::Value& dst, const ir::Value& src) override;
    void emitCeil(const ir::Value& dst, const ir::Value& src) override;
    void emitTrunc(const ir::Value& dst, const ir::Value& src) override;

    // Logical & Bitwise
    void emitNot(const ir::Value& dst, const ir::Value& src) override;
    void emitBitwiseNot(const ir::Value& dst, const ir::Value& src) override;
    void emitBitwiseAnd(const ir::Value& dst, const ir::Value& srcA,
                        const ir::Value& srcB) override;
    void emitBitwiseOr(const ir::Value& dst, const ir::Value& srcA,
                       const ir::Value& srcB) override;
    void emitBitwiseXor(const ir::Value& dst, const ir::Value& srcA,
                        const ir::Value& srcB) override;
    void emitShiftLeft(const ir::Value& dst, const ir::Value& src_a,
                       const ir::Value& src_b) override;
    void emitShiftRight(const ir::Value& dst, const ir::Value& src_a,
                        const ir::Value& src_b) override;

    // Comparison & Selection
    void emitCompareLess(const ir::Value& dst, const ir::Value& srcA,
                         const ir::Value& srcB) override;
    void emitCompareLessOrEqual(const ir::Value& dst, const ir::Value& srcA,
                                const ir::Value& srcB) override;
    void emitCompareGreater(const ir::Value& dst, const ir::Value& srcA,
                            const ir::Value& srcB) override;
    void emitCompareGreaterOrEqual(const ir::Value& dst, const ir::Value& srcA,
                                   const ir::Value& srcB) override;
    void emitCompareEqual(const ir::Value& dst, const ir::Value& srcA,
                          const ir::Value& srcB) override;
    void emitCompareNotEqual(const ir::Value& dst, const ir::Value& srcA,
                             const ir::Value& srcB) override;
    void emitSelect(const ir::Value& dst, const ir::Value& cond,
                    const ir::Value& src_true,
                    const ir::Value& src_false) override;

    // Control flow
    void emitBeginIf(const ir::Value& cond) override;
    void emitEndIf() override;
    void emitBranch(label_t target) override;
    void emitBranchConditional(const ir::Value& cond, label_t target_true,
                               label_t target_false) override;

    // Vector
    void emitVectorExtract(const ir::Value& dst, const ir::Value& src,
                           u8 index) override;
    void emitVectorInsert(const ir::Value& dst, const ir::Value& src,
                          u8 index) override;
    void emitVectorConstruct(const ir::Value& dst,
                             const std::vector<ir::Value>& elements) override;

    // Exit
    void emitExit() override;

    // Helpers
    template <typename... T>
    void writeRaw(WRITE_ARGS) {
        code_str += FMT;
    }

    template <typename... T>
    void writeWithIndent(WRITE_ARGS) {
        writeRaw("{:{}}{}", "", indent * 4, FMT);
    }

    template <typename... T>
    void write(WRITE_ARGS) {
        writeWithIndent("{}\n", FMT);
    }

    void writeNewline() { code_str += '\n'; }

    template <typename... T>
    void writeStatement(WRITE_ARGS) {
        write("{};", FMT);
    }

    template <typename... T>
    void enterScope(WRITE_ARGS) {
        enterScopeImpl("{} ", FMT);
    }

    void enterScopeEmpty() { enterScopeImpl(""); }

    template <typename... T>
    void exitScope(WRITE_ARGS) {
        exitScopeImpl(" {};", FMT);
    }

    void exitScopeEmpty(bool semicolon = false) {
        if (semicolon)
            exitScopeImpl(";");
        else
            exitScopeImpl("");
    }

    std::string getTypeStr(const ir::Type& type) {
        switch (type.getKind()) {
        case ir::TypeKind::Scalar: {
            switch (type.getScalarType()) {
            case ir::ScalarType::Bool:
                return "bool";
            case ir::ScalarType::U8:
                return "u8";
            case ir::ScalarType::U16:
                return "u16";
            case ir::ScalarType::U32:
                return "u32";
            case ir::ScalarType::I8:
                return "i8";
            case ir::ScalarType::I16:
                return "i16";
            case ir::ScalarType::I32:
                return "i32";
            case ir::ScalarType::F16:
                return "f16";
            case ir::ScalarType::F32:
                return "f32";
            }
        }
        case ir::TypeKind::Vector: {
            const auto& vec_type = type.getVectorType();
            return fmt::format("vec<{}, {}>",
                               getTypeStr(vec_type.getElementType()),
                               vec_type.getSize());
        }
        default:
            LOG_ERROR(ShaderDecompiler, "Unsupported type for immediate ({})",
                      type);
            return INVALID_VALUE;
        }
    }

    template <typename T>
    std::string getConstantStr(const T imm) {
        if constexpr (std::is_same_v<T, bool>)
            return fmt::format("({})", imm);
        else if constexpr (std::is_same_v<T, u32>)
            return fmt::format("({:#x}u)", imm);
        else if constexpr (std::is_same_v<T, i32>)
            return fmt::format("({:#x})", imm);
        else if constexpr (std::is_same_v<T, f32>)
            return fmt::format("({:#}f)", imm);
        else
            static_assert(always_false<T>::value, "Unsupported immediate type");
    }

    std::string getConstantStr(const u32 imm, ir::Type type) {
        // NOLINTNEXTLINE(readability-trivial-switch)
        switch (type.getKind()) {
        case ir::TypeKind::Scalar: {
            switch (type.getScalarType()) {
            case ir::ScalarType::Bool:
                return getConstantStr<bool>((imm & 0x1) != 0u);
            case ir::ScalarType::U8:
                return getConstantStr<u32>(imm & 0xff);
            case ir::ScalarType::U16:
                return getConstantStr<u32>(imm & 0xffff);
            case ir::ScalarType::U32:
                return getConstantStr<u32>(imm);
            case ir::ScalarType::I8:
                return getConstantStr<i32>(
                    std::bit_cast<i8>(static_cast<u8>(imm & 0xff)));
            case ir::ScalarType::I16:
                return getConstantStr<i32>(
                    std::bit_cast<i16>(static_cast<u16>(imm & 0xffff)));
            case ir::ScalarType::I32:
                return getConstantStr<i32>(std::bit_cast<i32>(imm));
            case ir::ScalarType::F16:
                return fmt::format("as_type<f16>((u16)0x{:04x})",
                                   static_cast<u16>(imm & 0xffff));
            case ir::ScalarType::F32:
                return getConstantStr<f32>(std::bit_cast<f32>(imm));
            }
        }
        default:
            LOG_ERROR(ShaderDecompiler, "Unsupported type for immediate ({})",
                      type);
            return INVALID_VALUE;
        }
    }

    static std::string getLocalStr(local_t local) {
        return fmt::format("local0x{:x}_{}", static_cast<u32>(local.label),
                           local.id);
    }

    template <bool load = true>
    std::string getRegisterStr(reg_t reg, ir::Type type = ir::ScalarType::U32) {
        if (load && reg == RZ)
            return getConstantStr(0, type);

        return fmt::format("state.r[{}].{}", static_cast<u32>(reg),
                           getTypeSuffixStr(type));
    }

    template <bool load = true>
    std::string getPredicateStr(pred_t pred) {
        if (load && pred == PT)
            return getConstantStr(true);

        return fmt::format("state.p[{}]", static_cast<u32>(pred));
    }

    std::string getAttrMemoryStr(const AMem amem,
                                 ir::Type type = ir::ScalarType::U32) {
        // TODO: what about unaligned access?
        return fmt::format("state.a_{}[({} + 0x{:08x}) >> 2].{}",
                           (amem.is_input ? "in" : "out"),
                           getRegisterStr(amem.reg), amem.imm,
                           getTypeSuffixStr(type));
    }

    std::string getConstMemoryStr(const CMem cmem,
                                  ir::Type type = ir::ScalarType::U32) {
        // TODO: what about unaligned access?
        return fmt::format("state.c{}[({} + 0x{:08x}) >> 2].{}", cmem.idx,
                           getRegisterStr(cmem.reg), cmem.imm,
                           getTypeSuffixStr(type));
    }

    std::string getValueStr(const ir::Value& value) {
        switch (value.getKind()) {
        case ir::ValueKind::Constant:
            return getConstantStr(value.getConstant(), value.getType());
        case ir::ValueKind::Local:
            return getLocalStr(value.getLocal());
        case ir::ValueKind::Register:
            return getRegisterStr(value.getRegister(), value.getType());
        case ir::ValueKind::Predicate:
            return getPredicateStr(value.getPredicate());
        case ir::ValueKind::AttrMemory:
            return getAttrMemoryStr(value.getAttrMemory(), value.getType());
        case ir::ValueKind::ConstMemory:
            return getConstMemoryStr(value.getConstMemory(), value.getType());
        default:
            LOG_FATAL(ShaderDecompiler, "Invalid value type {} for src",
                      value.getType());
        }
    }

    template <typename... T>
    void storeValue(const ir::Value& dst, WRITE_ARGS) {
        switch (dst.getKind()) {
        case ir::ValueKind::Local:
            // TODO: don't use auto
            writeStatement("auto {} = {}", getLocalStr(dst.getLocal()), FMT);
            break;
        case ir::ValueKind::Register:
            writeStatement(
                "{} = {}",
                getRegisterStr<false>(dst.getRegister(), dst.getType()), FMT);
            break;
        case ir::ValueKind::Predicate:
            writeStatement("{} = {}",
                           getPredicateStr<false>(dst.getPredicate()), FMT);
            break;
        case ir::ValueKind::AttrMemory:
            writeStatement("{} = {}",
                           getAttrMemoryStr(dst.getAttrMemory(), dst.getType()),
                           FMT);
            break;
        default:
            LOG_FATAL(ShaderDecompiler, "Invalid value type {} for dst",
                      dst.getType());
        }
    }

    static char getComponentStrFromIndex(u8 component_index) {
        ASSERT_DEBUG(component_index < 4, ShaderDecompiler,
                     "Invalid component index {}", component_index);

        return "xyzw"[component_index];
    }

    static std::string_view getTypeSuffixStr(ir::Type type) {
        switch (type.getKind()) {
        case ir::TypeKind::Scalar: {
            switch (type.getScalarType()) {
            case ir::ScalarType::Bool:
                return "_bool";
            case ir::ScalarType::U8:
                return "_u8";
            case ir::ScalarType::U16:
                return "_u16";
            case ir::ScalarType::U32:
                return "_u32";
            case ir::ScalarType::I8:
                return "_i8";
            case ir::ScalarType::I16:
                return "_i16";
            case ir::ScalarType::I32:
                return "_i32";
            case ir::ScalarType::F16:
                return "_f16";
            case ir::ScalarType::F32:
                return "_f32";
            }
        }
        case ir::TypeKind::Vector: {
            const auto& vec_type = type.getVectorType();
            if (vec_type.getElementType() == ir::ScalarType::F16 &&
                vec_type.getSize() == 2) {
                return "_2xf16";
            } else {
                LOG_ERROR(ShaderDecompiler,
                          "Unsupported vector type for register ({})",
                          vec_type);
                return INVALID_VALUE;
            }
        }
        default:
            LOG_ERROR(ShaderDecompiler, "Unsupported type for register ({})",
                      type);
            return INVALID_VALUE;
        }
    }

    // template <typename... T>
    // std::string GetQualifiedSVName(const SV sv, bool output,
    // WRITE_ARGS)
    // {
    //     // TODO: support qualifiers before the name as well
    //     return fmt::format("{} {}", FMT, GetSVQualifierName(sv,
    //     output));
    // }

    virtual std::string getSvAccessQualifiedStr(const SvAccess& sv_access,
                                                bool output) = 0;

  private:
    std::string code_str;

    u32 indent{0};

    template <typename... T>
    void enterScopeImpl(WRITE_ARGS) {
        write("{}{{", FMT);
        indent++;
    }

    template <typename... T>
    void exitScopeImpl(WRITE_ARGS) {
        ASSERT_DEBUG(indent != 0, ShaderDecompiler,
                     "Cannot exit scope when indentation is 0");
        indent--;
        write("}}{}", FMT);
    }
};

} // namespace
  // hydra::hw::tegra_x1::gpu::renderer::shader_decomp::codegen::lang

#undef FMT
#undef WRITE_ARGS
