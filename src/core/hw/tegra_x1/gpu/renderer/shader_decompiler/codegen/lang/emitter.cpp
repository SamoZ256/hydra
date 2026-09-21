#include "core/hw/tegra_x1/gpu/renderer/shader_decompiler/codegen/lang/emitter.hpp"

#include "core/hw/tegra_x1/gpu/renderer/shader_cache.hpp"
#include "core/hw/tegra_x1/gpu/renderer/shader_decompiler/analyzer/memory_analyzer.hpp"
#include "core/hw/tegra_x1/gpu/renderer/shader_decompiler/codegen/helper.hpp"

namespace hydra::hw::tegra_x1::gpu::renderer::shader_decomp::codegen::lang {

void LangEmitter::start() {
    // Header
    emitHeader();
    writeNewline();

    // Type aliases
    emitTypeAliases();
    writeNewline();

    // Reg type
    enterScope("union Reg");
    write("u8 _u8;");
    write("u16 _u16;");
    write("u32 _u32;");
    write("i8 _i8;");
    write("i16 _i16;");
    write("i32 _i32;");
    write("f16 _f16;");
    write("f32 _f32;");
    write("half2 _2xf16;");
    exitScopeEmpty(true);
    writeNewline();

    // State
    enterScope("struct State");
    write("Reg r[256];");
    write("bool p[8];"); // TODO: is the size correct?
    // TODO: move this to the backend
    for (auto index : memory_analyzer.getConstBuffers())
        write("constant Reg* c{};", index);
    write("Reg a_in[0x200];");  // TODO: what should the size be?
    write("Reg a_out[0x200];"); // TODO: what should the size be?
    emitStateBindings();
    exitScopeEmpty(true);
    writeNewline();

    // Declarations
    emitDeclarations();
}

void LangEmitter::finish() {
    ASSERT_DEBUG(indent == 0, ShaderDecompiler,
                 "Scope not fully exited (indentation: {})", indent);

    // TODO: footer?

    // TODO: avoid copying
    out_code.resize(code_str.size());
    std::ranges::copy(code_str, out_code.begin());

    // Debug
    LOG_DEBUG(ShaderDecompiler, "decompiled: \"\n{}\"", code_str);
}

void LangEmitter::emitMainFunctionPrologue() {
    // State
    writeStatement("State state");

    // Inputs
    // TODO: these are provided in the shader header, no need for analysis

#define ADD_INPUT(sv_semantic, index, base, c)                                 \
    {                                                                          \
        writeStatement("{} = as_type<uint>({})",                               \
                       getAttrMemoryStr({RZ, base + c * 0x4, true}),           \
                       getSvAccessQualifiedStr(                                \
                           SvAccess(Sv(sv_semantic, index), c), false));       \
    }
#define ADD_INPUT_1(sv_semantic, index, base)                                  \
    ADD_INPUT(sv_semantic, index, base, 0)
#define ADD_INPUT_VEC4(sv_semantic, index, base)                               \
    {                                                                          \
        for (u8 c = 0; c < 4; c++) {                                           \
            ADD_INPUT(sv_semantic, index, base, c);                            \
        }                                                                      \
    }

    switch (context.type) {
    case ShaderType::Vertex:
        for (u8 i = 0; i < VERTEX_ATTRIB_COUNT; i++) {
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
            for (u8 c = 0; c < 4; c++) {
                const auto attr =
                    getAttrMemoryStr({RZ, 0x80u + i * 0x10u + c * 0x4u, true});
                const auto qualified_name =
                    getSvAccessQualifiedStr(SvAccess(sv, c), false);
                if (needs_scaling)
                    writeStatement("{} = as_type<uint>((float){})", attr,
                                   qualified_name);
                else
                    writeStatement("{} = as_type<uint>({})", attr,
                                   qualified_name);
            }
        }

        ADD_INPUT_1(SvSemantic::InstanceID, invalid<u8>(), SV_INSTANCE_ID_BASE);
        ADD_INPUT_1(SvSemantic::VertexID, invalid<u8>(), SV_VERTEX_ID_BASE);

        break;
    case ShaderType::Fragment:
        ADD_INPUT_VEC4(SvSemantic::Position, invalid<u8>(), SV_POSITION_BASE);
        for (const auto input : memory_analyzer.getStageInputs())
            ADD_INPUT_VEC4(SvSemantic::UserInOut, input,
                           SV_USER_IN_OUT_BASE + input * 0x10);

        break;
    default:
        break;
    }

#undef ADD_INPUT

    writeNewline();

    // Constant memory
    for (auto index : memory_analyzer.getConstBuffers())
        writeStatement("state.c{} = c{}", index, index);
    writeNewline();

    emitStateBindingAssignments();
}

void LangEmitter::emitFunction(const ir::Function& func) {
    // Block enum
    enterScope("enum class Block_{}", func.getName());
    write("None = -1,");
    for (const auto& [label, block] : func.getBlocks()) {
        write("{} = {},", label, static_cast<u32>(label));
    }
    exitScopeEmpty(true);
    writeNewline();

    // Blocks
    for (const auto& [_, block] : func.getBlocks())
        emitBlock(func, block);

    // Function
    // TODO: function name
    std::string name = "main";
    if (name == "main") {
        emitMainPrototype();
    } else
        LOG_FATAL(ShaderDecompiler,
                  "Custom functions not implemented (name: {})", name);

    // Structurize
    /*
    analyzer::CfgBuilder cfg_builder;
    auto entry_block = cfg_builder.Build(func);
    auto entry_node = analyzer::Structurize(entry_block);
    */

    // Caller loop
    writeStatement("auto next = Block_{}::{}", func.getName(), label_t(0x0));
    enterScope("while (next != Block_{}::None)", func.getName());
    enterScope("switch (next)");
    for (const auto& [label, block] : func.getBlocks()) {
        indent--;
        write("case Block_{}::{}:", func.getName(), label);
        indent++;
        writeStatement("next = func_{}(state)", label);
        writeStatement("break");
    }
    exitScopeEmpty();
    exitScopeEmpty();
    writeNewline();

    // Exit
    // Outputs
    switch (context.type) {
    case ShaderType::Vertex:
        // TODO: don't hardcode the bit cast type
#define ADD_OUTPUT(sv_semantic, index, base, c)                                \
    {                                                                          \
        writeStatement("{} = as_type<float>({})",                              \
                       getSvAccessQualifiedStr(                                \
                           SvAccess(Sv(sv_semantic, index), c), true),         \
                       getAttrMemoryStr({RZ, base + c * 0x4, false}));         \
    }
#define ADD_OUTPUT_1(sv_semantic, index, base)                                 \
    ADD_OUTPUT(sv_semantic, index, base, 0)
#define ADD_OUTPUT_VEC4(sv_semantic, index, base)                              \
    {                                                                          \
        for (u8 c = 0; c < 4; c++) {                                           \
            ADD_OUTPUT(sv_semantic, index, base, c);                           \
        }                                                                      \
    }

        ADD_OUTPUT_VEC4(SvSemantic::Position, invalid<u8>(), SV_POSITION_BASE);
        for (const auto output : memory_analyzer.getStageOutputs())
            ADD_OUTPUT_VEC4(SvSemantic::UserInOut, output,
                            SV_USER_IN_OUT_BASE + output * 0x10);

#undef ADD_OUTPUT
        break;
    case ShaderType::Fragment:
        for (u8 i = 0; i < COLOR_TARGET_COUNT; i++) {
            const auto color_target_data_type =
                state.color_target_data_types[i];
            if (color_target_data_type == ColorDataType::Invalid)
                continue;

            for (u8 c = 0; c < 4; c++) {
                writeStatement(
                    "{} = as_type<{}>({})",
                    getSvAccessQualifiedStr(
                        SvAccess(Sv(SvSemantic::UserInOut, i), c), true),
                    toType(color_target_data_type), getRegisterStr(i * 4 + c));
            }
        }
        break;
    default:
        break;
    }
    writeNewline();

    emitExitReturn();

    // Emit
    // EmitNode(func, entry_node);
    exitScopeEmpty();
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
            writeStatement("break");
            break;
        case analyzer::LastStatement::Continue:
            writeStatement("continue");
            break;
        }
    } else if (auto block = dynamic_cast<const analyzer::CfgBlock*>(node)) {
        for (const auto block_node : block->nodes)
            EmitNode(func, block_node);
    } else if (auto if_block =
                   dynamic_cast<const analyzer::CfgIfBlock*>(node)) {
        // If
        enterScope("if ({})", GetValueStr(if_block->cond));
        EmitNode(func, if_block->then_block);
        ExitScopeEmpty();
    } else if (auto if_else_block =
                   dynamic_cast<const analyzer::CfgIfElseBlock*>(node)) {
        // If
        enterScope("if ({})", GetValueStr(if_else_block->cond));
        EmitNode(func, if_else_block->then_block);
        ExitScopeEmpty();

        // Else
        enterScope("else");
        EmitNode(func, if_else_block->else_block);
        ExitScopeEmpty();
    } else if (auto while_block =
                   dynamic_cast<const analyzer::CfgWhileBlock*>(node)) {
        // While
        if (!while_block->IsDoWhile()) {
            enterScope("while ({})", GetValueStr(if_block->cond));
            EmitNode(func, while_block->body_block);
            ExitScopeEmpty();
        } else {
            enterScope("do");
            EmitNode(func, while_block->body_block);
            ExitScope("while ({})", GetValueStr(if_block->cond));
        }
    } else {
        LOG_ERROR(ShaderDecompiler, "Invalid structured node");
    }
}
*/

