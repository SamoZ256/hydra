#include "core/hw/tegra_x1/gpu/renderer/shader_decompiler/lang/lang_builder_base.hpp"

#include "core/hw/tegra_x1/gpu/renderer/shader_cache.hpp"
#include "core/hw/tegra_x1/gpu/renderer/shader_decompiler/analyzer/memory_analyzer.hpp"
#include "core/hw/tegra_x1/gpu/renderer/shader_decompiler/const.hpp"

#define V(value) static_cast<Value*>(value)->str

#define RET_V(f, ...) return new Value(fmt::format(f PASS_VA_ARGS(__VA_ARGS__)))

namespace hydra::hw::tegra_x1::gpu::renderer::shader_decomp::Lang {

namespace {

std::string cmp_op_to_str(ComparisonOperator cmp) {
    // TODO: are the U ones the same?
    switch (cmp) {
    case ComparisonOperator::F:
        return INVALID_VALUE; // TODO
    case ComparisonOperator::Less:
    case ComparisonOperator::LessU:
        return "<";
    case ComparisonOperator::Equal:
    case ComparisonOperator::EqualU:
        return "==";
    case ComparisonOperator::LessEqual:
    case ComparisonOperator::LessEqualU:
        return "<=";
    case ComparisonOperator::Greater:
    case ComparisonOperator::GreaterU:
        return ">";
    case ComparisonOperator::NotEqual:
    case ComparisonOperator::NotEqualU:
        return "!=";
    case ComparisonOperator::GreaterEqual:
    case ComparisonOperator::GreaterEqualU:
        return ">=";
    case ComparisonOperator::Num:
        return INVALID_VALUE; // TODO
    case ComparisonOperator::Nan:
        return INVALID_VALUE; // TODO
    case ComparisonOperator::T:
        return INVALID_VALUE; // TODO
    default:
        return INVALID_VALUE;
    }
}

std::string bin_op_to_str(BinaryOperator bin) {
    switch (bin) {
    case BinaryOperator::And:
        return "&";
    case BinaryOperator::Or:
        return "|";
    case BinaryOperator::Xor:
        return "^";
    default:
        return INVALID_VALUE;
    }
}

} // namespace

void LangBuilderBase::Start() {
    // Header
    EmitHeader();
    WriteNewline();

    // Type aliases
    EmitTypeAliases();
    WriteNewline();

    // Reg type
    EnterScope("union Reg");
    Write("u8 _u8;");
    Write("u16 _u16;");
    Write("u32 _u32;");
    Write("i8 _i8;");
    Write("i16 _i16;");
    Write("i32 _i32;");
    Write("f16 _f16;");
    Write("f32 _f32;");
    ExitScopeEmpty(true);
    WriteNewline();

    // Declarations
    EmitDeclarations();

    // Main prototype
    EmitMainPrototype();

    // Temporary
    EnterScope("union");
    Write("int4 i;");
    Write("uint4 u;");
    Write("float4 f;");
    ExitScope("temp");
    WriteNewline();

    // Registers
    Write("Reg r[256];");
    WriteNewline();

    // Predicates
    Write("bool p[8];"); // TODO: is the size correct?
    WriteNewline();

    // Attribute memory
    Write("Reg a[0x200];"); // TODO: what should the size be?
    WriteNewline();

    // Inputs
    switch (type) {
    case ShaderType::Vertex:
        for (u32 i = 0; i < VERTEX_ATTRIB_COUNT; i++) {
            const auto vertex_attrib_state = state.vertex_attrib_states[i];
            if (vertex_attrib_state.type == engines::VertexAttribType::None)
                continue;

            // HACK: how are attributes disabled?
            if (vertex_attrib_state.is_fixed)
                continue;

            // TODO: only set if the Rendered backend doesn't support scaled
            // attributes
            bool needs_scaling = (vertex_attrib_state.type ==
                                      engines::VertexAttribType::Sscaled ||
                                  vertex_attrib_state.type ==
                                      engines::VertexAttribType::Uscaled);

            const auto sv = Sv(SvSemantic::UserInOut, i);
            for (u32 c = 0; c < 4; c++) {
                const auto attr =
                    GetAttributeMemory(false, {RZ, 0x80 + i * 0x10 + c * 0x4});
                const auto qualified_name =
                    GetSvAccessNameQualified(SvAccess(sv, c), false);
                if (needs_scaling)
                    WriteStatement("{} = as_type<uint>((float){})", attr,
                                   qualified_name);
                else
                    WriteStatement("{} = as_type<uint>({})", attr,
                                   qualified_name);
            }
        }
        break;
    case ShaderType::Fragment:
#define ADD_INPUT(sv_semantic, index, a_base)                                  \
    {                                                                          \
        for (u32 c = 0; c < 4; c++) {                                          \
            WriteStatement("{} = as_type<uint>({})",                           \
                           GetAttributeMemory(false, {RZ, a_base + c * 0x4}),  \
                           GetSvAccessNameQualified(                           \
                               SvAccess(Sv(sv_semantic, index), c), false));   \
        }                                                                      \
    }

        ADD_INPUT(SvSemantic::Position, invalid<u8>(), 0x70);
        for (const auto input : memory_analyzer.GetStageInputs())
            ADD_INPUT(SvSemantic::UserInOut, input, 0x80 + input * 0x10);

#undef ADD_INPUT
        break;
    default:
        break;
    }
    WriteNewline();

    // Constant memory
    Write("Reg c[{}][0x40];",
          CONST_BUFFER_BINDING_COUNT); // TODO: what should the size be?
    WriteNewline();

    // Uniform buffers
    for (const auto& [index, size] : memory_analyzer.GetUniformBuffers()) {
        u32 u32_count = size / sizeof(u32);
        for (u32 i = 0; i < u32_count; i++)
            WriteStatement("{} = ubuff{}.data[{}]",
                           GetConstMemoryL({index, RZ, i * sizeof(u32)}), index,
                           i);
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

void LangBuilderBase::SetPredCond(const PredCond pred_cond) {
    EnterScopeTemp("if ({}{})", pred_cond.not_ ? "!" : "",
                   GetPredicate(true, pred_cond.pred));
}

#define RET_V_WITH_NEG(s)                                                      \
    {                                                                          \
        auto str = s;                                                          \
        if (neg)                                                               \
            str = fmt::format("(-{})", str);                                   \
        return new Value(str);                                                 \
    }

ValueBase* LangBuilderBase::OpImmediateL(u32 imm, DataType data_type,
                                         bool neg) {
    RET_V_WITH_NEG(GetImmediateL(imm, data_type));
}

ValueBase* LangBuilderBase::OpRegister(bool load, reg_t reg, DataType data_type,
                                       bool neg) {
    RET_V_WITH_NEG(GetRegister(load, reg, data_type));
}

ValueBase* LangBuilderBase::OpPredicate(bool load, pred_t pred, bool not_) {
    RET_V("{}{}", not_ ? "!" : "", GetPredicate(load, pred));
}

ValueBase* LangBuilderBase::OpAttributeMemory(bool load, const AMem& amem,
                                              DataType data_type, bool neg) {
    RET_V_WITH_NEG(GetAttributeMemory(load, amem, data_type));
}

ValueBase* LangBuilderBase::OpConstMemoryL(const CMem& cmem, DataType data_type,
                                           bool neg) {
    RET_V_WITH_NEG(GetConstMemoryL(cmem, data_type));
}

void LangBuilderBase::OpMove(ValueBase* dst, ValueBase* src) {
    WriteStatement("{} = {}", V(dst), V(src));
}

ValueBase* LangBuilderBase::OpAdd(ValueBase* srcA, ValueBase* srcB) {
    RET_V("({} + {})", V(srcA), V(srcB));
}

ValueBase* LangBuilderBase::OpMultiply(ValueBase* srcA, ValueBase* srcB) {
    RET_V("({} * {})", V(srcA), V(srcB));
}

ValueBase* LangBuilderBase::OpFloatFma(ValueBase* srcA, ValueBase* srcB,
                                       ValueBase* srcC) {
    RET_V("({} * {} + {})", V(srcA), V(srcB), V(srcC));
}

ValueBase* LangBuilderBase::OpShiftLeft(ValueBase* src, u32 shift) {
    RET_V("({} << 0x{:x})", V(src), shift);
}

ValueBase* LangBuilderBase::OpCast(ValueBase* src, DataType dst_type) {
    RET_V("({}){}", dst_type, V(src));
}

ValueBase* LangBuilderBase::OpCompare(ComparisonOperator cmp, ValueBase* srcA,
                                      ValueBase* srcB) {
    RET_V("({} {} {})", V(srcA), cmp_op_to_str(cmp), V(srcB));
}

ValueBase* LangBuilderBase::OpBinary(BinaryOperator bin, ValueBase* srcA,
                                     ValueBase* srcB) {
    RET_V("({} {} {})", V(srcA), bin_op_to_str(bin), V(srcB));
}

ValueBase* LangBuilderBase::OpSelect(ValueBase* cond, ValueBase* src_true,
                                     ValueBase* src_false) {
    RET_V("({} ? {} : {})", V(cond), V(src_true), V(src_false));
}

void LangBuilderBase::OpExit() {
    // Outputs
    switch (type) {
    case ShaderType::Vertex:
        // TODO: don't hardcode the bit cast type
#define ADD_OUTPUT(sv_semantic, index, a_base)                                 \
    {                                                                          \
        for (u32 c = 0; c < 4; c++) {                                          \
            WriteStatement("{} = as_type<float>({})",                          \
                           GetSvAccessNameQualified(                           \
                               SvAccess(Sv(sv_semantic, index), c), true),     \
                           GetAttributeMemory(true, {RZ, a_base + c * 0x4}));  \
        }                                                                      \
    }

        ADD_OUTPUT(SvSemantic::Position, invalid<u8>(), 0x70);
        for (const auto output : memory_analyzer.GetStageOutputs())
            ADD_OUTPUT(SvSemantic::UserInOut, output, 0x80 + output * 0x10);

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
                    GetSvAccessNameQualified(
                        SvAccess(Sv(SvSemantic::UserInOut, i), c), true),
                    to_data_type(color_target_format),
                    GetRegister(true, i * 4 + c));
            }
        }
        break;
    default:
        break;
    }
    WriteNewline();

