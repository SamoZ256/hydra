#include "core/hw/tegra_x1/gpu/renderer/shader_decompiler/codegen/lang/emitter.hpp"

#include "core/hw/tegra_x1/gpu/renderer/shader_cache.hpp"
#include "core/hw/tegra_x1/gpu/renderer/shader_decompiler/analyzer/memory_analyzer.hpp"

namespace hydra::hw::tegra_x1::gpu::renderer::shader_decomp::codegen::lang {

namespace {

// TODO: move this to the decoder
std::string cmp_to_str(ComparisonOp cmp, std::string_view lhs,
                       std::string_view rhs) {
    // TODO: are the U ones the same?
    switch (cmp) {
    case ComparisonOp::F:
        return "false";
    case ComparisonOp::Less:
    case ComparisonOp::LessU:
        return fmt::format("{} < {}", lhs, rhs);
    case ComparisonOp::Equal:
    case ComparisonOp::EqualU:
        return fmt::format("{} == {}", lhs, rhs);
    case ComparisonOp::LessEqual:
    case ComparisonOp::LessEqualU:
        return fmt::format("{} <= {}", lhs, rhs);
    case ComparisonOp::Greater:
    case ComparisonOp::GreaterU:
        return fmt::format("{} > {}", lhs, rhs);
    case ComparisonOp::NotEqual:
    case ComparisonOp::NotEqualU:
        return fmt::format("{} != {}", lhs, rhs);
    case ComparisonOp::GreaterEqual:
    case ComparisonOp::GreaterEqualU:
        return fmt::format("{} >= {}", lhs, rhs);
    case ComparisonOp::Num:
        return fmt::format("!(isnan({}) || isnan({}))", lhs, rhs);
    case ComparisonOp::Nan:
        return fmt::format("isnan({}) || isnan({})", lhs, rhs);
    case ComparisonOp::T:
        return "true";
    default:
        return INVALID_VALUE;
    }
}

std::string bit_op_to_str(BitwiseOp bin) {
    switch (bin) {
    case BitwiseOp::And:
        return "&";
    case BitwiseOp::Or:
        return "|";
    case BitwiseOp::Xor:
        return "^";
    default:
        return INVALID_VALUE;
    }
}

} // namespace

void LangEmitter::Start() {
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
    Write("half2 _f16x2;");
    ExitScopeEmpty(true);
    WriteNewline();

    // State
    EnterScope("struct State");
    Write("Reg r[256];");
    Write("bool p[8];"); // TODO: is the size correct?
    Write("Reg c[{}][0x40];",
          CONST_BUFFER_BINDING_COUNT); // TODO: what should the size be?
    Write("Reg a_in[0x200];");         // TODO: what should the size be?
    Write("Reg a_out[0x200];");        // TODO: what should the size be?
    EmitStateBindings();
    ExitScopeEmpty(true);
    WriteNewline();

    // Declarations
    EmitDeclarations();
}

void LangEmitter::Finish() {
    ASSERT_DEBUG(indent == 0, ShaderDecompiler,
                 "Scope not fully exited (indentation: {})", indent);

    // TODO: footer?

    // TODO: avoid copying
    out_code.resize(code_str.size());
    std::copy(code_str.begin(), code_str.end(), out_code.begin());

    // Debug
    LOG_DEBUG(ShaderDecompiler, "decompiled: \"\n{}\"", code_str);
}

void LangEmitter::EmitMainFunctionPrologue() {
    // State
    WriteStatement("State state");

    // Inputs
    // TODO: these are provided in the shader header, no need for analysis

#define ADD_INPUT(sv_semantic, index, base, c)                                 \
    {                                                                          \
        WriteStatement("{} = as_type<uint>({})",                               \
                       GetAttrMemoryStr({RZ, base + c * 0x4, true}),           \
                       GetSvAccessQualifiedStr(                                \
                           SvAccess(Sv(sv_semantic, index), c), false));       \
    }
#define ADD_INPUT_1(sv_semantic, index, base)                                  \
    ADD_INPUT(sv_semantic, index, base, 0)
#define ADD_INPUT_VEC4(sv_semantic, index, base)                               \
    {                                                                          \
        for (u32 c = 0; c < 4; c++) {                                          \
            ADD_INPUT(sv_semantic, index, base, c);                            \
        }                                                                      \
    }

    switch (context.type) {
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
                    GetAttrMemoryStr({RZ, 0x80 + i * 0x10 + c * 0x4, true});
                const auto qualified_name =
                    GetSvAccessQualifiedStr(SvAccess(sv, c), false);
                if (needs_scaling)
                    WriteStatement("{} = as_type<uint>((float){})", attr,
                                   qualified_name);
                else
                    WriteStatement("{} = as_type<uint>({})", attr,
                                   qualified_name);
            }
        }