void LangEmitter::emitBlock(const ir::Function& func, const ir::Block& block) {
    enterScope("Block_{} func_{}(thread State& state)", func.getName(),
               block.getLabel());

    // Block enum alias
    writeStatement("using Block = Block_{}", func.getName());
    writeNewline();

    // Temporary
    enterScope("union");
    write("int4 i;");
    write("uint4 u;");
    write("float4 f;");
    exitScope("temp");
    writeNewline();

    for (const auto& instruction : block.getInstructions())
        emitInstruction(instruction);
    exitScopeEmpty();
}

// Data
void LangEmitter::emitCopy(const ir::Value& dst, const ir::Value& src) {
    storeValue(dst, "{}", getValueStr(src));
}

void LangEmitter::emitCast(const ir::Value& dst, const ir::Value& src) {
    storeValue(dst, "({}({}))", getTypeStr(dst.getType()), getValueStr(src));
}

// Arithmetic
void LangEmitter::emitAbs(const ir::Value& dst, const ir::Value& src) {
    storeValue(dst, "abs({})", getValueStr(src));
}

void LangEmitter::emitNeg(const ir::Value& dst, const ir::Value& src) {
    storeValue(dst, "(-{})", getValueStr(src));
}

void LangEmitter::emitAdd(const ir::Value& dst, const ir::Value& srcA,
                          const ir::Value& srcB) {
    storeValue(dst, "({} + {})", getValueStr(srcA), getValueStr(srcB));
}

