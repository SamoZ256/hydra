#pragma once

#include "hw/tegra_x1/gpu/shader_decompiler/builder_base.hpp"
#include "hw/tegra_x1/gpu/shader_decompiler/const.hpp"

#define WRITE_ARGS fmt::format_string<T...> fmt, T &&... args
#define FMT fmt::format(fmt, std::forward<T>(args)...)

#define INVALID_VALUE "INVALID"

namespace Hydra::HW::TegraX1::GPU::ShaderDecompiler::Lang {

class LangBuilderBase : public BuilderBase {
  public:
    LangBuilderBase(std::vector<u8>& out_code_) : out_code{out_code_} {}

    void Start() override;
    void Finish() override;

    // Operations
    void OpMove(reg_t dst, u32 value) override;
    void OpLoad(reg_t dst, reg_t src, u64 imm) override;
    void OpStore(reg_t src, reg_t dst, u64 imm) override;

  protected:
    virtual void EmitHeader() = 0;
    virtual void EmitTypeAliases() = 0;

    template <typename... T> void Write(WRITE_ARGS) {
        // TODO: handle indentation differently
        std::string indent_str;
        for (u32 i = 0; i < indent; i++) {
            indent_str += "    ";
        }
        WriteRaw("{}{}\n", indent_str, FMT);
    }

    void WriteNewline() { code_str += "\n"; }
    template <typename... T> void WriteStatement(WRITE_ARGS) {
        Write("{};", FMT);
    }

    void EnterScopeEmpty() { EnterScopeImpl(""); }

    template <typename... T> void EnterScope(WRITE_ARGS) {
        EnterScopeImpl("{} ", FMT);
    }

    void ExitScopeEmpty() { ExitScopeImpl(""); }

    template <typename... T> void ExitScope(WRITE_ARGS) {
        ExitScopeImpl(" {};", FMT);
    }

    // Helpers
    std::string GetReg(reg_t reg, bool write = false,
                       DataType data_type = DataType::UInt) {
        if (reg == RZ && !write)
            return fmt::format("0{}", data_type);

        return fmt::format("r{}.{}", reg, data_type);
    }

    std::string GetComponentFromIndex(u8 component_index) {
        switch (component_index) {
        case 0:
            return "x";
        case 1:
            return "y";
        case 2:
            return "z";
        case 3:
            return "w";
        default:
            LOG_ERROR(ShaderDecompiler, "Invalid component index {}",
                      component_index);
            return INVALID_VALUE;
        }
    }

    const SV GetSVFromAddr(u64 addr) {
        SV sv;

        struct SVBase {
            SVSemantic semantic;
            u64 base_addr;
        };

        static constexpr SVBase bases[] = {
            {SVSemantic::UserInOut, 0x80},
            {SVSemantic::Position, 0x70},
        };

        for (const auto& base : bases) {
            if (addr >= base.base_addr) {
                sv.semantic = base.semantic;
                sv.index = (addr - base.base_addr) >> 4;
                sv.component_index = (addr >> 2) & 0x3;
                return sv;
            }
        }

        LOG_NOT_IMPLEMENTED(ShaderDecompiler, "SV address 0x{:02x}", addr);

        return {SVSemantic::Invalid};
    }

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
    std::vector<u8>& out_code;
    std::string code_str;

    u32 indent = 0;

    template <typename... T> void WriteRaw(WRITE_ARGS) { code_str += FMT; }

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
};

} // namespace Hydra::HW::TegraX1::GPU::ShaderDecompiler::Lang

#undef INVALID_VALUE
#undef FMT
#undef WRITE_ARGS