        ADD_INPUT_1(SvSemantic::InstanceID, invalid<u8>(), SV_INSTANCE_ID_BASE);
        ADD_INPUT_1(SvSemantic::VertexID, invalid<u8>(), SV_VERTEX_ID_BASE);

        break;
    case ShaderType::Fragment:
        ADD_INPUT_VEC4(SvSemantic::Position, invalid<u8>(), SV_POSITION_BASE);
        for (const auto input : memory_analyzer.GetStageInputs())
            ADD_INPUT_VEC4(SvSemantic::UserInOut, input,
                           SV_USER_IN_OUT_BASE + input * 0x10);

        break;
    default:
        break;
    }

#undef ADD_INPUT

    WriteNewline();

    // Constant memory

    // Uniform buffers
    for (const auto& [index, size] : memory_analyzer.GetUniformBuffers()) {
        u32 u32_count = size / sizeof(u32);
        for (u32 i = 0; i < u32_count; i++)
            WriteStatement("{} = ubuff{}.data[{}]",
                           GetConstMemoryStr({index, RZ, i * sizeof(u32)}),
                           index, i);
    }
    WriteNewline();

    EmitStateBindingAssignments();
}

void LangEmitter::EmitFunction(const ir::Function& func) {
    // Block enum
    EnterScope("enum class Block_{}", func.GetName());
    Write("None = -1,");
    for (const auto& [label, block] : func.GetBlocks()) {
        Write("{} = {},", label, u32(label));
    }
    ExitScopeEmpty(true);
    WriteNewline();

    // Blocks
    for (const auto& [_, block] : func.GetBlocks())
        EmitBlock(func, block);

    // Function
    // TODO: function name
    std::string name = "main";
    if (name == "main")
        EmitMainPrototype();
    else
        LOG_FATAL(ShaderDecompiler,
                  "Custom functions not implemented (name: {})", name);

    // Structurize
    /*
    analyzer::CfgBuilder cfg_builder;
    auto entry_block = cfg_builder.Build(func);
    auto entry_node = analyzer::Structurize(entry_block);
    */

    // Caller loop
    WriteStatement("auto next = Block_{}::{}", func.GetName(), label_t(0x0));
    EnterScope("while (next != Block_{}::None)", func.GetName());
    EnterScope("switch (next)");
    for (const auto& [label, block] : func.GetBlocks()) {
        indent--;
        Write("case Block_{}::{}:", func.GetName(), label);
        indent++;
        WriteStatement("next = func_{}(state)", label);
        WriteStatement("break");
    }
    ExitScopeEmpty();
    ExitScopeEmpty();
    WriteNewline();

    // Exit
    // Outputs
    switch (context.type) {
    case ShaderType::Vertex:
        // TODO: don't hardcode the bit cast type
#define ADD_OUTPUT(sv_semantic, index, base, c)                                \
    {                                                                          \
        WriteStatement("{} = as_type<float>({})",                              \
                       GetSvAccessQualifiedStr(                                \
                           SvAccess(Sv(sv_semantic, index), c), true),         \
                       GetAttrMemoryStr({RZ, base + c * 0x4, false}));         \
    }
#define ADD_OUTPUT_1(sv_semantic, index, base)                                 \
    ADD_OUTPUT(sv_semantic, index, base, 0)
#define ADD_OUTPUT_VEC4(sv_semantic, index, base)                              \
    {                                                                          \
        for (u32 c = 0; c < 4; c++) {                                          \
            ADD_OUTPUT(sv_semantic, index, base, c);                           \
        }                                                                      \
    }

        ADD_OUTPUT_VEC4(SvSemantic::Position, invalid<u8>(), SV_POSITION_BASE);
        for (const auto output : memory_analyzer.GetStageOutputs())
            ADD_OUTPUT_VEC4(SvSemantic::UserInOut, output,
                            SV_USER_IN_OUT_BASE + output * 0x10);

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
                    GetSvAccessQualifiedStr(
                        SvAccess(Sv(SvSemantic::UserInOut, i), c), true),
                    to_data_type(color_target_format),
                    GetRegisterStr(i * 4 + c));
            }
        }
        break;
    default:
        break;
    }
    WriteNewline();

    EmitExitReturn();

    // Emit
    // EmitNode(func, entry_node);
    ExitScopeEmpty();
}