void LangEmitter::emitMultiply(const ir::Value& dst, const ir::Value& srcA,
                               const ir::Value& srcB) {
    storeValue(dst, "({} * {})", getValueStr(srcA), getValueStr(srcB));
}

void LangEmitter::emitFma(const ir::Value& dst, const ir::Value& srcA,
                          const ir::Value& srcB, const ir::Value& srcC) {
    storeValue(dst, "fma({}, {}, {})", getValueStr(srcA), getValueStr(srcB),
               getValueStr(srcC));
}

void LangEmitter::emitMin(const ir::Value& dst, const ir::Value& srcA,
                          const ir::Value& srcB) {
    storeValue(dst, "min({}, {})", getValueStr(srcA), getValueStr(srcB));
}

void LangEmitter::emitMax(const ir::Value& dst, const ir::Value& srcA,
                          const ir::Value& srcB) {
    storeValue(dst, "max({}, {})", getValueStr(srcA), getValueStr(srcB));
}

void LangEmitter::emitClamp(const ir::Value& dst, const ir::Value& srcA,
                            const ir::Value& srcB, const ir::Value& srcC) {
    storeValue(dst, "clamp({}, {}, {})", getValueStr(srcA), getValueStr(srcB),
               getValueStr(srcC));
}

// Math
void LangEmitter::emitRound(const ir::Value& dst, const ir::Value& src) {
    storeValue(dst, "round({})", getValueStr(src));
}

void LangEmitter::emitFloor(const ir::Value& dst, const ir::Value& src) {
    storeValue(dst, "floor({})", getValueStr(src));
}

void LangEmitter::emitCeil(const ir::Value& dst, const ir::Value& src) {
    storeValue(dst, "ceil({})", getValueStr(src));
}

void LangEmitter::emitTrunc(const ir::Value& dst, const ir::Value& src) {
    storeValue(dst, "trunc({})", getValueStr(src));
}

// Logical & Bitwise
void LangEmitter::emitNot(const ir::Value& dst, const ir::Value& src) {
    storeValue(dst, "(!{})", getValueStr(src));
}

void LangEmitter::emitBitwiseNot(const ir::Value& dst, const ir::Value& src) {
    storeValue(dst, "(~{})", getValueStr(src));
}

void LangEmitter::emitBitwiseAnd(const ir::Value& dst, const ir::Value& srcA,
                                 const ir::Value& srcB) {
    storeValue(dst, "({} & {})", getValueStr(srcA), getValueStr(srcB));
}