    EmitExit();
}

ValueBase* LangBuilderBase::OpMin(ValueBase* srcA, ValueBase* srcB) {
    RET_V("min({}, {})", V(srcA), V(srcB));
}

ValueBase* LangBuilderBase::OpMax(ValueBase* srcA, ValueBase* srcB) {
    RET_V("max({}, {})", V(srcA), V(srcB));
}

ValueBase* LangBuilderBase::OpMathFunction(MathFunc func, ValueBase* src) {
    RET_V("({}({}))", GetMathFunc(func), V(src));
}

ValueBase* LangBuilderBase::OpInterpolate(ValueBase* src) {
    // TODO: interpolate param
    RET_V("{}", V(src));
}

void LangBuilderBase::OpTextureSample(ValueBase* dstA, ValueBase* dstB,
                                      ValueBase* dstC, ValueBase* dstD,
                                      u32 const_buffer_index,
                                      ValueBase* coords_x,
                                      ValueBase* coords_y) {
    EmitReadToTemp(coords_x, 0);
    EmitReadToTemp(coords_y, 1);
    WriteStatement("temp.f = {}",
                   EmitTextureSample(const_buffer_index, "temp.f.xy"));
    EmitWriteFromTemp(dstA, 0);
    EmitWriteFromTemp(dstB, 1);
    EmitWriteFromTemp(dstC, 2);
    EmitWriteFromTemp(dstD, 3);
}

void LangBuilderBase::EmitReadToTemp(ValueBase* src, u32 offset) {
    WriteStatement("temp.u.{} = {}", GetComponentFromIndex(offset), V(src));
}

void LangBuilderBase::EmitWriteFromTemp(ValueBase* dst, u32 offset) {
    WriteStatement("{} = temp.u.{}", V(dst), GetComponentFromIndex(offset));
}

} // namespace hydra::hw::tegra_x1::gpu::renderer::shader_decomp::Lang

#undef RET_V
