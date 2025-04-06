#include "hw/tegra_x1/gpu/renderer/shader_decompiler/lang/lang_builder_base.hpp"

#include "hw/tegra_x1/gpu/renderer/shader_cache.hpp"
#include "hw/tegra_x1/gpu/renderer/shader_decompiler/analyzer.hpp"

namespace Hydra::HW::TegraX1::GPU::Renderer::ShaderDecompiler::Lang {

void LangBuilderBase::Start() {
    InitializeResourceMapping();

    // Header
    EmitHeader();
    WriteNewline();

    // Type aliases
    EmitTypeAliases();
    WriteNewline();

    // Reg type
    EnterScope("union Reg");
    Write("int i;");
    Write("uint u;");
    Write("float f;");
    ExitScopeEmpty(true);
    WriteNewline();

    // Declarations
    EmitDeclarations();

    // Main prototype
    EmitMainPrototype();

    // Output
    Write("StageOut __out;");
    WriteNewline();

    // Registers
    Write("Reg r[256];");
    WriteNewline();

    // Temporary
    EnterScope("union");
    Write("int4 i;");
    Write("uint4 u;");
    Write("float4 f;");
    ExitScope("temp");
    WriteNewline();

    // Attribute memory
    Write("Reg a[0x200];"); // TODO: what should the size be?
    WriteNewline();

    // Inputs
    switch (type) {
    case ShaderType::Vertex:
        for (u32 i = 0; i < VERTEX_ATTRIB_COUNT; i++) {
            const auto vertex_attrib_state = state.vertex_attrib_states[i];
            if (vertex_attrib_state.type == Engines::VertexAttribType::None)
                continue;

            // HACK: how are attributes disabled?
            if (vertex_attrib_state.is_fixed)
                continue;

            // TODO: only set if the Rendered backend doesn't support scaled
            // attributes
            bool needs_scaling = (vertex_attrib_state.type ==
                                      Engines::VertexAttribType::Sscaled ||
                                  vertex_attrib_state.type ==
                                      Engines::VertexAttribType::Uscaled);

            const auto sv = SV(SVSemantic::UserInOut, i);
            for (u32 c = 0; c < 4; c++) {
                WriteStatement(
                    (needs_scaling ? "{} = as_type<uint>((float){})"
                                   : "{} = as_type<uint>({})"),
                    GetA({RZ, 0x80 + i * 0x10 + c * 0x4}),
                    GetSVNameQualified(SV(SVSemantic::UserInOut, i, c), false));
            }
        }
        break;
    case ShaderType::Fragment:
#define ADD_INPUT(sv_semantic, index, a_base)                                  \
    {                                                                          \
        for (u32 c = 0; c < 4; c++) {                                          \
            WriteStatement(                                                    \
                "{} = as_type<uint>({})", GetA({RZ, a_base + c * 0x4}),        \
                GetSVNameQualified(SV(sv_semantic, index, c), false));         \
        }                                                                      \
    }

        ADD_INPUT(SVSemantic::Position, invalid<u8>(), 0x70);
        for (const auto input : analyzer.GetStageInputs())
            ADD_INPUT(SVSemantic::UserInOut, input, 0x80 + input * 0x10);

#undef ADD_INPUT
        break;
    default:
        break;
    }
    WriteNewline();

    // Constant memory
    Write("Reg c[{}][0x40];",
          UNIFORM_BUFFER_BINDING_COUNT); // TODO: what should the size be?
    WriteNewline();

    // Uniform buffers
    for (const auto& [index, size] : analyzer.GetUniformBuffers()) {
        u32 u32_count = size / sizeof(u32);
        for (u32 i = 0; i < u32_count; i++)
            WriteStatement("{} = ubuff{}.data[{}]",
                           GetC({index, RZ, i * sizeof(u32)}), index, i);
    }
    WriteNewline();
}

void LangBuilderBase::Finish() {
    // End
    ExitScopeEmpty();
    ASSERT_DEBUG(indent == 0, ShaderDecompiler,
                 "Scope not fully exited (indentation: {})", indent);

    // TODO: footer

    // TODO: avoid copying
    out_code.resize(code_str.size());
    std::copy(code_str.begin(), code_str.end(), out_code.begin());

    // Debug
    LOG_DEBUG(ShaderDecompiler, "Decompiled: \"\n{}\"", code_str);
}

void LangBuilderBase::OpExit() {
    // Outputs
    switch (type) {
    case ShaderType::Vertex:
        // TODO: don't hardcode the bit cast type
#define ADD_OUTPUT(sv_semantic, index, a_base)                                 \
    {                                                                          \
        for (u32 c = 0; c < 4; c++) {                                          \
            WriteStatement(                                                    \
                "{} = as_type<float>({})",                                     \
                GetSVNameQualified(SV(sv_semantic, index, c), true),           \
                GetA({RZ, a_base + c * 0x4}));                                 \
        }                                                                      \
    }

        ADD_OUTPUT(SVSemantic::Position, invalid<u8>(), 0x70);
        for (const auto output : analyzer.GetStageOutputs())
            ADD_OUTPUT(SVSemantic::UserInOut, output, 0x80 + output * 0x10);

#undef ADD_OUTPUT
        break;
    case ShaderType::Fragment:
        for (u32 i = 0; i < COLOR_TARGET_COUNT; i++) {
            const auto color_target_format = state.color_target_formats[i];
            if (color_target_format == TextureFormat::Invalid)
                continue;

            for (u32 c = 0; c < 4; c++) {
                WriteStatement(
                    "{} = as_type<{}>({})",
                    GetSVNameQualified(SV(SVSemantic::UserInOut, i, c), true),
                    to_data_type(color_target_format),
                    GetReg(i * 4 + c, false));
            }
        }
        break;
    default:
        break;
    }
    WriteNewline();

    // Return
    WriteStatement("return __out");
}

void LangBuilderBase::OpMove(reg_t dst, Operand src) {
    WriteStatement("{} = {}", GetReg(dst, true), GetOperand(src, false));
}

void LangBuilderBase::OpFloatAdd(reg_t dst, reg_t src1, Operand src2) {
    WriteStatement("{} = {} + {}", GetReg(dst, true, DataType::Float),
                   GetReg(src1, false, DataType::Float),
                   GetOperand(src2, false, DataType::Float));
}

void LangBuilderBase::OpFloatMultiply(reg_t dst, reg_t src1, Operand src2) {
    WriteStatement("{} = {} * {}", GetReg(dst, true, DataType::Float),
                   GetReg(src1, false, DataType::Float),
                   GetOperand(src2, false, DataType::Float));
}

void LangBuilderBase::OpFloatFma(reg_t dst, reg_t src1, Operand src2,
                                 reg_t src3) {
    WriteStatement("{} = {} * {} + {}", GetReg(dst, true, DataType::Float),
                   GetReg(src1, false, DataType::Float),
                   GetOperand(src2, false, DataType::Float),
                   GetReg(src3, false, DataType::Float));
}

void LangBuilderBase::OpShiftLeft(reg_t dst, reg_t src, u32 shift) {
    WriteStatement("{} = {} << 0x{:x}", GetReg(dst, true, DataType::UInt),
                   GetReg(src, false, DataType::UInt), shift);
}

void LangBuilderBase::OpMathFunction(MathFunc func, reg_t dst, reg_t src) {
    WriteStatement("{} = {}({})", GetReg(dst, true, DataType::Float),
                   GetMathFunc(func), GetReg(src, false, DataType::Float));
}

void LangBuilderBase::OpLoad(reg_t dst, Operand src) {
    WriteStatement("{} = {}", GetReg(dst, true), GetOperand(src));
}

void LangBuilderBase::OpStore(AMem dst, reg_t src) {
    WriteStatement("{} = {}", GetA(dst), GetReg(src, false));
}

void LangBuilderBase::OpInterpolate(reg_t dst, AMem src) {
    WriteStatement("{} = {}", GetReg(dst, true), GetA(src));
}

void LangBuilderBase::OpTextureSample(reg_t dst, u32 index, reg_t coords) {
    EmitReadToTemp(coords, 2);
    WriteStatement("temp.f = {}", EmitTextureSample(index, "temp.f.xy"));
    EmitWriteFromTemp(dst);
}

void LangBuilderBase::EmitReadToTemp(reg_t src, u32 count) {
    for (u32 i = 0; i < count; i++) {
        WriteStatement("temp.u.{} = {}", GetComponentFromIndex(i),
                       GetReg(src + i, false));
    }
}

void LangBuilderBase::EmitWriteFromTemp(reg_t dst, u32 count) {
    for (u32 i = 0; i < count; i++) {
        WriteStatement("{} = temp.u.{}", GetReg(dst + i, true),
                       GetComponentFromIndex(i));
    }
}

} // namespace Hydra::HW::TegraX1::GPU::Renderer::ShaderDecompiler::Lang