void LangEmitter::emitBitwiseOr(const ir::Value& dst, const ir::Value& srcA,
                                const ir::Value& srcB) {
    storeValue(dst, "({} | {})", getValueStr(srcA), getValueStr(srcB));
}

void LangEmitter::emitBitwiseXor(const ir::Value& dst, const ir::Value& srcA,
                                 const ir::Value& srcB) {
    storeValue(dst, "({} ^ {})", getValueStr(srcA), getValueStr(srcB));
}

void LangEmitter::emitShiftLeft(const ir::Value& dst, const ir::Value& src_a,
                                const ir::Value& src_b) {
    storeValue(dst, "({} << {})", getValueStr(src_a), getValueStr(src_b));
}

void LangEmitter::emitShiftRight(const ir::Value& dst, const ir::Value& src_a,
                                 const ir::Value& src_b) {
    storeValue(dst, "({} >> {})", getValueStr(src_a), getValueStr(src_b));
}

// Comparison & Selection
void LangEmitter::emitCompareLess(const ir::Value& dst, const ir::Value& srcA,
                                  const ir::Value& srcB) {
    storeValue(dst, "({} < {})", getValueStr(srcA), getValueStr(srcB));
}

void LangEmitter::emitCompareLessOrEqual(const ir::Value& dst,
                                         const ir::Value& srcA,
                                         const ir::Value& srcB) {
    storeValue(dst, "({} <= {})", getValueStr(srcA), getValueStr(srcB));
}

void LangEmitter::emitCompareGreater(const ir::Value& dst,
                                     const ir::Value& srcA,
                                     const ir::Value& srcB) {
    storeValue(dst, "({} > {})", getValueStr(srcA), getValueStr(srcB));
}

void LangEmitter::emitCompareGreaterOrEqual(const ir::Value& dst,
                                            const ir::Value& srcA,
                                            const ir::Value& srcB) {
    storeValue(dst, "({} >= {})", getValueStr(srcA), getValueStr(srcB));
}

void LangEmitter::emitCompareEqual(const ir::Value& dst, const ir::Value& srcA,
                                   const ir::Value& srcB) {
    storeValue(dst, "({} == {})", getValueStr(srcA), getValueStr(srcB));
}

void LangEmitter::emitCompareNotEqual(const ir::Value& dst,
                                      const ir::Value& srcA,
                                      const ir::Value& srcB) {
    storeValue(dst, "({} != {})", getValueStr(srcA), getValueStr(srcB));
}

void LangEmitter::emitSelect(const ir::Value& dst, const ir::Value& cond,
                             const ir::Value& src_true,
                             const ir::Value& src_false) {
    storeValue(dst, "({} ? {} : {})", getValueStr(cond), getValueStr(src_true),
               getValueStr(src_false));
}

// Control flow
void LangEmitter::emitBeginIf(const ir::Value& cond) {
    enterScope("if ({})", getValueStr(cond));
}

void LangEmitter::emitEndIf() { exitScopeEmpty(); }

void LangEmitter::emitBranch(label_t target) {
    // LOG_FATAL(ShaderDecompiler, "Should not happen");
    writeStatement("return Block::{}", target);
}

void LangEmitter::emitBranchConditional(const ir::Value& cond,
                                        label_t target_true,
                                        label_t target_false) {
    // LOG_FATAL(ShaderDecompiler, "Should not happen");
    enterScope("if ({})", getValueStr(cond));
    writeStatement("return Block::{}", target_true);
    exitScopeEmpty();
    enterScope("else");
    writeStatement("return Block::{}", target_false);
    exitScopeEmpty();
}

// Vector
void LangEmitter::emitVectorExtract(const ir::Value& dst, const ir::Value& src,
                                    u8 index) {
    storeValue(dst, "({}.{})", getValueStr(src),
               getComponentStrFromIndex(index));
}

void LangEmitter::emitVectorInsert(const ir::Value& dst, const ir::Value& src,
                                   u8 index) {
    writeStatement("{}.{} = {}", getValueStr(dst),
                   getComponentStrFromIndex(index), getValueStr(src));
}

void LangEmitter::emitVectorConstruct(const ir::Value& dst,
                                      const std::vector<ir::Value>& elements) {
    std::string str;
    for (u32 i = 0; i < elements.size(); i++) {
        if (i != 0)
            str += ", ";
        str += getValueStr(elements[i]);
    }
    storeValue(dst, "{}({})", getTypeStr(dst.getType()), str);
}

// Exit
void LangEmitter::emitExit() { writeStatement("return Block::None"); }

} // namespace hydra::hw::tegra_x1::gpu::renderer::shader_decomp::codegen::lang