/*
void LangEmitter::EmitNode(const ir::Function& func,
                           const analyzer::CfgNode* node) {
    if (auto code_block = dynamic_cast<const analyzer::CfgCodeBlock*>(node)) {
        auto& block = func.GetBlock(code_block->label);
        for (u32 i = 0; i < block.GetInstructions().size() - 1;
             i++) // Skip last instruction
            EmitInstruction(block.GetInstructions()[i]);
    } else if (auto last_statement =
                   dynamic_cast<const analyzer::CfgLastStatement*>(node)) {
        switch (last_statement->last_statement) {
        case analyzer::LastStatement::Exit:
            EmitExit();
            break;
        case analyzer::LastStatement::Break:
            WriteStatement("break");
            break;
        case analyzer::LastStatement::Continue:
            WriteStatement("continue");
            break;
        }
    } else if (auto block = dynamic_cast<const analyzer::CfgBlock*>(node)) {
        for (const auto block_node : block->nodes)
            EmitNode(func, block_node);
    } else if (auto if_block =
                   dynamic_cast<const analyzer::CfgIfBlock*>(node)) {
        // If
        EnterScope("if ({})", GetValueStr(if_block->cond));
        EmitNode(func, if_block->then_block);
        ExitScopeEmpty();
    } else if (auto if_else_block =
                   dynamic_cast<const analyzer::CfgIfElseBlock*>(node)) {
        // If
        EnterScope("if ({})", GetValueStr(if_else_block->cond));
        EmitNode(func, if_else_block->then_block);
        ExitScopeEmpty();

        // Else
        EnterScope("else");
        EmitNode(func, if_else_block->else_block);
        ExitScopeEmpty();
    } else if (auto while_block =
                   dynamic_cast<const analyzer::CfgWhileBlock*>(node)) {
        // While
        if (!while_block->IsDoWhile()) {
            EnterScope("while ({})", GetValueStr(if_block->cond));
            EmitNode(func, while_block->body_block);
            ExitScopeEmpty();
        } else {
            EnterScope("do");
            EmitNode(func, while_block->body_block);
            ExitScope("while ({})", GetValueStr(if_block->cond));
        }
    } else {
        LOG_ERROR(ShaderDecompiler, "Invalid structured node");
    }
}
*/

void LangEmitter::EmitBlock(const ir::Function& func, const ir::Block& block) {
    EnterScope("Block_{} func_{}(thread State& state)", func.GetName(),
               block.GetLabel());

    // Block enum alias
    WriteStatement("using Block = Block_{}", func.GetName());
    WriteNewline();

    // Temporary
    EnterScope("union");
    Write("int4 i;");
    Write("uint4 u;");
    Write("float4 f;");
    ExitScope("temp");
    WriteNewline();

    for (const auto& instruction : block.GetInstructions())
        EmitInstruction(instruction);
    ExitScopeEmpty();
}

// Basic
void LangEmitter::EmitCopy(const ir::Value& dst, const ir::Value& src) {
    StoreValue(dst, "{}", GetValueStr(src));
}

void LangEmitter::EmitNeg(const ir::Value& dst, const ir::Value& src) {
    StoreValue(dst, "(-{})", GetValueStr(src));
}

void LangEmitter::EmitNot(const ir::Value& dst, const ir::Value& src) {
    StoreValue(dst, "(!{})", GetValueStr(src));
}

void LangEmitter::EmitAdd(const ir::Value& dst, const ir::Value& srcA,
                          const ir::Value& srcB) {
    StoreValue(dst, "({} + {})", GetValueStr(srcA), GetValueStr(srcB));
}

void LangEmitter::EmitMultiply(const ir::Value& dst, const ir::Value& srcA,
                               const ir::Value& srcB) {
    StoreValue(dst, "({} * {})", GetValueStr(srcA), GetValueStr(srcB));
}

void LangEmitter::EmitFma(const ir::Value& dst, const ir::Value& srcA,
                          const ir::Value& srcB, const ir::Value& srcC) {
    StoreValue(dst, "fma({}, {}, {})", GetValueStr(srcA), GetValueStr(srcB),
               GetValueStr(srcC));
}

void LangEmitter::EmitShiftLeft(const ir::Value& dst, const ir::Value& src,
                                u32 shift) {
    StoreValue(dst, "({} << {})", GetValueStr(src), shift);
}

void LangEmitter::EmitShiftRight(const ir::Value& dst, const ir::Value& src,
                                 u32 shift) {
    StoreValue(dst, "({} >> {})", GetValueStr(src), shift);
}

void LangEmitter::EmitCast(const ir::Value& dst, const ir::Value& src,
                           DataType dst_type) {
    StoreValue(dst, "({}({}))", dst_type, GetValueStr(src));
}

void LangEmitter::EmitCompare(const ir::Value& dst, ComparisonOp op,
                              const ir::Value& srcA, const ir::Value& srcB) {
    StoreValue(dst, "({})",
               cmp_to_str(op, GetValueStr(srcA), GetValueStr(srcB)));
}

void LangEmitter::EmitBitwise(const ir::Value& dst, BitwiseOp op,
                              const ir::Value& srcA, const ir::Value& srcB) {
    StoreValue(dst, "({} {} {})", GetValueStr(srcA), bit_op_to_str(op),
               GetValueStr(srcB));
}

void LangEmitter::EmitSelect(const ir::Value& dst, const ir::Value& cond,
                             const ir::Value& src_true,
                             const ir::Value& src_false) {
    StoreValue(dst, "({} ? {} : {})", GetValueStr(cond), GetValueStr(src_true),
               GetValueStr(src_false));
}

// Control flow
void LangEmitter::EmitBranch(label_t target) {
    // LOG_FATAL(ShaderDecompiler, "Should not happen");
    WriteStatement("return Block::{}", target);
}

void LangEmitter::EmitBranchConditional(const ir::Value& cond,
                                        label_t target_true,
                                        label_t target_false) {
    // LOG_FATAL(ShaderDecompiler, "Should not happen");
    EnterScope("if ({})", GetValueStr(cond));
    WriteStatement("return Block::{}", target_true);
    ExitScopeEmpty();
    EnterScope("else");
    WriteStatement("return Block::{}", target_false);
    ExitScopeEmpty();
}

void LangEmitter::EmitBeginIf(const ir::Value& cond) {
    EnterScope("if ({})", GetValueStr(cond));
}

void LangEmitter::EmitEndIf() { ExitScopeEmpty(); }

// Math
void LangEmitter::EmitRound(const ir::Value& dst, const ir::Value& src) {
    StoreValue(dst, "round({})", GetValueStr(src));
}

void LangEmitter::EmitFloor(const ir::Value& dst, const ir::Value& src) {
    StoreValue(dst, "floor({})", GetValueStr(src));
}

void LangEmitter::EmitCeil(const ir::Value& dst, const ir::Value& src) {
    StoreValue(dst, "ceil({})", GetValueStr(src));
}

void LangEmitter::EmitTrunc(const ir::Value& dst, const ir::Value& src) {
    StoreValue(dst, "trunc({})", GetValueStr(src));
}

void LangEmitter::EmitMin(const ir::Value& dst, const ir::Value& srcA,
                          const ir::Value& srcB) {
    StoreValue(dst, "min({}, {})", GetValueStr(srcA), GetValueStr(srcB));
}

void LangEmitter::EmitMax(const ir::Value& dst, const ir::Value& srcA,
                          const ir::Value& srcB) {
    StoreValue(dst, "max({}, {})", GetValueStr(srcA), GetValueStr(srcB));
}

void LangEmitter::EmitClamp(const ir::Value& dst, const ir::Value& srcA,
                            const ir::Value& srcB, const ir::Value& srcC) {
    StoreValue(dst, "clamp({}, {}, {})", GetValueStr(srcA), GetValueStr(srcB),
               GetValueStr(srcC));
}

void LangEmitter::EmitMathFunction(const ir::Value& dst, MathFunc func,
                                   const ir::Value& src) {
    StoreValue(dst, "({}({}))", GetMathFuncStr(func), GetValueStr(src));
}

// Vector
void LangEmitter::EmitVectorExtract(const ir::Value& dst, const ir::Value& src,
                                    u32 index) {
    StoreValue(dst, "({}.{})", GetValueStr(src),
               GetComponentStrFromIndex(index));
}

void LangEmitter::EmitVectorInsert(const ir::Value& dst, const ir::Value& src,
                                   u32 index) {
    WriteStatement("{}.{} = {}", GetValueStr(dst),
                   GetComponentStrFromIndex(index), GetValueStr(src));
}

void LangEmitter::EmitVectorConstruct(const ir::Value& dst, DataType data_type,
                                      const std::vector<ir::Value>& elements) {
    std::string str;
    for (u32 i = 0; i < elements.size(); i++) {
        if (i != 0)
            str += ", ";
        str += GetValueStr(elements[i]);
    }
    StoreValue(dst, "vec<{}, {}>({})", data_type, elements.size(), str);
}

// Special
void LangEmitter::EmitExit() { WriteStatement("return Block::None"); }

} // namespace hydra::hw::tegra_x1::gpu::renderer::shader_decomp::codegen::lang
