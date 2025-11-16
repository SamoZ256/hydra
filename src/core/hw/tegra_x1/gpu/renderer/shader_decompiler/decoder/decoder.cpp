#include "core/hw/tegra_x1/gpu/renderer/shader_decompiler/decoder/decoder.hpp"

#include "core/hw/tegra_x1/gpu/renderer/shader_decompiler/decoder/tables.hpp"

#define BUILDER context.builder

namespace hydra::hw::tegra_x1::gpu::renderer::shader_decomp::decoder {

namespace {

inline ir::Value neg_if(ir::Builder& builder, const ir::Value& value,
                        bool neg) {
    return neg ? builder.OpNeg(value) : value;
}

inline ir::Value not_if(ir::Builder& builder, const ir::Value& value,
                        bool not_) {
    return not_ ? builder.OpNot(value) : value;
}

} // namespace

void Decoder::Decode() {
    crnt_block = &blocks[0x0];
    while (crnt_block) {
        ParseNextInstruction();
    }
}

void Decoder::ParseNextInstruction() {
    const u32 pc = GetPC();
    const auto inst = context.code_reader.Read<instruction_t>();
    if ((pc % 4) == 0) // Sched
        return;

    LOG_DEBUG(ShaderDecompiler, "Instruction 0x{:016x}", inst);

#define GET_REG(b) extract_bits<reg_t, b, 8>(inst)
#define GET_PRED(b) extract_bits<pred_t, b, 3>(inst)

#define GET_VALUE_U(type_bit_count, b, count)                                  \
    extract_bits<u##type_bit_count, b, count>(inst)
#define GET_VALUE_U_EXTEND(type_bit_count, b, count)                           \
    (GET_VALUE_U(type_bit_count, b, count) << (type_bit_count - count))
#define GET_VALUE_U32(b, count) GET_VALUE_U(32, b, count)
#define GET_VALUE_U32_EXTEND(b, count) GET_VALUE_U_EXTEND(32, b, count)
#define GET_VALUE_U64(b, count) GET_VALUE_U(64, b, count)
#define GET_VALUE_U64_EXTEND(b, count) GET_VALUE_U_EXTEND(64, b, count)

#define GET_VALUE_I(type_bit_count, b, count)                                  \
    extract_bits<i##type_bit_count, b, count>(inst)
#define GET_VALUE_I_SIGN_EXTEND(type_bit_count, b, count)                      \
    sign_extend<i##type_bit_count, count>(GET_VALUE_U(type_bit_count, b, count))
#define GET_VALUE_I32(b, count) GET_VALUE_I(32, b, count)
#define GET_VALUE_I32_SIGN_EXTEND(b, count)                                    \
    GET_VALUE_I_SIGN_EXTEND(32, b, count)
#define GET_VALUE_I64(b, count) GET_VALUE_I(64, b, count)
#define GET_VALUE_I64_SIGN_EXTEND(b, count)                                    \
    GET_VALUE_I_SIGN_EXTEND(64, b, count)

// TODO: correct?
#define GET_VALUE_F16(b) ((GET_VALUE_U32(b, 9)) << 6)

// TODO: correct?
#define GET_VALUE_F32() ((GET_BIT(56) << 31) | (GET_VALUE_U32(20, 19) << 12))

#define GET_BIT(b) extract_bits<u32, b, 1>(inst)

#define GET_BTARG()                                                            \
    static_cast<label_t>(                                                      \
        pc +                                                                   \
        std::bit_cast<i32>(GET_VALUE_I32_SIGN_EXTEND(20, 24)) /                \
            sizeof(instruction_t) +                                            \
        1)

#define GET_AMEM(is_input)                                                     \
    AMem { GET_REG(8), 0, is_input }
#define GET_AMEM_IDX(b, is_input)                                              \
    AMem { GET_REG(8), extract_bits<u32, b, 10>(inst), is_input }

#define GET_CMEM(b_idx, count_imm)                                             \
    CMem {                                                                     \
        GET_VALUE_U32(b_idx, 5), RZ,                                           \
            extract_bits<u32, 20, count_imm>(inst) * 4                         \
    }
#define GET_CMEM_R(b_idx, b_reg, count_imm)                                    \
    CMem {                                                                     \
        GET_VALUE_U32(b_idx, 5), GET_REG(b_reg),                               \
            extract_bits<u32, 20, count_imm>(inst) * 4                         \
    }

// TODO: global memory
#define GET_NCGMEM_R(b_reg, count_imm, is_input)                               \
    AMem {                                                                     \
        GET_REG(b_reg), extract_bits<u32, 20, count_imm>(inst) * 4, is_input   \
    }

#define NEG_IF(value, neg) neg_if(BUILDER, value, neg)
#define NOT_IF(value, not_) not_if(BUILDER, value, not_)

#define PRED_COND_NOTHING ((inst & 0x00000000000f0000) == 0x0000000000070000)
#define PRED_COND_NEVER ((inst & 0x00000000000f0000) == 0x00000000000f0000)

#define HANDLE_PRED_COND_BEGIN()                                               \
    bool conditional = false;                                                  \
    if (PRED_COND_NOTHING) {      /* nothing */                                \
    } else if (PRED_COND_NEVER) { /* never */                                  \
        COMMENT("never");                                                      \
        abort(); /* TODO: implement */                                         \
    } else {     /* conditional */                                             \
        const auto pred = GET_PRED(16);                                        \
        const bool not_ = GET_BIT(19);                                         \
        COMMENT("if {}{}", not_ ? "!" : "", pred);                             \
        BUILDER.OpBeginIf({NOT_IF(ir::Value::Predicate(pred), not_)});         \
        conditional = true;                                                    \
    }
#define HANDLE_PRED_COND_END()                                                 \
    if (conditional) {                                                         \
        BUILDER.OpEndIf();                                                     \
    }

// TODO: ignore on release
#define COMMENT_IMPL(log_level, f_comment, f_log, ...)                         \
    {                                                                          \
        /* TODO: comments */                                                   \
        /*BUILDER.OpDebugComment(fmt::format(f_comment                         \
         * PASS_VA_ARGS(__VA_ARGS__)));*/                                      \
        LOG_##log_level(ShaderDecompiler, f_log PASS_VA_ARGS(__VA_ARGS__));    \
    }
#define COMMENT(f, ...) COMMENT_IMPL(DEBUG, f, f, __VA_ARGS__)
#define COMMENT_NOT_IMPLEMENTED(f, ...)                                        \
    COMMENT_IMPL(NOT_IMPLEMENTED, f " (NOT IMPLEMENTED)", f, __VA_ARGS__)

#define INST0(value, mask) if ((inst & mask##ull) == value##ull)
#define INST(value, mask) else INST0(value, mask)

    INST0(0xfbe0000000000000, 0xfff8000000000000) {
        COMMENT_NOT_IMPLEMENTED("out");
    }
    INST(0xf6e0000000000000, 0xfef8000000000000) {
        COMMENT_NOT_IMPLEMENTED("out");
    }
    INST(0xf0f8000000000000, 0xfff8000000000000) {
        // TODO: f0f8_0
        COMMENT("sync");

        ASSERT_DEBUG(!crnt_block->sync_point_stack.empty(), ShaderDecompiler,
                     "No sync point in stack");

        const auto target = crnt_block->sync_point_stack.top();
        if (PRED_COND_NOTHING) { // Nothing
            // Pop and then inherit
            crnt_block->sync_point_stack.pop();
            InheritSyncPoints(target);

            BUILDER.OpBranch(target);
            EndBlock();
        } else if (PRED_COND_NEVER) { // Never
            // Pop
            crnt_block->sync_point_stack.pop();

            COMMENT("never");
            abort(); /* TODO: implement */
        } else {     // Conditional
            const auto pred = GET_PRED(16);
            const bool not_ = GET_BIT(19);
            COMMENT("if {}{}", not_ ? "!" : "", pred);

            // Inherit for the continuation block, then pop and inherit for sync
            // block
            InheritSyncPoints(pc + 1);
            crnt_block->sync_point_stack.pop();
            InheritSyncPoints(target);

            BUILDER.OpBranchConditional(
                NOT_IF(ir::Value::Predicate(pred), not_), target, pc + 1);
            EndBlock();
        }
    }
    INST(0xf0f0000000000000, 0xfff8000000000000) {
        COMMENT_NOT_IMPLEMENTED("depbar");
    }
    INST(0xf0c8000000000000, 0xfff8000000000000) {
        COMMENT_NOT_IMPLEMENTED("mov");
    }
    INST(0xf0c0000000000000, 0xfff8000000000000) {
        COMMENT_NOT_IMPLEMENTED("r2b");
    }
    INST(0xf0b8000000000000, 0xfff8000000000000) {
        COMMENT_NOT_IMPLEMENTED("b2r");
    }
    INST(0xf0a8000000000000, 0xfff8000000000000) {
        COMMENT_NOT_IMPLEMENTED("bar");
    }
    INST(0xeff0000000000000, 0xfff8000000000000) {
        const auto mode = get_operand_eff0_0(inst);
        auto dst = GET_AMEM_IDX(20, false);
        const auto src = GET_REG(0);
        const auto todo = GET_REG(39); // TODO: what is this?
        COMMENT("st {} a[{} + 0x{:08x}] {} {}", mode, dst.reg, dst.imm, src,
                todo);

        HANDLE_PRED_COND_BEGIN();

        for (u32 i = 0; i < get_load_store_count(mode); i++) {
            BUILDER.OpCopy(ir::Value::AttrMemory(dst),
                           ir::Value::Register(src + i));
            dst.imm += sizeof(u32);
        }

        HANDLE_PRED_COND_END();
    }
    INST(0xefe8000000000000, 0xfff8000000000000) {
        COMMENT_NOT_IMPLEMENTED("pixld");
    }
    INST(0xefd8000000000000, 0xfff8000000000000) {
        const auto size = get_operand_eff0_0(inst);
        const auto dst = GET_REG(0);
        auto src = GET_AMEM_IDX(20, true);
        const auto todo = GET_REG(39); // TODO: what is this?
        COMMENT("ld {} {} a[{} + 0x{:08x}] {}", size, dst, src.reg, src.imm,
                todo);

        HANDLE_PRED_COND_BEGIN();

        for (u32 i = 0; i < get_load_store_count(size); i++) {
            BUILDER.OpCopy(ir::Value::Register(dst + i),
                           ir::Value::AttrMemory(src));
            src.imm += sizeof(u32);
        }

        HANDLE_PRED_COND_END();
    }
    INST(0xefd0000000000000, 0xfff8000000000000) {
        COMMENT_NOT_IMPLEMENTED("isberd");
    }
    INST(0xefa0000000000000, 0xfff8000000000000) {
        COMMENT_NOT_IMPLEMENTED("al2p");
    }
    INST(0xef98000000000000, 0xfff8000000000000) {
        COMMENT_NOT_IMPLEMENTED("membar");
    }
    INST(0xef90000000000000, 0xfff8000000000000) {
        const auto addr_mode = get_operand_ef90_0(inst);
        const auto size = get_operand_ef90sz(inst);
        const auto dst = GET_REG(0);
        auto src = GET_CMEM_R(36, 8, 16);
        COMMENT("ld {} {} {} c{}[{} + 0x{:x}]", dst, addr_mode, size, src.idx,
                src.reg, src.imm);

        HANDLE_PRED_COND_BEGIN();

        // HACK
        src.imm /= 4;

        // TODO: address mode

        for (u32 i = 0; i < get_load_store_count(size); i++) {
            BUILDER.OpCopy(ir::Value::Register(dst + i),
                           ir::Value::ConstMemory(src));
            src.imm += sizeof(u32);
        }

        HANDLE_PRED_COND_END();
    }
    INST(0xef80000000000000, 0xffe0000000000000) {
        COMMENT_NOT_IMPLEMENTED("cctll");
    }
    INST(0xef60000000000000, 0xffe0000000000000) {
        COMMENT_NOT_IMPLEMENTED("cctl");
    }
    INST(0xef58000000000000, 0xfff8000000000000) {
        COMMENT_NOT_IMPLEMENTED("st");
    }
    INST(0xef50000000000000, 0xfff8000000000000) {
        COMMENT_NOT_IMPLEMENTED("st");
    }
    INST(0xef48000000000000, 0xfff8000000000000) {
        COMMENT_NOT_IMPLEMENTED("ld");
    }
    INST(0xef40000000000000, 0xfff8000000000000) {
        COMMENT_NOT_IMPLEMENTED("ld");
    }
    INST(0xef10000000000000, 0xfff8000000000000) {
        const auto mode = get_operand_ef10_0(inst);
        const auto dst_pred = GET_PRED(48);
        const auto dst = GET_REG(0);
        const auto srcA = GET_REG(8);
        COMMENT("shfl {} {} {} {} ...", mode, dst_pred, dst, srcA);

        HANDLE_PRED_COND_BEGIN();

        auto dst_v = ir::Value::Register(dst);
        auto dst_pred_v = ir::Value::Predicate(dst_pred);
        auto srcA_v = ir::Value::Register(srcA);
        auto srcB_v = ((inst & 0x10000000) == 0x0
                           ? ir::Value::Register(GET_REG(20))
                           : ir::Value::Immediate(GET_VALUE_U32(20, 5)));
        auto srcC_v = ((inst & 0x10000000) == 0x0
                           ? ir::Value::Register(GET_REG(39))
                           : ir::Value::Immediate(GET_VALUE_U32(34, 13)));

        std::optional<ir::Value> res_v;
        std::optional<ir::Value> res_valid_v;

        // HACK
        res_v = srcA_v;
        res_valid_v = ir::Value::Predicate(PT);
        /*
        switch (mode) {
        case ShuffleMode::Index:
            // TODO
            break;
        case ShuffleMode::Up:
            // TODO
            break;
        case ShuffleMode::Down:
            // TODO
            break;
        case ShuffleMode::Bfly:
            // TODO
            break;
        }
        */

        BUILDER.OpCopy(dst_v, res_v.value());
        BUILDER.OpCopy(dst_pred_v, res_valid_v.value());

        HANDLE_PRED_COND_END();
    }
    INST(0xeef0000000000000, 0xfff0000000000000) {
        COMMENT_NOT_IMPLEMENTED("atom");
    }
    INST(0xeed8000000000000, 0xfff8000000000000) {
        COMMENT_NOT_IMPLEMENTED("stg");
    }
    INST(0xeed0000000000000, 0xfff8000000000000) {
        LOG_WARN(ShaderDecompiler, "LDG");

        // TODO: eed0_0 (CacheLoadOp)
        const auto size = get_operand_eed0sz(inst);
        const auto dst = GET_REG(0);
        auto src = GET_NCGMEM_R(8, 20, true);
        COMMENT("ldg {} {} a[{} + 0x{:x}]", size, dst, src.reg, src.imm);

        HANDLE_PRED_COND_BEGIN();

        for (u32 i = 0; i < get_load_store_count(size); i++) {
            // TODO: global memory
            BUILDER.OpCopy(ir::Value::Register(dst + i),
                           ir::Value::Immediate(std::bit_cast<u32>(0.0f)));
            src.imm += sizeof(u32);
        }

        HANDLE_PRED_COND_END();
    }
    INST(0xeec8000000000000, 0xfff8000000000000) {
        COMMENT_NOT_IMPLEMENTED("ldg");
    }
    INST(0xeea0000000000000, 0xfff8000000000000) {
        COMMENT_NOT_IMPLEMENTED("stp");
    }
    INST(0xee00000000000000, 0xff80000000000000) {
        COMMENT_NOT_IMPLEMENTED("atoms");
    }
    INST(0xed00000000000000, 0xff00000000000000) {
        COMMENT_NOT_IMPLEMENTED("atom");
    }
    INST(0xec00000000000000, 0xff00000000000000) {
        COMMENT_NOT_IMPLEMENTED("atoms");
    }
    INST(0xebf8000000000000, 0xfff8000000000000) {
        COMMENT_NOT_IMPLEMENTED("red");
    }
    INST(0xebf0000000000000, 0xfff9000000000000) {
        COMMENT_NOT_IMPLEMENTED("cctlt");
    }
    INST(0xebe0000000000000, 0xfff8000000000000) {
        COMMENT_NOT_IMPLEMENTED("out");
    }
    INST(0xeb40000000000000, 0xffe0000000000000) {
        COMMENT_NOT_IMPLEMENTED("sured");
    }
    INST(0xeb20000000000000, 0xffe0000000000000) {
        COMMENT_NOT_IMPLEMENTED("sust");
    }
    INST(0xeb00000000000000, 0xffe0000000000000) {
        COMMENT_NOT_IMPLEMENTED("suld");
    }
    INST(0xeac0000000000000, 0xffe0000000000000) {
        COMMENT_NOT_IMPLEMENTED("suatom");
    }
    INST(0xea80000000000000, 0xffc0000000000000) {
        COMMENT_NOT_IMPLEMENTED("suatom");
    }
    INST(0xea60000000000000, 0xffe0000000000000) {
        COMMENT_NOT_IMPLEMENTED("suatom");
    }
    INST(0xea00000000000000, 0xffc0000000000000) {
        COMMENT_NOT_IMPLEMENTED("suatom");
    }
    INST(0xe3a0000000000000, 0xfff0000000000000) {
        COMMENT_NOT_IMPLEMENTED("bpt");
    }
    INST(0xe390000000000000, 0xfff0000000000000) {
        COMMENT_NOT_IMPLEMENTED("ide");
    }
    INST(0xe380000000000000, 0xfff0000000000000) {
        COMMENT_NOT_IMPLEMENTED("ram");
    }
    INST(0xe370000000000000, 0xfff0000000000000) {
        COMMENT_NOT_IMPLEMENTED("sam");
    }
    INST(0xe360000000000000, 0xfff0000000000000) {
        COMMENT_NOT_IMPLEMENTED("rtt");
    }
    INST(0xe350000000000000, 0xfff0000000000000) {
        COMMENT_NOT_IMPLEMENTED("cont");
    }
    INST(0xe340000000000000, 0xfff0000000000000) {
        COMMENT_NOT_IMPLEMENTED("brk");
    }
    INST(0xe330000000000000, 0xfff0000000000000) {
        // TODO: f0f8_0
        COMMENT("kil");

        HANDLE_PRED_COND_BEGIN();

        BUILDER.OpDiscard();

        HANDLE_PRED_COND_END();
    }
    INST(0xe320000000000000, 0xfff0000000000000) {
        COMMENT_NOT_IMPLEMENTED("ret");
    }
    INST(0xe310000000000000, 0xfff0000000000000) {
        COMMENT_NOT_IMPLEMENTED("longjmp");
    }
    INST(0xe300000000000000, 0xfff0000000000000) {
        // TODO: f0f8_0
        COMMENT("exit");

        if (PRED_COND_NOTHING) { // Nothing
            BUILDER.OpExit();
            EndBlock();
        } else if (PRED_COND_NEVER) { // Never
            COMMENT("never");
            abort(); /* TODO: implement */
        } else {     // Conditional
            const auto pred = GET_PRED(16);
            const bool not_ = GET_BIT(19);
            COMMENT("if {}{}", not_ ? "!" : "", pred);

            BUILDER.OpBeginIf({NOT_IF(ir::Value::Predicate(pred), not_)});
            BUILDER.OpExit();
            BUILDER.OpEndIf();
        }
    }
    INST(0xe2f0000000000000, 0xfff0000000000000) {
        COMMENT_NOT_IMPLEMENTED("setlmembase");
    }
    INST(0xe2e0000000000000, 0xfff0000000000000) {
        COMMENT_NOT_IMPLEMENTED("setcrsptr");
    }
    INST(0xe2d0000000000000, 0xfff0000000000000) {
        COMMENT_NOT_IMPLEMENTED("getlmembase");
    }
    INST(0xe2c0000000000000, 0xfff0000000000000) {
        COMMENT_NOT_IMPLEMENTED("getcrsptr");
    }
    INST(0xe2b0000000000020, 0xfff0000000000020) {
        COMMENT_NOT_IMPLEMENTED("pcnt");
    }
    INST(0xe2b0000000000000, 0xfff0000000000020) {
        COMMENT_NOT_IMPLEMENTED("pcnt");
    }
    INST(0xe2a0000000000020, 0xfff0000000000020) {
        COMMENT_NOT_IMPLEMENTED("pbk");
    }
    INST(0xe2a0000000000000, 0xfff0000000000020) {
        COMMENT_NOT_IMPLEMENTED("pbk");
    }
    INST(0xe290000000000020, 0xfff0000000000020) {
        COMMENT_NOT_IMPLEMENTED("ssy");
    }
    INST(0xe290000000000000, 0xfff0000000000020) {
        const auto target = GET_BTARG();
        COMMENT("ssy {}", target);

        PushSyncPoint(target);
    }
    INST(0xe280000000000020, 0xfff0000000000020) {
        COMMENT_NOT_IMPLEMENTED("plongjmp");
    }
    INST(0xe280000000000000, 0xfff0000000000020) {
        COMMENT_NOT_IMPLEMENTED("plongjmp");
    }
    INST(0xe270000000000020, 0xfff0000000000020) {
        COMMENT_NOT_IMPLEMENTED("pret");
    }
    INST(0xe270000000000000, 0xfff0000000000020) {
        COMMENT_NOT_IMPLEMENTED("pret");
    }
    INST(0xe260000000000020, 0xfff0000000000020) {
        COMMENT_NOT_IMPLEMENTED("cal");
    }
    INST(0xe260000000000000, 0xfff0000000000020) {
        COMMENT_NOT_IMPLEMENTED("cal");
    }
    INST(0xe250000000000020, 0xfff0000000000020) {
        COMMENT_NOT_IMPLEMENTED("brx");
    }
    INST(0xe250000000000000, 0xfff0000000000000) {
        COMMENT_NOT_IMPLEMENTED("brx");
    }
    INST(0xe240000000000020, 0xfff0000000000020) {
        COMMENT_NOT_IMPLEMENTED("bra");
    }
    INST(0xe240000000000000, 0xfff0000000000020) {
        // TODO: f0f8_0
        const auto target = GET_BTARG();
        COMMENT("bra {}", target);

        if (PRED_COND_NOTHING) { // Nothing
            InheritSyncPoints(target);
            BUILDER.OpBranch(target);
            EndBlock();
        } else if (PRED_COND_NEVER) { // Never
            COMMENT("never");
            abort(); /* TODO: implement */
        } else {     // Conditional
            const auto pred = GET_PRED(16);
            const bool not_ = GET_BIT(19);
            COMMENT("if {}{}", not_ ? "!" : "", pred);

            InheritSyncPoints(target);
            InheritSyncPoints(pc + 1);
            BUILDER.OpBranchConditional(
                NOT_IF(ir::Value::Predicate(pred), not_), target, pc + 1);
            EndBlock();
        }
    }
    INST(0xe230000000000000, 0xfff0000000000000) {
        COMMENT_NOT_IMPLEMENTED("pexit");
    }
    INST(0xe220000000000020, 0xfff0000000000020) {
        COMMENT_NOT_IMPLEMENTED("jcal");
    }
    INST(0xe220000000000000, 0xfff0000000000020) {
        COMMENT_NOT_IMPLEMENTED("jcal");
    }
    INST(0xe210000000000020, 0xfff0000000000020) {
        COMMENT_NOT_IMPLEMENTED("jmp");
    }
    INST(0xe210000000000000, 0xfff0000000000000) {
        COMMENT_NOT_IMPLEMENTED("jmp");
    }
    INST(0xe200000000000020, 0xfff0000000000020) {
        COMMENT_NOT_IMPLEMENTED("jmx");
    }
    INST(0xe200000000000000, 0xfff0000000000020) {
        COMMENT_NOT_IMPLEMENTED("jmx");
    }
    INST(0xe00000000000ff00, 0xff0000400000ff00) {
        const auto op = get_operand_e000_0(inst);
        // TODO: e000_1 (interpolation attribute)
        const auto dst = GET_REG(0);
        const auto amem = GET_AMEM_IDX(28, true);
        const auto interp_param = GET_REG(20);
        const auto flag1 = GET_REG(39);
        COMMENT("ipa {} {} a[{} + 0x{:08x}] {} {}", op, dst, amem.reg, amem.imm,
                interp_param, flag1);

        HANDLE_PRED_COND_BEGIN();

        auto src_v = ir::Value::AttrMemory(amem, DataType::F32);

        // HACK: multiply by position.w
        if (amem.reg == RZ && amem.imm >= 0x80 &&
            context.decomp_context.frag.pixel_imaps[(amem.imm - 0x80) >> 0x4]
                    .x == PixelImapType::Perspective)
            src_v = BUILDER.OpMultiply(
                src_v,
                ir::Value::AttrMemory(AMem{RZ, 0x7c, true}, DataType::F32));

        auto interp_param_v = ir::Value::Register(interp_param, DataType::F32);
        std::optional<ir::Value> res_v;
        switch (op) {
        case IpaOp::Pass:
            res_v = src_v;
            break;
        case IpaOp::Multiply:
            res_v = BUILDER.OpMultiply(src_v, interp_param_v);
            break;
        case IpaOp::Constant:
            LOG_NOT_IMPLEMENTED(ShaderDecompiler, "IpaOp Constant");
            // TODO: implement
            res_v = src_v;
            break;
        case IpaOp::SC:
            LOG_NOT_IMPLEMENTED(ShaderDecompiler, "IpaOp SC");
            // TODO: implement
            res_v = src_v;
            break;
        default:
            res_v = std::nullopt;
            break;
        }
        BUILDER.OpCopy(ir::Value::Register(dst, DataType::F32), res_v.value());

        HANDLE_PRED_COND_END();
    }
    INST(0xe000004000000000, 0xff00004000000000) {
        COMMENT_NOT_IMPLEMENTED("ipa");
    }
    INST(0xdf60000000000000, 0xfff8000000000000) {
        COMMENT_NOT_IMPLEMENTED("tmml");
    }
    INST(0xdf58000000000000, 0xfff8000000000000) {
        COMMENT_NOT_IMPLEMENTED("tmml");
    }
    INST(0xdf50000000000000, 0xfff8000000000000) {
        const auto dst = GET_REG(0);
        const auto srcA = GET_REG(8); // TODO: is this for LOD and stuff?
        const auto query = get_operand_df50_0(inst);
        const auto const_buffer_index = GET_VALUE_U32(36, 13);
        const auto component_mask = GET_VALUE_U32(31, 4);
        COMMENT("txq {} {} {} 0x{:08x} 0x{:x}", dst, srcA, query,
                const_buffer_index, component_mask);

        HANDLE_PRED_COND_BEGIN();

        switch (query) {
        case TextureQuery::Dimensions:
            for (u32 i = 0, comp_mask = component_mask; comp_mask != 0x0;
                 i++, comp_mask >>= 1) {
                if (comp_mask & 1) {
                    const auto res =
                        BUILDER.OpTextureQueryDimension(const_buffer_index, i);
                    BUILDER.OpCopy(ir::Value::Register(dst + i), res);
                }
            }
            break;
        default:
            LOG_NOT_IMPLEMENTED(ShaderDecompiler, "Texture query {}", query);
            break;
        }

        HANDLE_PRED_COND_END();
    }
    INST(0xdf48000000000000, 0xfff8000000000000) {
        COMMENT_NOT_IMPLEMENTED("txq");
    }
    INST(0xdf40000000000000, 0xfff8000000000000) {
        COMMENT_NOT_IMPLEMENTED("txa");
    }
    INST(0xdf00000000000000, 0xff40000000000000) {
        const auto component = get_operand_df00_0(inst);
        const auto dst1 = GET_REG(28);
        const auto dst0 = GET_REG(0);
        const auto coords_x = GET_REG(8);
        const auto coords_y = GET_REG(20);
        const auto const_buffer_index = GET_VALUE_U32(36, 13);
        COMMENT("tld4s {} {} {} {} {} 0x{:08x}", component, dst1, dst0,
                coords_x, coords_y, const_buffer_index);

        HANDLE_PRED_COND_BEGIN();

        auto coords_v = BUILDER.OpVectorConstruct(
            DataType::F32, {ir::Value::Register(coords_x, DataType::F32),
                            ir::Value::Register(coords_y, DataType::F32)});
        auto res_v =
            BUILDER.OpTextureGather(const_buffer_index, coords_v, component);
        BUILDER.OpCopy(ir::Value::Register(dst0, DataType::F32),
                       BUILDER.OpVectorExtract(res_v, 0));
        BUILDER.OpCopy(ir::Value::Register(dst0 + 1, DataType::F32),
                       BUILDER.OpVectorExtract(res_v, 1));
        BUILDER.OpCopy(ir::Value::Register(dst1, DataType::F32),
                       BUILDER.OpVectorExtract(res_v, 2));
        BUILDER.OpCopy(ir::Value::Register(dst1 + 1, DataType::F32),
                       BUILDER.OpVectorExtract(res_v, 3));

        HANDLE_PRED_COND_END();
    }
    INST(0xdef8000000000000, 0xfff8000000000000) {
        COMMENT_NOT_IMPLEMENTED("tld4");
    }
    INST(0xdeb8000000000000, 0xfff8000000000000) {
        // TODO: more stuff
        // TOOD: correct?
        const auto dst = GET_REG(0);
        const auto coords_x = GET_REG(8);
        const auto coords_y = GET_REG(20);
        const auto todo =
            GET_VALUE_U32(31, 4); // TODO: what is this? (some sort of mask)
        COMMENT("tex {} {} {} 0x{:x}", dst, coords_x, coords_y, todo);

        HANDLE_PRED_COND_BEGIN();

        // TODO: how does bindless work?
        COMMENT_NOT_IMPLEMENTED("Bindless");
        // BUILDER.OpTextureSample(
        //     ir::Value::Register(dst),
        //     ir::Value::Register(dst + 1),
        //     ir::Value::Register(dst + 2),
        //     ir::Value::Register(dst + 3), const_buffer_index,
        //     ir::Value::Register(coords_x),
        //     ir::Value::Register(coords_y));

        HANDLE_PRED_COND_END();
    }
    INST(0xde78000000000000, 0xfffc000000000000) {
        COMMENT_NOT_IMPLEMENTED("txd");
    }
    INST(0xde38000000000000, 0xfffc000000000000) {
        COMMENT_NOT_IMPLEMENTED("txd");
    }
    INST(0xdd38000000000000, 0xff38000000000000) {
        COMMENT_NOT_IMPLEMENTED("tld");
    }
    INST(0xdc38000000000000, 0xff38000000000000) {
        COMMENT_NOT_IMPLEMENTED("tld");
    }
    INST(0xd200000000000000, 0xf600000000000000) {
        // TODO: d200_0
        const auto dst1 = GET_REG(28);
        const auto dst0 = GET_REG(0);
        const auto coords_x = GET_REG(8);
        const auto coords_y = GET_REG(20);
        const auto const_buffer_index = GET_VALUE_U32(36, 13);
        // TODO: texture type
        // TODO: component swizzle?
        COMMENT("tlds {} {} {} {} 0x{:08x}", dst1, dst0, coords_x, coords_y,
                const_buffer_index);

        HANDLE_PRED_COND_BEGIN();

        auto coords_v = BUILDER.OpVectorConstruct(
            DataType::I32, {ir::Value::Register(coords_x, DataType::I32),
                            ir::Value::Register(coords_y, DataType::I32)});
        auto res_v = BUILDER.OpTextureRead(const_buffer_index, coords_v);
        BUILDER.OpCopy(ir::Value::Register(dst0, DataType::F32),
                       BUILDER.OpVectorExtract(res_v, 0));
        BUILDER.OpCopy(ir::Value::Register(dst0 + 1, DataType::F32),
                       BUILDER.OpVectorExtract(res_v, 1));
        BUILDER.OpCopy(ir::Value::Register(dst1, DataType::F32),
                       BUILDER.OpVectorExtract(res_v, 2));
        BUILDER.OpCopy(ir::Value::Register(dst1 + 1, DataType::F32),
                       BUILDER.OpVectorExtract(res_v, 3));

        HANDLE_PRED_COND_END();
    }
    INST(0xd000000000000000, 0xf600000000000000) {
        const auto dst1 = GET_REG(28);
        const auto dst0 = GET_REG(0);
        const auto coords_x = GET_REG(8);
        const auto coords_y = GET_REG(20);
        const auto const_buffer_index = GET_VALUE_U32(36, 13);
        // TODO: texture type
        auto component_mask = get_operand_d200_2_ff(inst);
        if (component_mask == ComponentMask::Invalid)
            component_mask = get_operand_d200_2_00(inst);
        COMMENT("texs {} {} {} {} 0x{:08x} {}", dst1, dst0, coords_x, coords_y,
                const_buffer_index, component_mask);

        HANDLE_PRED_COND_BEGIN();

        auto coords_v = BUILDER.OpVectorConstruct(
            DataType::F32, {ir::Value::Register(coords_x, DataType::F32),
                            ir::Value::Register(coords_y, DataType::F32)});
        auto res_v = BUILDER.OpTextureSample(const_buffer_index, coords_v);

        std::optional<ir::Value> c[4] = {std::nullopt, std::nullopt,
                                         std::nullopt, std::nullopt};

#define C(i) BUILDER.OpVectorExtract(res_v, i)

        switch (component_mask) {
        case ComponentMask::R:
            c[0] = C(0);
            break;
        case ComponentMask::G:
            c[0] = C(1);
            break;
        case ComponentMask::B:
            c[0] = C(2);
            break;
        case ComponentMask::A:
            c[0] = C(3);
            break;
        case ComponentMask::RG:
            c[0] = C(0);
            c[1] = C(1);
            break;
        case ComponentMask::RA:
            c[0] = C(0);
            c[1] = C(3);
            break;
        case ComponentMask::GA:
            c[0] = C(1);
            c[1] = C(3);
            break;
        case ComponentMask::BA:
            c[0] = C(2);
            c[1] = C(3);
            break;
        case ComponentMask::RGB:
            c[0] = C(0);
            c[1] = C(1);
            c[2] = C(2);
            break;
        case ComponentMask::RGA:
            c[0] = C(0);
            c[1] = C(1);
            c[2] = C(3);
            break;
        case ComponentMask::RBA:
            c[0] = C(0);
            c[1] = C(2);
            c[2] = C(3);
            break;
        case ComponentMask::GBA:
            c[0] = C(1);
            c[1] = C(2);
            c[2] = C(3);
            break;
        case ComponentMask::RGBA:
            c[0] = C(0);
            c[1] = C(1);
            c[2] = C(2);
            c[3] = C(3);
            break;
        default:
            LOG_ERROR(ShaderDecompiler, "Invalid component mask");
            break;
        }

#undef C

        reg_t regs[4] = {dst0, dst0 + 1, dst1, dst1 + 1};
        for (int i = 0; i < 4; i++) {
            if (auto value = c[i])
                BUILDER.OpCopy(ir::Value::Register(regs[i], DataType::F32),
                               *value);
        }

        HANDLE_PRED_COND_END();
    }
    INST(0xc838000000000000, 0xfc38000000000000) {
        const auto component = get_operand_c838_0(inst);
        // TODO: c838_1
        const auto dst = GET_REG(0);
        const auto coords_x = GET_REG(8);
        const auto coords_y = GET_REG(20);
        const auto const_buffer_index = GET_VALUE_U32(36, 13);
        // TODO: texture type
        const auto component_mask = GET_VALUE_U32(31, 4);
        COMMENT("tld4 {} {} {} {} 0x{:08x} {:x}", component, dst, coords_x,
                coords_y, const_buffer_index, component_mask);

        HANDLE_PRED_COND_BEGIN();

        auto coords_v = BUILDER.OpVectorConstruct(
            DataType::F32, {ir::Value::Register(coords_x, DataType::F32),
                            ir::Value::Register(coords_y, DataType::F32)});
        auto res_v =
            BUILDER.OpTextureGather(const_buffer_index, coords_v, component);
        for (u32 i = 0; i < 4; i++) {
            if (component_mask & (1 << i)) {
                BUILDER.OpCopy(ir::Value::Register(dst + i, DataType::F32),
                               BUILDER.OpVectorExtract(res_v, i));
            }
        }

        HANDLE_PRED_COND_END();
    }
    INST(0xc038000000000000, 0xfc38000000000000) {
        COMMENT_NOT_IMPLEMENTED("tex");
    }
    INST(0xa000000000000000, 0xe000000000000000) {
        COMMENT_NOT_IMPLEMENTED("st");
    }
    INST(0x8000000000000000, 0xe000000000000000) {
        COMMENT_NOT_IMPLEMENTED("ld");
    }
    INST(0x7e80000000000000, 0xfe80000000000000) {
        COMMENT_NOT_IMPLEMENTED("hsetp2"); // TODO: needed by Cuphead
    }
    INST(0x7e00000000000000, 0xfe80000000000000) {
        COMMENT_NOT_IMPLEMENTED("hsetp2"); // TODO: needed by Cuphead
    }
    INST(0x7c80000000000000, 0xfe80000000000000) {
        COMMENT_NOT_IMPLEMENTED("hset2");
    }
    INST(0x7c00000000000000, 0xfe80000000000000) {
        COMMENT_NOT_IMPLEMENTED("hset2");
    }
    INST(0x7a00000000000000, 0xfe80000000000000) {
        // TODO: 5d10_0
        const auto dst = GET_REG(0);
        const bool negA = GET_BIT(43);
        // TODO: 5d10_1
        const auto srcA = GET_REG(8);
        const bool negB1 = GET_BIT(56);
        const auto srcB1 = GET_VALUE_F16(30);
        const bool negB0 = GET_BIT(29);
        const auto srcB0 = GET_VALUE_F16(20);
        COMMENT("hadd2 {} {}{} {}0x{:x} {}0x{:x}", dst, negA ? "-" : "", srcA,
                negB1 ? "-" : "", srcB1, negB0 ? "-" : "", srcB0);

        HANDLE_PRED_COND_BEGIN();

        auto srcB_v = BUILDER.OpVectorConstruct(
            DataType::F16,
            {NEG_IF(ir::Value::Immediate(srcB0, DataType::F16), negB0),
             NEG_IF(ir::Value::Immediate(srcB1, DataType::F16), negB1)});
        auto res = BUILDER.OpAdd(
            NEG_IF(ir::Value::Register(srcA, DataType::F16X2), negA), srcB_v);
        BUILDER.OpCopy(ir::Value::Register(dst, DataType::F16X2), res);

        HANDLE_PRED_COND_END();
    }
    INST(0x7a80000000000000, 0xfe80000000000000) {
        // TODO: 5d10_0
        const auto dst = GET_REG(0);
        const bool negA = GET_BIT(43);
        // TODO: 5d10_1
        const auto srcA = GET_REG(8);
        const bool negB = GET_BIT(56);
        const auto srcB = GET_CMEM(34, 14);
        COMMENT("hadd2 {} {}{} {}{}", dst, negA ? "-" : "", srcA,
                negB ? "-" : "", srcB);

        HANDLE_PRED_COND_BEGIN();

        auto res = BUILDER.OpAdd(
            NEG_IF(ir::Value::Register(srcA, DataType::F16X2), negA),
            NEG_IF(ir::Value::ConstMemory(srcB, DataType::F16X2), negB));
        BUILDER.OpCopy(ir::Value::Register(dst, DataType::F16X2), res);

        HANDLE_PRED_COND_END();
    }
    INST(0x7800000000000000, 0xfe80000000000000) {
        // TODO: 5d10_0
        const auto dst = GET_REG(0);
        const bool negA = GET_BIT(43);
        // TODO: 5d10_1
        const auto srcA = GET_REG(8);
        const bool negB1 = GET_BIT(56);
        const auto srcB1 = GET_VALUE_F16(30);
        const bool negB0 = GET_BIT(29);
        const auto srcB0 = GET_VALUE_F16(20);
        COMMENT("hmul2 {} {}{} {}0x{:x} {}0x{:x}", dst, negA ? "-" : "", srcA,
                negB1 ? "-" : "", srcB1, negB0 ? "-" : "", srcB0);

        HANDLE_PRED_COND_BEGIN();

        auto srcB_v = BUILDER.OpVectorConstruct(
            DataType::F16,
            {NEG_IF(ir::Value::Immediate(srcB0, DataType::F16), negB0),
             NEG_IF(ir::Value::Immediate(srcB1, DataType::F16), negB1)});
        auto res = BUILDER.OpMultiply(
            NEG_IF(ir::Value::Register(srcA, DataType::F16X2), negA), srcB_v);
        BUILDER.OpCopy(ir::Value::Register(dst, DataType::F16X2), res);

        HANDLE_PRED_COND_END();
    }
    INST(0x7880000000000000, 0xfe80000000000000) {
        // TODO: 5d10_0
        const auto dst = GET_REG(0);
        const bool negA = GET_BIT(43);
        // TODO: 5d10_1
        const auto srcA = GET_REG(8);
        const bool negB = GET_BIT(56);
        const auto srcB = GET_CMEM(34, 14);
        COMMENT("hmul2 {} {}{} {}{}", dst, negA ? "-" : "", srcA,
                negB ? "-" : "", srcB);

        HANDLE_PRED_COND_BEGIN();

        auto res = BUILDER.OpMultiply(
            NEG_IF(ir::Value::Register(srcA, DataType::F16X2), negA),
            NEG_IF(ir::Value::ConstMemory(srcB, DataType::F16X2), negB));
        BUILDER.OpCopy(ir::Value::Register(dst, DataType::F16X2), res);

        HANDLE_PRED_COND_END();
    }
    INST(0x7080000000000000, 0xf880000000000000) {
        // TODO: 6080_0
        // TODO: 5d10_0
        const auto dst = GET_REG(0);
        // TODO: 5d10_1
        const auto srcA = GET_REG(8);
        const bool negB = GET_BIT(56);
        const auto srcB = GET_CMEM(34, 14);
        const bool negC = GET_BIT(51);
        // TODO: 2c00_0
        const auto srcC = GET_REG(39);
        COMMENT("hfma2 {} {} {}{}", dst, srcA, negB ? "-" : "", srcB);

        HANDLE_PRED_COND_BEGIN();

        auto res = BUILDER.OpFma(
            ir::Value::Register(srcA, DataType::F16X2),
            NEG_IF(ir::Value::ConstMemory(srcB, DataType::F16X2), negB),
            NEG_IF(ir::Value::Register(srcC, DataType::F16X2), negC));
        BUILDER.OpCopy(ir::Value::Register(dst, DataType::F16X2), res);

        HANDLE_PRED_COND_END();
    }
    INST(0x7000000000000000, 0xf880000000000000) {
        // TODO: 6080_0
        // TODO: 5d10_0
        const auto dst = GET_REG(0);
        // TODO: 5d10_1
        const auto srcA = GET_REG(8);
        const bool negB1 = GET_BIT(56);
        const auto srcB1 = GET_VALUE_F16(30);
        const bool negB0 = GET_BIT(29);
        const auto srcB0 = GET_VALUE_F16(20);
        const bool negC = GET_BIT(51);
        // TODO: 2c00_0
        const auto srcC = GET_REG(39);
        COMMENT("hfma2 {} {} {}0x{:x} {}0x{:x} {}{}", dst, srcA,
                negB1 ? "-" : "", srcB1, negB0 ? "-" : "", srcB0,
                negC ? "-" : "", srcC);

        HANDLE_PRED_COND_BEGIN();

        auto srcB_v = BUILDER.OpVectorConstruct(
            DataType::F16,
            {NEG_IF(ir::Value::Immediate(srcB0, DataType::F16), negB0),
             NEG_IF(ir::Value::Immediate(srcB1, DataType::F16), negB1)});
        auto res = BUILDER.OpFma(
            ir::Value::Register(srcA, DataType::F16X2), srcB_v,
            NEG_IF(ir::Value::Register(srcC, DataType::F16X2), negC));
        BUILDER.OpCopy(ir::Value::Register(dst, DataType::F16X2), res);

        HANDLE_PRED_COND_END();
    }
    INST(0x6080000000000000, 0xf880000000000000) {
        // TODO: 6080_0
        // TODO: 5d10_0
        const auto dst = GET_REG(0);
        // TODO: 5d10_1
        const auto srcA = GET_REG(8);
        const bool negB = GET_BIT(56);
        // TODO: 2c00_0
        const auto srcB = GET_REG(39);
        const bool negC = GET_BIT(51);
        const auto srcC = GET_CMEM(34, 14);
        COMMENT("hfma2 {} {} {}{} {}c{}[0x{:x}]", dst, srcA, negB ? "-" : "",
                srcB, negC ? "-" : "", srcC.idx, srcC.imm);

        HANDLE_PRED_COND_BEGIN();

        auto res = BUILDER.OpFma(
            ir::Value::Register(srcA, DataType::F16X2),
            NEG_IF(ir::Value::Register(srcB, DataType::F16X2), negB),
            NEG_IF(ir::Value::ConstMemory(srcC, DataType::F16X2), negC));
        BUILDER.OpCopy(ir::Value::Register(dst, DataType::F16X2), res);

        HANDLE_PRED_COND_END();
    }
    INST(0x5f00000000000000, 0xff00000000000000) {
        COMMENT_NOT_IMPLEMENTED("vmad");
    }
    INST(0x5d20000000000000, 0xfff8000000000000) {
        COMMENT_NOT_IMPLEMENTED("hsetp2");
    }
    INST(0x5d18000000000000, 0xfff8000000000000) {
        COMMENT_NOT_IMPLEMENTED("hset2");
    }
    INST(0x5d10000000000000, 0xfff8000000000000) {
        // TODO: 6080_0
        // TODO: 5d10_0
        const auto dst = GET_REG(0);
        // TODO: 5d10_1
        const auto srcA = GET_REG(8);
        const bool negB = GET_BIT(31);
        // TODO: 5d10_2
        const auto srcB = GET_REG(20);
        COMMENT("hadd2 {} {} {}{}", dst, srcA, negB ? "-" : "", srcB);

        HANDLE_PRED_COND_BEGIN();

        auto res = BUILDER.OpAdd(
            ir::Value::Register(srcA, DataType::F16X2),
            NEG_IF(ir::Value::Register(srcB, DataType::F16X2), negB));
        BUILDER.OpCopy(ir::Value::Register(dst, DataType::F16X2), res);

        HANDLE_PRED_COND_END();
    }
    INST(0x5d08000000000000, 0xfff8000000000000) {
        // TODO: 6080_0
        // TODO: 5d10_0
        const auto dst = GET_REG(0);
        // TODO: 5d10_1
        const auto srcA = GET_REG(8);
        const bool negB = GET_BIT(31);
        // TODO: 5d10_2
        const auto srcB = GET_REG(20);
        COMMENT("hmul2 {} {} {}{}", dst, srcA, negB ? "-" : "", srcB);

        HANDLE_PRED_COND_BEGIN();

        auto res = BUILDER.OpMultiply(
            ir::Value::Register(srcA, DataType::F16X2),
            NEG_IF(ir::Value::Register(srcB, DataType::F16X2), negB));
        BUILDER.OpCopy(ir::Value::Register(dst, DataType::F16X2), res);

        HANDLE_PRED_COND_END();
    }
    INST(0x5d00000000000000, 0xfff8000000000000) {
        // TODO: 6080_0
        // TODO: 5d10_0
        const auto dst = GET_REG(0);
        // TODO: 5d10_1
        const auto srcA = GET_REG(8);
        const bool negB = GET_BIT(31);
        // TODO: 5d10_2
        const auto srcB = GET_REG(20);
        const bool negC = GET_BIT(30);
        // TODO: 5d00_1
        const auto srcC = GET_REG(39);
        COMMENT("hfma2 {} {} {}{} {}{}", dst, srcA, negB ? "-" : "", srcB,
                negC ? "-" : "", srcC);

        HANDLE_PRED_COND_BEGIN();

        auto res = BUILDER.OpFma(
            ir::Value::Register(srcA, DataType::F16X2),
            NEG_IF(ir::Value::Register(srcB, DataType::F16X2), negB),
            NEG_IF(ir::Value::Register(srcC, DataType::F16X2), negC));
        BUILDER.OpCopy(ir::Value::Register(dst, DataType::F16X2), res);

        HANDLE_PRED_COND_END();
    }
    INST(0x5cf8000000000000, 0xfff8000000000000) {
        COMMENT_NOT_IMPLEMENTED("shf");
    }
    INST(0x5cf0000000000000, 0xfff8000000000000) {
        COMMENT_NOT_IMPLEMENTED("r2p");
    }
    INST(0x5ce8000000000000, 0xfff8000000000000) {
        COMMENT_NOT_IMPLEMENTED("p2r");
    }
    INST(0x5ce0000000000000, 0xfff8000000000000) {
        // TODO: is dst and src type correct?
        const auto dst_type = get_operand_5ce0_0(inst);
        const auto src_type = get_operand_5ce0_1(inst);
        const auto dst = GET_REG(0);
        const auto src = GET_REG(20);
        COMMENT("i2i {} {} {} {}", dst_type, src_type, dst, src);

        HANDLE_PRED_COND_BEGIN();

        auto res = BUILDER.OpCast(ir::Value::Register(src, src_type), dst_type);
        BUILDER.OpCopy(ir::Value::Register(dst, dst_type), res);

        HANDLE_PRED_COND_END();
    }
    INST(0x5cc0000000000000, 0xfff0000000000000) {
        COMMENT_NOT_IMPLEMENTED("iadd3");
    }
    INST(0x5cb8000000000000, 0xfff8000000000000) {
        const auto dst_type = get_operand_5cb8_0(inst);
        const auto src_type = get_operand_5cb8_1(inst);
        // TODO: 5cb8_2
        const auto dst = GET_REG(0);
        const auto neg = GET_BIT(45);
        const auto src = GET_REG(20);
        COMMENT("i2f {} {} {} {}{}", dst_type, src_type, dst, neg ? "-" : "",
                src);

        HANDLE_PRED_COND_BEGIN();

        auto res = BUILDER.OpCast(
            NEG_IF(ir::Value::Register(src, src_type), neg), dst_type);
        BUILDER.OpCopy(ir::Value::Register(dst, dst_type), res);

        HANDLE_PRED_COND_END();
    }
    INST(0x5cb0000000000000, 0xfff8000000000000) {
        const auto dst_type = get_operand_5cb0_2(inst);
        const auto src_type = get_operand_5cb0_0(inst);
        const auto mode = get_operand_5cb0_1(inst);
        const auto dst = GET_REG(0);
        const auto neg = GET_BIT(45);
        const auto src = GET_REG(20);
        COMMENT("f2i {} {} {} {} {}{}", dst_type, src_type, mode, dst,
                neg ? "-" : "", src);

        HANDLE_PRED_COND_BEGIN();

        // TODO: negate before or after?
        auto src_v = NEG_IF(ir::Value::Register(src, src_type), neg);
        switch (mode) {
        case IntegerRoundMode::Round:
            unreachable();
        case IntegerRoundMode::Floor:
            src_v = BUILDER.OpFloor(src_v);
            break;
        case IntegerRoundMode::Ceil:
            src_v = BUILDER.OpCeil(src_v);
            break;
        case IntegerRoundMode::Trunc:
            src_v = BUILDER.OpTrunc(src_v);
            break;
        default:
            break;
        }

        auto res = BUILDER.OpCast(src_v, dst_type);
        BUILDER.OpCopy(ir::Value::Register(dst, dst_type), res);

        HANDLE_PRED_COND_END();
    }
    INST(0x5ca8000000000000, 0xfff8000000000000) {
        const auto dst_type = get_operand_5cb0_0(inst);
        const auto src_type = get_operand_5cb8_0(inst);
        const auto mode = get_operand_5ca8_0(inst);
        const auto dst = GET_REG(0);
        const auto src = GET_REG(20);
        COMMENT("f2f {} {} {} {} {}", dst_type, src_type, mode, dst, src);

        HANDLE_PRED_COND_BEGIN();

        auto src_v = ir::Value::Register(src, src_type);
        switch (mode) {
        case IntegerRoundMode::Round:
            src_v = BUILDER.OpRound(src_v);
            break;
        case IntegerRoundMode::Floor:
            src_v = BUILDER.OpFloor(src_v);
            break;
        case IntegerRoundMode::Ceil:
            src_v = BUILDER.OpCeil(src_v);
            break;
        case IntegerRoundMode::Trunc:
            src_v = BUILDER.OpTrunc(src_v);
            break;
        default:
            break;
        }

        auto res = BUILDER.OpCast(src_v, dst_type);
        BUILDER.OpCopy(ir::Value::Register(dst, dst_type), res);

        HANDLE_PRED_COND_END();
    }
    INST(0x5ca0000000000000, 0xfff8000000000000) {
        COMMENT_NOT_IMPLEMENTED("sel");
    }
    INST(0x5c98000000000000, 0xfff8000000000000) {
        const auto dst = GET_REG(0);
        const auto src = GET_REG(20);
        const auto todo = GET_VALUE_U32(39, 4); // TODO: what is this?
        COMMENT("mov {} {} 0x{:x}", dst, src, todo);

        HANDLE_PRED_COND_BEGIN();

        BUILDER.OpCopy(ir::Value::Register(dst), ir::Value::Register(src));

        HANDLE_PRED_COND_END();
    }
    INST(0x5c90000000000000, 0xfff8000000000000) {
        // TODO: mode
        const auto dst = GET_REG(0);
        const auto src = GET_REG(20);
        COMMENT("rro {} {}", dst, src);

        HANDLE_PRED_COND_BEGIN();

        // TODO: is it okay to just move?
        BUILDER.OpCopy(ir::Value::Register(dst), ir::Value::Register(src));

        HANDLE_PRED_COND_END();
    }
    INST(0x5c88000000000000, 0xfff8000000000000) {
        COMMENT_NOT_IMPLEMENTED("fchk");
    }
    INST(0x5c80000000000000, 0xfff8000000000000) {
        COMMENT_NOT_IMPLEMENTED("dmul");
    }
    INST(0x5c70000000000000, 0xfff8000000000000) {
        COMMENT_NOT_IMPLEMENTED("dadd");
    }
    INST(0x5c68000000000000, 0xfff8000000000000) {
        const auto dst = GET_REG(0);
        const auto srcA = GET_REG(8);
        const auto srcB = GET_REG(20);
        COMMENT("fmul {} {} {}", dst, srcA, srcB);

        HANDLE_PRED_COND_BEGIN();

        auto res = BUILDER.OpMultiply(ir::Value::Register(srcA, DataType::F32),
                                      ir::Value::Register(srcB, DataType::F32));
        BUILDER.OpCopy(ir::Value::Register(dst, DataType::F32), res);

        HANDLE_PRED_COND_END();
    }
    INST(0x5c60000000000000, 0xfff8000000000000) {
        const auto dst = GET_REG(0);
        const auto srcA = GET_REG(8);
        const auto srcB = GET_REG(20);
        const auto pred = GET_PRED(39);
        const bool pred_not = GET_BIT(42);
        COMMENT("fmnmx {} {} {} {}{}", dst, srcA, srcB, (pred_not ? "!" : ""),
                pred);

        HANDLE_PRED_COND_BEGIN();

        auto srcA_v = ir::Value::Register(srcA, DataType::F32);
        auto srcB_v = ir::Value::Register(srcB, DataType::F32);
        auto min_v = BUILDER.OpMin(srcA_v, srcB_v);
        auto max_v = BUILDER.OpMax(srcA_v, srcB_v);
        auto res = BUILDER.OpSelect(
            NOT_IF(ir::Value::Predicate(pred), pred_not), min_v, max_v);
        BUILDER.OpCopy(ir::Value::Register(dst, DataType::F32), res);

        HANDLE_PRED_COND_END();
    }
    INST(0x5c58000000000000, 0xfff8000000000000) {
        const auto dst = GET_REG(0);
        const bool negA = GET_BIT(48);
        const auto srcA = GET_REG(8);
        const bool negB = GET_BIT(45);
        const auto srcB = GET_REG(20);
        COMMENT("fadd {} {}{} {}{}", dst, negA ? "-" : "", srcA,
                negB ? "-" : "", srcB);

        HANDLE_PRED_COND_BEGIN();

        auto res = BUILDER.OpAdd(
            NEG_IF(ir::Value::Register(srcA, DataType::F32), negA),
            NEG_IF(ir::Value::Register(srcB, DataType::F32), negB));
        BUILDER.OpCopy(ir::Value::Register(dst, DataType::F32), res);

        HANDLE_PRED_COND_END();
    }
    INST(0x5c50000000000000, 0xfff8000000000000) {
        COMMENT_NOT_IMPLEMENTED("dmnmx");
    }
    INST(0x5c48000000000000, 0xfff8000000000000) {
        COMMENT_NOT_IMPLEMENTED("shl");
    }
    INST(0x5c40000000000000, 0xfff8000000000000) {
        const auto bin = get_operand_5c40_0(inst);
        const auto pred_op = get_operand_5c40_1(inst);
        const auto dst_pred = GET_PRED(48);
        const auto dst = GET_REG(0);
        const auto invA = GET_BIT(39);
        const auto srcA = GET_REG(8);
        const auto invB = GET_BIT(40);
        const auto srcB = GET_REG(20);
        COMMENT("lop {} {} {} {}{} {}{}", bin, pred_op, dst, (invA ? "!" : ""),
                srcA, (invB ? "!" : ""), srcB);

        HANDLE_PRED_COND_BEGIN();

        // TODO: inv
        auto srcB_v = ir::Value::Register(srcB);
        auto res =
            (bin == BitwiseOp::PassB
                 ? srcB_v
                 : BUILDER.OpBitwise(bin, ir::Value::Register(srcA), srcB_v));
        BUILDER.OpCopy(ir::Value::Register(dst), res);

        std::optional<ir::Value> pred_v;
        switch (pred_op) {
        case PredOp::False:
            pred_v = ir::Value::Immediate(0); // TODO: false
            break;
        case PredOp::True:
            pred_v = ir::Value::Immediate(1); // TODO: true
            break;
        case PredOp::Zero:
            pred_v = BUILDER.OpCompare(ComparisonOp::Equal, res,
                                       ir::Value::Immediate(0));
            break;
        case PredOp::NotZero:
            pred_v = BUILDER.OpCompare(ComparisonOp::NotEqual, res,
                                       ir::Value::Immediate(0));
            break;
        default:
            pred_v = std::nullopt;
            break;
        }
        BUILDER.OpCopy(ir::Value::Predicate(dst_pred), pred_v.value());

        HANDLE_PRED_COND_END();
    }
    INST(0x5c38000000000000, 0xfff8000000000000) {
        COMMENT_NOT_IMPLEMENTED("imul");
    }
    INST(0x5c30000000000000, 0xfff8000000000000) {
        COMMENT_NOT_IMPLEMENTED("flo");
    }
    INST(0x5c28000000000000, 0xfff8000000000000) {
        COMMENT_NOT_IMPLEMENTED("shr");
    }
    INST(0x5c20000000000000, 0xfff8000000000000) {
        COMMENT_NOT_IMPLEMENTED("imnmx");
    }
    INST(0x5c18000000000000, 0xfff8000000000000) {
        const auto dst = GET_REG(0);
        const bool negA = GET_BIT(49);
        const auto srcA = GET_REG(8);
        const bool negB = GET_BIT(48);
        const auto srcB = GET_REG(20);
        const auto shift = GET_VALUE_U32(39, 5);
        COMMENT("iscadd {} {}{} {}{} 0x{:x}", dst, (negA ? "-" : ""), srcA,
                (negB ? "-" : ""), srcB, shift);

        HANDLE_PRED_COND_BEGIN();

        auto srcA_v = ir::Value::Register(srcA, DataType::I32);
        srcA_v = BUILDER.OpShiftLeft(srcA_v, shift);
        // TODO: negA

        auto res = BUILDER.OpAdd(
            srcA_v, NEG_IF(ir::Value::Register(srcB, DataType::I32), negB));
        BUILDER.OpCopy(ir::Value::Register(dst, DataType::I32), res);

        HANDLE_PRED_COND_END();
    }
    INST(0x5c10000000000000, 0xfff8000000000000) {
        const auto dst = GET_REG(0);
        const bool negA = GET_BIT(49);
        const auto srcA = GET_REG(8);
        const bool negB = GET_BIT(48);
        const auto srcB = GET_REG(20);
        COMMENT("iadd {} {}{} {}{}", dst, (negA ? "-" : ""), srcA,
                (negB ? "-" : ""), srcB);

        HANDLE_PRED_COND_BEGIN();

        auto res = BUILDER.OpAdd(
            NEG_IF(ir::Value::Register(srcA, DataType::I32), negA),
            NEG_IF(ir::Value::Register(srcB, DataType::I32), negB));
        BUILDER.OpCopy(ir::Value::Register(dst, DataType::I32), res);

        HANDLE_PRED_COND_END();
    }
    INST(0x5c08000000000000, 0xfff8000000000000) {
        COMMENT_NOT_IMPLEMENTED("popc");
    }
    INST(0x5c00000000000000, 0xfff8000000000000) {
        COMMENT_NOT_IMPLEMENTED("bfe");
    }
    INST(0x5bf8000000000000, 0xfff8000000000000) {
        COMMENT_NOT_IMPLEMENTED("shf");
    }
    INST(0x5bf0000000000000, 0xfff8000000000000) {
        COMMENT_NOT_IMPLEMENTED("bfi");
    }
    INST(0x5be0000000000000, 0xfff8000000000000) {
        COMMENT_NOT_IMPLEMENTED("lop3");
    }
    INST(0x5bd8000000000000, 0xfff8000000000000) {
        COMMENT_NOT_IMPLEMENTED("lea");
    }
    INST(0x5bd0000000000000, 0xfff8000000000000) {
        COMMENT_NOT_IMPLEMENTED("lea");
    }
    INST(0x5bc0000000000000, 0xfff0000000000000) {
        COMMENT_NOT_IMPLEMENTED("prmt");
    }
    INST(0x5bb0000000000000, 0xfff0000000000000) {
        const auto cmp = get_operand_5bb0_0(inst);
        const auto combine_bin = get_operand_5bb0_1(inst);
        const auto dst = GET_PRED(3);
        const auto combine = GET_PRED(0); // TODO: combine?
        const auto srcA = GET_REG(8);
        const auto srcB = GET_REG(20);
        // TODO: pred 39
        COMMENT("fsetp {} {} {} {} {} {}", cmp, combine_bin, dst, combine, srcA,
                srcB);

        HANDLE_PRED_COND_BEGIN();

        auto cmp_res =
            BUILDER.OpCompare(cmp, ir::Value::Register(srcA, DataType::F32),
                              ir::Value::Register(srcB, DataType::F32));
        auto bin_res = BUILDER.OpBitwise(combine_bin, cmp_res,
                                         ir::Value::Predicate(combine));
        BUILDER.OpCopy(ir::Value::Predicate(dst), bin_res);

        HANDLE_PRED_COND_END();
    }
    INST(0x5ba0000000000000, 0xfff0000000000000) {
        COMMENT_NOT_IMPLEMENTED("fcmp");
    }
    INST(0x5b80000000000000, 0xfff0000000000000) {
        COMMENT_NOT_IMPLEMENTED("dsetp");
    }
    INST(0x5b70000000000000, 0xfff0000000000000) {
        COMMENT_NOT_IMPLEMENTED("dfma");
    }
    INST(0x5b60000000000000, 0xfff0000000000000) {
        const auto cmp = get_operand_5b60_0(inst);
        const auto type = get_operand_5c30_0(inst);
        const auto combine_bin = get_operand_5bb0_1(inst);
        const auto dst = GET_PRED(3);
        const auto combine = GET_PRED(0);
        const auto srcA = GET_REG(8);
        const auto srcB = GET_REG(20);
        // TODO: pred 39
        COMMENT("isetp {} {} {} {} {} {} {}", cmp, type, combine_bin, dst,
                combine, srcA, srcB);

        HANDLE_PRED_COND_BEGIN();

        auto cmp_res = BUILDER.OpCompare(cmp, ir::Value::Register(srcA, type),
                                         ir::Value::Register(srcB, type));
        auto bin_res = BUILDER.OpBitwise(combine_bin, cmp_res,
                                         ir::Value::Predicate(combine));
        BUILDER.OpCopy(ir::Value::Predicate(dst), bin_res);

        HANDLE_PRED_COND_END();
    }
    INST(0x5b50000000000000, 0xfff0000000000000) {
        const auto bool_float = GET_BIT(44);
        const auto cmp = get_operand_5b60_0(inst);
        const auto type = get_operand_5c30_0(inst);
        const auto combine_bin = get_operand_5bb0_1(inst);
        const auto dst = GET_REG(0);
        const auto srcA = GET_REG(8);
        const auto srcB = GET_REG(20);
        const auto combine = GET_PRED(39);
        COMMENT("iset {} {} {} {} {} {} {}", cmp, type, combine_bin, dst, srcA,
                srcB, combine);

        HANDLE_PRED_COND_BEGIN();

        auto cmp_res = BUILDER.OpCompare(cmp, ir::Value::Register(srcA, type),
                                         ir::Value::Register(srcB, type));
        auto bin_res = BUILDER.OpBitwise(combine_bin, cmp_res,
                                         ir::Value::Predicate(combine));
        if (bool_float) {
            // TODO: simplify immediate value creation
            auto res = BUILDER.OpSelect(
                bin_res,
                ir::Value::Immediate(std::bit_cast<u32>(f32(1.0f)),
                                     DataType::F32),
                ir::Value::Immediate(std::bit_cast<u32>(f32(0.0f)),
                                     DataType::F32));
            BUILDER.OpCopy(ir::Value::Register(dst, DataType::F32), res);
        } else {
            auto res = BUILDER.OpCast(bin_res, DataType::U32);
            BUILDER.OpCopy(ir::Value::Register(dst), res);
        }

        HANDLE_PRED_COND_END();
    }
    INST(0x5b40000000000000, 0xfff0000000000000) {
        COMMENT_NOT_IMPLEMENTED("icmp");
    }
    INST(0x5b00000000000000, 0xffc0000000000000) {
        COMMENT_NOT_IMPLEMENTED("xmad");
    }
    INST(0x5a80000000000000, 0xff80000000000000) {
        COMMENT_NOT_IMPLEMENTED("imadsp");
    }
    INST(0x5a00000000000000, 0xff80000000000000) {
        COMMENT_NOT_IMPLEMENTED("imad");
    }
    INST(0x5980000000000000, 0xff80000000000000) {
        const bool saturate = GET_BIT(50);
        const auto dst = GET_REG(0);
        const auto srcA = GET_REG(8);
        const auto negB = GET_BIT(48);
        const auto srcB = GET_REG(20);
        const auto negC = GET_BIT(49);
        const auto srcC = GET_REG(39);
        COMMENT("ffma {} {} {} {}{} {}{}", (saturate ? "sat " : ""), dst, srcA,
                (negB ? "-" : ""), srcB, (negC ? "-" : ""), srcC);

        HANDLE_PRED_COND_BEGIN();

        auto res = BUILDER.OpFma(
            ir::Value::Register(srcA, DataType::F32),
            NEG_IF(ir::Value::Register(srcB, DataType::F32), negB),
            NEG_IF(ir::Value::Register(srcC, DataType::F32), negC));
        if (saturate)
            res = BUILDER.OpClamp(
                res,
                ir::Value::Immediate(std::bit_cast<u32>(0.0f), DataType::F32),
                ir::Value::Immediate(std::bit_cast<u32>(1.0f), DataType::F32));
        BUILDER.OpCopy(ir::Value::Register(dst, DataType::F32), res);

        HANDLE_PRED_COND_END();
    }
    INST(0x5900000000000000, 0xff80000000000000) {
        COMMENT_NOT_IMPLEMENTED("dset");
    }
    INST(0x5800000000000000, 0xff00000000000000) {
        const auto bool_float = GET_BIT(52);
        const auto cmp = get_operand_5bb0_0(inst);
        const auto combine_bin = get_operand_5bb0_1(inst);
        const auto dst = GET_REG(0);
        const auto srcA = GET_REG(8);
        const auto negA = GET_BIT(43);
        const auto srcB = GET_REG(20);
        const auto negB = GET_BIT(53);
        // TODO: combine
        COMMENT("fset {} {} {} {} {}{} {}{}", (bool_float ? "BF" : ""), cmp,
                combine_bin, dst, (negA ? "-" : ""), srcA, (negB ? "-" : ""),
                srcB);

        HANDLE_PRED_COND_BEGIN();

        auto cmp_res = BUILDER.OpCompare(
            cmp, NEG_IF(ir::Value::Register(srcA, DataType::F32), negA),
            NEG_IF(ir::Value::Register(srcB, DataType::F32), negB));
        // TODO: uncomment
        // auto bin_res = BUILDER.OpBitwise(combine_bin, cmp_res,
        //                    ir::Value::Predicate(combine));
        // TODO: use bin_res instead of cmp_res
        // TODO: simplify immediate value creation
        if (bool_float) {
            auto res = BUILDER.OpSelect(
                cmp_res,
                ir::Value::Immediate(std::bit_cast<u32>(f32(1.0f)),
                                     DataType::F32),
                ir::Value::Immediate(std::bit_cast<u32>(f32(0.0f)),
                                     DataType::F32));
            BUILDER.OpCopy(ir::Value::Register(dst, DataType::F32), res);
        } else {
            auto res = BUILDER.OpCast(cmp_res, DataType::U32);
            BUILDER.OpCopy(ir::Value::Register(dst), res);
        }

        HANDLE_PRED_COND_END();
    }
    INST(0x5700000000000000, 0xff00000000000000) {
        COMMENT_NOT_IMPLEMENTED("vshl");
    }
    INST(0x5600000000000000, 0xff00000000000000) {
        COMMENT_NOT_IMPLEMENTED("vshr");
    }
    INST(0x5400000000000000, 0xff00000000000000) {
        COMMENT_NOT_IMPLEMENTED("vabsdiff");
    }
    INST(0x53f8000000000000, 0xfff8000000000000) {
        COMMENT_NOT_IMPLEMENTED("idp");
    }
    INST(0x53f0000000000000, 0xfff8000000000000) {
        COMMENT_NOT_IMPLEMENTED("bfi");
    }
    INST(0x53d8000000000000, 0xfff8000000000000) {
        COMMENT_NOT_IMPLEMENTED("idp");
    }
    INST(0x53c0000000000000, 0xfff0000000000000) {
        COMMENT_NOT_IMPLEMENTED("prmt");
    }
    INST(0x53a0000000000000, 0xfff0000000000000) {
        COMMENT_NOT_IMPLEMENTED("fcmp");
    }
    INST(0x5370000000000000, 0xfff0000000000000) {
        COMMENT_NOT_IMPLEMENTED("dfma");
    }
    INST(0x5340000000000000, 0xfff0000000000000) {
        COMMENT_NOT_IMPLEMENTED("icmp");
    }
    INST(0x5280000000000000, 0xff80000000000000) {
        COMMENT_NOT_IMPLEMENTED("imadsp");
    }
    INST(0x5200000000000000, 0xff80000000000000) {
        COMMENT_NOT_IMPLEMENTED("imad");
    }
    INST(0x5180000000000000, 0xff80000000000000) {
        const bool saturate = GET_BIT(50);
        const auto dst = GET_REG(0);
        const auto srcA = GET_REG(8);
        const auto negB = GET_BIT(48);
        const auto srcB = GET_REG(39);
        const auto negC = GET_BIT(49);
        const auto srcC = GET_CMEM(34, 14);
        COMMENT("ffma {} {} {} {}{} {}c{}[0x{:x}]", (saturate ? "sat " : ""),
                dst, srcA, (negB ? "-" : ""), srcB, (negC ? "-" : ""), srcC.idx,
                srcC.imm);

        HANDLE_PRED_COND_BEGIN();

        auto res = BUILDER.OpFma(
            ir::Value::Register(srcA, DataType::F32),
            NEG_IF(ir::Value::Register(srcB, DataType::F32), negB),
            NEG_IF(ir::Value::ConstMemory(srcC, DataType::F32), negC));
        if (saturate)
            res = BUILDER.OpClamp(
                res,
                ir::Value::Immediate(std::bit_cast<u32>(0.0f), DataType::F32),
                ir::Value::Immediate(std::bit_cast<u32>(1.0f), DataType::F32));
        BUILDER.OpCopy(ir::Value::Register(dst, DataType::F32), res);

        HANDLE_PRED_COND_END();
    }
    INST(0x5100000000000000, 0xff80000000000000) {
        COMMENT_NOT_IMPLEMENTED("xmad");
    }
    INST(0x50f8000000000000, 0xfff8000000000000) {
        COMMENT_NOT_IMPLEMENTED("fswzadd");
    }
    INST(0x50f0000000000000, 0xfff8000000000000) {
        COMMENT_NOT_IMPLEMENTED("vsetp");
    }
    INST(0x50e0000000000000, 0xfff8000000000000) {
        COMMENT_NOT_IMPLEMENTED("vote");
    }
    INST(0x50d8000000000000, 0xfff8000000000000) {
        COMMENT_NOT_IMPLEMENTED("vote");
    }
    INST(0x50d0000000000000, 0xfff8000000000000) {
        COMMENT_NOT_IMPLEMENTED("lepc");
    }
    INST(0x50c8000000000000, 0xfff8000000000000) {
        COMMENT_NOT_IMPLEMENTED("cs2r");
    }
    INST(0x50b0000000000000, 0xfff8000000000000) { COMMENT("nop"); }
    INST(0x50a0000000000000, 0xfff8000000000000) {
        COMMENT_NOT_IMPLEMENTED("csetp");
    }
    INST(0x5098000000000000, 0xfff8000000000000) {
        COMMENT_NOT_IMPLEMENTED("cset");
    }
    INST(0x5090000000000000, 0xfff8000000000000) {
        const auto bin = get_operand_5090_0(inst);
        const auto combine_bin = get_operand_5bb0_1(inst);
        const auto dst = GET_PRED(3);
        const auto combine = GET_PRED(0); // TODO: combine?
        const auto srcA = GET_PRED(12);
        const auto srcB = GET_PRED(29);
        const auto srcC = GET_PRED(39);
        COMMENT("psetp {} {} {} {} {} {} {}", bin, combine_bin, dst, combine,
                srcA, srcB, srcC);

        HANDLE_PRED_COND_BEGIN();

        auto bin1_res = BUILDER.OpBitwise(bin, ir::Value::Predicate(srcA),
                                          ir::Value::Predicate(srcB));
        auto bin2_res =
            BUILDER.OpBitwise(bin, bin1_res, ir::Value::Predicate(srcC));
        auto bin3_res = BUILDER.OpBitwise(combine_bin, bin2_res,
                                          ir::Value::Predicate(combine));
        BUILDER.OpCopy(ir::Value::Predicate(dst), bin3_res);

        HANDLE_PRED_COND_END();
    }
    INST(0x5088000000000000, 0xfff8000000000000) {
        COMMENT_NOT_IMPLEMENTED("pset");
    }
    INST(0x5080000000000000, 0xfff8000000000000) {
        const auto func = get_operand_5080_0(inst);
        const auto dst = GET_REG(0);
        const auto src = GET_REG(8);
        COMMENT("mufu {} {} {}", func, dst, src);

        HANDLE_PRED_COND_BEGIN();

        auto res = BUILDER.OpMathFunction(
            func, ir::Value::Register(src, DataType::F32));
        BUILDER.OpCopy(ir::Value::Register(dst, DataType::F32), res);

        HANDLE_PRED_COND_END();
    }
    INST(0x5000000000000000, 0xff80000000000000) {
        COMMENT_NOT_IMPLEMENTED("vabsdiff4");
    }
    INST(0x4e00000000000000, 0xfe00000000000000) {
        COMMENT_NOT_IMPLEMENTED("xmad");
    }
    INST(0x4cf0000000000000, 0xfff8000000000000) {
        COMMENT_NOT_IMPLEMENTED("r2p");
    }
    INST(0x4ce8000000000000, 0xfff8000000000000) {
        COMMENT_NOT_IMPLEMENTED("p2r");
    }
    INST(0x4ce0000000000000, 0xfff8000000000000) {
        // TODO: is dst and src type correct?
        const auto dst_type = get_operand_5ce0_0(inst);
        const auto src_type = get_operand_5ce0_1(inst);
        const auto dst = GET_REG(0);
        const auto src = GET_CMEM(34, 14);
        COMMENT("i2i {} {} {} c{}[0x{:x}]", dst_type, src_type, dst, src.idx,
                src.imm);

        HANDLE_PRED_COND_BEGIN();

        auto res =
            BUILDER.OpCast(ir::Value::ConstMemory(src, src_type), dst_type);
        BUILDER.OpCopy(ir::Value::Register(dst, dst_type), res);

        HANDLE_PRED_COND_END();
    }
    INST(0x4cc0000000000000, 0xfff0000000000000) {
        COMMENT_NOT_IMPLEMENTED("iadd3");
    }
    INST(0x4cb8000000000000, 0xfff8000000000000) {
        const auto dst_type = get_operand_5cb8_0(inst);
        const auto src_type = get_operand_5cb8_1(inst);
        // TODO: 5cb8_2
        const auto dst = GET_REG(0);
        const auto neg = GET_BIT(45);
        const auto src = GET_CMEM(34, 14);
        COMMENT("i2f {} {} {} {}{}", dst_type, src_type, dst, (neg ? "-" : ""),
                src);

        HANDLE_PRED_COND_BEGIN();

        auto res = BUILDER.OpCast(
            NEG_IF(ir::Value::ConstMemory(src, src_type), neg), dst_type);
        BUILDER.OpCopy(ir::Value::Register(dst, dst_type), res);

        HANDLE_PRED_COND_END();
    }
    INST(0x4cb0000000000000, 0xfff8000000000000) {
        COMMENT_NOT_IMPLEMENTED("f2i");
    }
    INST(0x4ca8000000000000, 0xfff8000000000000) {
        COMMENT_NOT_IMPLEMENTED("f2f");
    }
    INST(0x4ca0000000000000, 0xfff8000000000000) {
        const auto dst = GET_REG(0);
        const auto srcA = GET_REG(8);
        const auto srcB = GET_CMEM(34, 14);
        const auto pred = GET_PRED(39);
        COMMENT("sel {} {} {} {}", dst, srcA, srcB, pred);

        HANDLE_PRED_COND_BEGIN();

        auto res = BUILDER.OpSelect(ir::Value::Predicate(pred),
                                    ir::Value::Register(srcA),
                                    ir::Value::ConstMemory(srcB));
        BUILDER.OpCopy(ir::Value::Register(dst), res);

        HANDLE_PRED_COND_END();
    }
    INST(0x4c98000000000000, 0xfff8000000000000) {
        const auto dst = GET_REG(0);
        const auto src = GET_CMEM(34, 14);
        const auto todo = GET_VALUE_U32(39, 4);
        COMMENT("mov {} {} 0x{:x}", dst, src, todo);

        HANDLE_PRED_COND_BEGIN();

        BUILDER.OpCopy(ir::Value::Register(dst), ir::Value::ConstMemory(src));

        HANDLE_PRED_COND_END();
    }
    INST(0x4c90000000000000, 0xfff8000000000000) {
        // TODO: 5c90_0
        const auto dst = GET_REG(0);
        const auto src = GET_CMEM(34, 14);
        COMMENT("rro {} {}", dst, src);

        HANDLE_PRED_COND_BEGIN();

        // This should always be followed by a corresponding MUFU instruction,
        // so a simple copy should be sufficient
        BUILDER.OpCopy(ir::Value::Register(dst), ir::Value::ConstMemory(src));

        HANDLE_PRED_COND_END();
    }
    INST(0x4c88000000000000, 0xfff8000000000000) {
        COMMENT_NOT_IMPLEMENTED("fchk");
    }
    INST(0x4c80000000000000, 0xfff8000000000000) {
        COMMENT_NOT_IMPLEMENTED("dmul");
    }
    INST(0x4c70000000000000, 0xfff8000000000000) {
        COMMENT_NOT_IMPLEMENTED("dadd");
    }
    INST(0x4c68000000000000, 0xfff8000000000000) {
        const auto dst = GET_REG(0);
        const auto srcA = GET_REG(8);
        const auto srcB = GET_CMEM(34, 14);
        COMMENT("fmul {} {} c{}[0x{:x}]", dst, srcA, srcB.idx, srcB.imm);

        HANDLE_PRED_COND_BEGIN();

        auto res =
            BUILDER.OpMultiply(ir::Value::Register(srcA, DataType::F32),
                               ir::Value::ConstMemory(srcB, DataType::F32));
        BUILDER.OpCopy(ir::Value::Register(dst, DataType::F32), res);

        HANDLE_PRED_COND_END();
    }
    INST(0x4c60000000000000, 0xfff8000000000000) {
        const auto dst = GET_REG(0);
        const auto negA = GET_BIT(48);
        const auto srcA = GET_REG(8);
        const auto negB = GET_BIT(45);
        const auto srcB = GET_CMEM(34, 14);
        const auto pred = GET_PRED(39);
        const bool pred_not = GET_BIT(42);
        COMMENT("fmnmx {} {}{} {}c{}[0x{:x}] {}{}", dst, (negA ? "-" : ""),
                srcA, (negB ? "-" : ""), srcB.idx, srcB.imm,
                (pred_not ? "!" : ""), pred);

        HANDLE_PRED_COND_BEGIN();

        auto srcA_v = NEG_IF(ir::Value::Register(srcA, DataType::F32), negA);
        auto srcB_v = NEG_IF(ir::Value::ConstMemory(srcB, DataType::F32), negB);
        auto min_v = BUILDER.OpMin(srcA_v, srcB_v);
        auto max_v = BUILDER.OpMax(srcA_v, srcB_v);
        auto res = BUILDER.OpSelect(
            NOT_IF(ir::Value::Predicate(pred), pred_not), min_v, max_v);
        BUILDER.OpCopy(ir::Value::Register(dst, DataType::F32), res);

        HANDLE_PRED_COND_END();
    }
    INST(0x4c58000000000000, 0xfff8000000000000) {
        const auto dst = GET_REG(0);
        const bool negA = GET_BIT(48);
        const auto srcA = GET_REG(8);
        const bool negB = GET_BIT(45);
        const auto srcB = GET_CMEM(34, 14);
        COMMENT("fadd {} {}{} {}c{}[0x{:x}]", dst, (negA ? "-" : ""), srcA,
                (negB ? "-" : ""), srcB.idx, srcB.imm);

        HANDLE_PRED_COND_BEGIN();

        auto res = BUILDER.OpAdd(
            NEG_IF(ir::Value::Register(srcA, DataType::F32), negA),
            NEG_IF(ir::Value::ConstMemory(srcB, DataType::F32), negB));
        BUILDER.OpCopy(ir::Value::Register(dst, DataType::F32), res);

        HANDLE_PRED_COND_END();
    }
    INST(0x4c50000000000000, 0xfff8000000000000) {
        COMMENT_NOT_IMPLEMENTED("dmnmx");
    }
    INST(0x4c48000000000000, 0xfff8000000000000) {
        COMMENT_NOT_IMPLEMENTED("shl");
    }
    INST(0x4c40000000000000, 0xfff8000000000000) {
        COMMENT_NOT_IMPLEMENTED("lop");
    }
    INST(0x4c38000000000000, 0xfff8000000000000) {
        COMMENT_NOT_IMPLEMENTED("imul");
    }
    INST(0x4c30000000000000, 0xfff8000000000000) {
        COMMENT_NOT_IMPLEMENTED("flo");
    }
    INST(0x4c28000000000000, 0xfff8000000000000) {
        COMMENT_NOT_IMPLEMENTED("shr");
    }
    INST(0x4c20000000000000, 0xfff8000000000000) {
        COMMENT_NOT_IMPLEMENTED("imnmx");
    }
    INST(0x4c18000000000000, 0xfff8000000000000) {
        COMMENT_NOT_IMPLEMENTED("iscadd");
    }
    INST(0x4c10000000000000, 0xfff8000000000000) {
        const auto dst = GET_REG(0);
        const bool negA = GET_BIT(49);
        const auto srcA = GET_REG(8);
        const bool negB = GET_BIT(48);
        const auto srcB = GET_CMEM(34, 14);
        COMMENT("iadd {} {}{} {}c{}[0x{:x}]", dst, (negA ? "-" : ""), srcA,
                (negB ? "-" : ""), srcB.idx, srcB.imm);

        HANDLE_PRED_COND_BEGIN();

        auto res = BUILDER.OpAdd(
            NEG_IF(ir::Value::Register(srcA, DataType::I32), negA),
            NEG_IF(ir::Value::ConstMemory(srcB, DataType::I32), negB));
        BUILDER.OpCopy(ir::Value::Register(dst, DataType::I32), res);

        HANDLE_PRED_COND_END();
    }
    INST(0x4c08000000000000, 0xfff8000000000000) {
        COMMENT_NOT_IMPLEMENTED("popc");
    }
    INST(0x4c00000000000000, 0xfff8000000000000) {
        COMMENT_NOT_IMPLEMENTED("bfe");
    }
    INST(0x4bf0000000000000, 0xfff8000000000000) {
        COMMENT_NOT_IMPLEMENTED("bfi");
    }
    INST(0x4bd0000000000000, 0xfff8000000000000) {
        COMMENT_NOT_IMPLEMENTED("lea");
    }
    INST(0x4bc0000000000000, 0xfff0000000000000) {
        COMMENT_NOT_IMPLEMENTED("prmt");
    }
    INST(0x4bb0000000000000, 0xfff0000000000000) {
        const auto cmp = get_operand_5bb0_0(inst);
        const auto combine_bin = get_operand_5bb0_1(inst);
        const auto dst = GET_PRED(3);
        const auto combine = GET_PRED(0); // TODO: combine?
        const auto srcA = GET_REG(8);
        const auto srcB = GET_CMEM(34, 14);
        COMMENT("fsetp {} {} {} {} {} c{}[0x{:x}]", cmp, combine_bin, dst,
                combine, srcA, srcB.idx, srcB.imm);

        HANDLE_PRED_COND_BEGIN();

        auto cmp_res =
            BUILDER.OpCompare(cmp, ir::Value::Register(srcA, DataType::F32),
                              ir::Value::ConstMemory(srcB, DataType::F32));
        auto bin_res = BUILDER.OpBitwise(combine_bin, cmp_res,
                                         ir::Value::Predicate(combine));
        BUILDER.OpCopy(ir::Value::Predicate(dst), bin_res);

        HANDLE_PRED_COND_END();
    }
    INST(0x4ba0000000000000, 0xfff0000000000000) {
        COMMENT_NOT_IMPLEMENTED("fcmp");
    }
    INST(0x4b80000000000000, 0xfff0000000000000) {
        COMMENT_NOT_IMPLEMENTED("dsetp");
    }
    INST(0x4b70000000000000, 0xfff0000000000000) {
        COMMENT_NOT_IMPLEMENTED("dfma");
    }
    INST(0x4b60000000000000, 0xfff0000000000000) {
        const auto cmp = get_operand_5b60_0(inst);
        const auto type = get_operand_5c30_0(inst);
        const auto combine_bin = get_operand_5bb0_1(inst);
        const auto dst = GET_PRED(3);
        const auto combine = GET_PRED(0); // TODO: combine?
        const auto srcA = GET_REG(8);
        const auto srcB = GET_CMEM(34, 14);
        // TODO: pred 39
        COMMENT("isetp {} {} {} {} {} {} c{}[0x{:x}]", cmp, type, combine_bin,
                dst, combine, srcA, srcB.idx, srcB.imm);

        HANDLE_PRED_COND_BEGIN();

        auto cmp_res = BUILDER.OpCompare(cmp, ir::Value::Register(srcA, type),
                                         ir::Value::ConstMemory(srcB, type));
        auto bin_res = BUILDER.OpBitwise(combine_bin, cmp_res,
                                         ir::Value::Predicate(combine));
        BUILDER.OpCopy(ir::Value::Predicate(dst), bin_res);

        HANDLE_PRED_COND_END();
    }
    INST(0x4b50000000000000, 0xfff0000000000000) {
        COMMENT_NOT_IMPLEMENTED("iset");
    }
    INST(0x4b40000000000000, 0xfff0000000000000) {
        COMMENT_NOT_IMPLEMENTED("icmp");
    }
    INST(0x4a80000000000000, 0xff80000000000000) {
        COMMENT_NOT_IMPLEMENTED("imadsp");
    }
    INST(0x4a00000000000000, 0xff80000000000000) {
        COMMENT_NOT_IMPLEMENTED("imad");
    }
    INST(0x4980000000000000, 0xff80000000000000) {
        const bool saturate = GET_BIT(50);
        const auto dst = GET_REG(0);
        const auto srcA = GET_REG(8);
        const auto negB = GET_BIT(48);
        const auto srcB = GET_CMEM(34, 14);
        const auto negC = GET_BIT(49);
        const auto srcC = GET_REG(39);
        COMMENT("ffma {} {} {} {}c{}[0x{:x}] {}{}", (saturate ? "sat " : ""),
                dst, srcA, (negB ? "-" : ""), srcB.idx, srcB.imm,
                (negC ? "-" : ""), srcC);

        HANDLE_PRED_COND_BEGIN();

        auto res = BUILDER.OpFma(
            ir::Value::Register(srcA, DataType::F32),
            NEG_IF(ir::Value::ConstMemory(srcB, DataType::F32), negB),
            NEG_IF(ir::Value::Register(srcC, DataType::F32), negC));
        if (saturate)
            res = BUILDER.OpClamp(
                res,
                ir::Value::Immediate(std::bit_cast<u32>(0.0f), DataType::F32),
                ir::Value::Immediate(std::bit_cast<u32>(1.0f), DataType::F32));
        BUILDER.OpCopy(ir::Value::Register(dst, DataType::F32), res);

        HANDLE_PRED_COND_END();
    }
    INST(0x4900000000000000, 0xff80000000000000) {
        COMMENT_NOT_IMPLEMENTED("dset");
    }
    INST(0x4800000000000000, 0xfe00000000000000) {
        const auto bool_float = GET_BIT(52);
        const auto cmp = get_operand_5bb0_0(inst);
        const auto combine_bin = get_operand_5bb0_1(inst);
        const auto dst = GET_REG(0);
        const auto srcA = GET_REG(8);
        const auto negA = GET_BIT(43);
        const auto srcB = GET_CMEM(34, 14);
        const auto negB = GET_BIT(53);
        const auto combine = GET_PRED(39);
        COMMENT("fset {} {} {} {} {}{} {}{} {}", (bool_float ? "BF" : ""), cmp,
                combine_bin, dst, (negA ? "-" : ""), srcA, (negB ? "-" : ""),
                srcB, combine);

        HANDLE_PRED_COND_BEGIN();

        auto cmp_res = BUILDER.OpCompare(
            cmp, NEG_IF(ir::Value::Register(srcA, DataType::F32), negA),
            NEG_IF(ir::Value::ConstMemory(srcB, DataType::F32), negB));
        auto bin_res = BUILDER.OpBitwise(combine_bin, cmp_res,
                                         ir::Value::Predicate(combine));
        if (bool_float) {
            // TODO: simplify immediate value creation
            auto res = BUILDER.OpSelect(
                bin_res,
                ir::Value::Immediate(std::bit_cast<u32>(f32(1.0f)),
                                     DataType::F32),
                ir::Value::Immediate(std::bit_cast<u32>(f32(0.0f)),
                                     DataType::F32));
            BUILDER.OpCopy(ir::Value::Register(dst, DataType::F32), res);
        } else {
            auto res = BUILDER.OpCast(bin_res, DataType::U32);
            BUILDER.OpCopy(ir::Value::Register(dst), res);
        }

        HANDLE_PRED_COND_END();
    }
    INST(0x4000000000000000, 0xfe00000000000000) {
        COMMENT_NOT_IMPLEMENTED("vset");
    }
    INST(0x3c00000000000000, 0xfc00000000000000) {
        COMMENT_NOT_IMPLEMENTED("lop3");
    }
    INST(0x3a00000000000000, 0xfe00000000000000) {
        COMMENT_NOT_IMPLEMENTED("vmnmx");
    }
    INST(0x38f8000000000000, 0xfef8000000000000) {
        COMMENT_NOT_IMPLEMENTED("shf");
    }
    INST(0x38f0000000000000, 0xfef8000000000000) {
        COMMENT_NOT_IMPLEMENTED("r2p");
    }
    INST(0x38e8000000000000, 0xfef8000000000000) {
        COMMENT_NOT_IMPLEMENTED("p2r");
    }
    INST(0x38e0000000000000, 0xfef8000000000000) {
        // TODO: is dst and src type correct?
        const auto dst_type = get_operand_5ce0_0(inst);
        const auto src_type = get_operand_5ce0_1(inst);
        const auto dst = GET_REG(0);
        const auto src = GET_VALUE_U32(20, 19) |
                         (GET_VALUE_U32(56, 1) << 19); // TODO: correct?
        COMMENT("i2i {} {} {} 0x{:x}", dst_type, src_type, dst, src);

        HANDLE_PRED_COND_BEGIN();

        auto res =
            BUILDER.OpCast(ir::Value::Immediate(src, src_type), dst_type);
        BUILDER.OpCopy(ir::Value::Register(dst, dst_type), res);

        HANDLE_PRED_COND_END();
    }
    INST(0x38c0000000000000, 0xfef0000000000000) {
        COMMENT_NOT_IMPLEMENTED("iadd3");
    }
    INST(0x38b8000000000000, 0xfef8000000000000) {
        COMMENT_NOT_IMPLEMENTED("i2f");
    }
    INST(0x38b0000000000000, 0xfef8000000000000) {
        COMMENT_NOT_IMPLEMENTED("f2i");
    }
    INST(0x38a8000000000000, 0xfef8000000000000) {
        COMMENT_NOT_IMPLEMENTED("f2f");
    }
    INST(0x38a0000000000000, 0xfef8000000000000) {
        const auto dst = GET_REG(0);
        const auto srcA = GET_REG(8);
        const auto srcB = GET_VALUE_I32_SIGN_EXTEND(20, 20);
        const auto pred = GET_PRED(39);
        COMMENT("sel {} {} 0x{:x} {}", dst, srcA, srcB, pred);

        HANDLE_PRED_COND_BEGIN();

        auto res = BUILDER.OpSelect(ir::Value::Predicate(pred),
                                    ir::Value::Register(srcA, DataType::I32),
                                    ir::Value::Immediate(srcB, DataType::I32));
        BUILDER.OpCopy(ir::Value::Register(dst, DataType::I32), res);

        HANDLE_PRED_COND_END();
    }
    INST(0x3898000000000000, 0xfef8000000000000) {
        COMMENT_NOT_IMPLEMENTED("mov");
    }
    INST(0x3890000000000000, 0xfef8000000000000) {
        COMMENT_NOT_IMPLEMENTED("rro");
    }
    INST(0x3888000000000000, 0xfef8000000000000) {
        COMMENT_NOT_IMPLEMENTED("fchk");
    }
    INST(0x3880000000000000, 0xfef8000000000000) {
        COMMENT_NOT_IMPLEMENTED("dmul");
    }
    INST(0x3870000000000000, 0xfef8000000000000) {
        COMMENT_NOT_IMPLEMENTED("dadd");
    }
    INST(0x3868000000000000, 0xfef8000000000000) {
        const auto dst = GET_REG(0);
        const auto srcA = GET_REG(8);
        const auto srcB = GET_VALUE_F32();
        COMMENT("fmul {} {} 0x{:08x}", dst, srcA, srcB);

        HANDLE_PRED_COND_BEGIN();

        auto res =
            BUILDER.OpMultiply(ir::Value::Register(srcA, DataType::F32),
                               ir::Value::Immediate(srcB, DataType::F32));
        BUILDER.OpCopy(ir::Value::Register(dst, DataType::F32), res);

        HANDLE_PRED_COND_END();
    }
    INST(0x3860000000000000, 0xfef8000000000000) {
        const auto dst = GET_REG(0);
        const auto srcA = GET_REG(8);
        const auto srcB = GET_VALUE_F32();
        const auto pred = GET_PRED(39);
        const bool pred_not = GET_BIT(42);
        COMMENT("fmnmx {} {} 0x{:08x} {}{}", dst, srcA, srcB,
                (pred_not ? "!" : ""), pred);

        HANDLE_PRED_COND_BEGIN();

        auto srcA_v = ir::Value::Register(srcA, DataType::F32);
        auto srcB_v = ir::Value::Immediate(srcB, DataType::F32);
        auto min_v = BUILDER.OpMin(srcA_v, srcB_v);
        auto max_v = BUILDER.OpMax(srcA_v, srcB_v);
        auto res = BUILDER.OpSelect(
            NOT_IF(ir::Value::Predicate(pred), pred_not), min_v, max_v);
        BUILDER.OpCopy(ir::Value::Register(dst, DataType::F32), res);

        HANDLE_PRED_COND_END();
    }
    INST(0x3858000000000000, 0xfef8000000000000) {
        const auto dst = GET_REG(0);
        const bool negA = GET_BIT(48);
        const auto srcA = GET_REG(8);
        const bool negB = GET_BIT(45);
        const auto srcB = GET_VALUE_F32();
        COMMENT("fadd {} {}{} {}0x{:x}", dst, negA ? "-" : "", srcA,
                negB ? "-" : "", srcB);

        HANDLE_PRED_COND_BEGIN();

        auto res = BUILDER.OpAdd(
            NEG_IF(ir::Value::Register(srcA, DataType::F32), negA),
            NEG_IF(ir::Value::Immediate(srcB, DataType::F32), negB));
        BUILDER.OpCopy(ir::Value::Register(dst, DataType::F32), res);

        HANDLE_PRED_COND_END();
    }
    INST(0x3850000000000000, 0xfef8000000000000) {
        COMMENT_NOT_IMPLEMENTED("dmnmx");
    }
    INST(0x3848000000000000, 0xfef8000000000000) {
        const auto dst = GET_REG(0);
        const auto src = GET_REG(8);
        const auto shift = GET_VALUE_U32(20, 19) |
                           (GET_VALUE_U32(56, 1) << 19); // TODO: correct?
        COMMENT("shl {} {} 0x{:x}", dst, src, shift);

        HANDLE_PRED_COND_BEGIN();

        auto res = BUILDER.OpShiftLeft(ir::Value::Register(src), shift);
        BUILDER.OpCopy(ir::Value::Register(dst), res);

        HANDLE_PRED_COND_END();
    }
    INST(0x3840000000000000, 0xfef8000000000000) {
        const auto bin = get_operand_5c40_0(inst);
        const auto pred_op = get_operand_5c40_1(inst);
        const auto dst_pred = GET_PRED(48);
        const auto dst = GET_REG(0);
        const auto invA = GET_BIT(39);
        const auto srcA = GET_REG(8);
        const auto invB = GET_BIT(40);
        const auto srcB = GET_VALUE_U32(20, 20);
        COMMENT("lop {} {} {} {}{} {}0x{:x}", bin, pred_op, dst,
                (invA ? "!" : ""), srcA, (invB ? "!" : ""), srcB);

        HANDLE_PRED_COND_BEGIN();

        // TODO: inv
        auto srcB_v = ir::Value::Immediate(srcB);
        auto res =
            (bin == BitwiseOp::PassB
                 ? srcB_v
                 : BUILDER.OpBitwise(bin, ir::Value::Register(srcA), srcB_v));
        BUILDER.OpCopy(ir::Value::Register(dst), res);

        std::optional<ir::Value> pred_v;
        switch (pred_op) {
        case PredOp::False:
            pred_v = ir::Value::Immediate(0); // TODO: false
            break;
        case PredOp::True:
            pred_v = ir::Value::Immediate(1); // TODO: true
            break;
        case PredOp::Zero:
            pred_v = BUILDER.OpCompare(ComparisonOp::Equal, res,
                                       ir::Value::Immediate(0));
            break;
        case PredOp::NotZero:
            pred_v = BUILDER.OpCompare(ComparisonOp::NotEqual, res,
                                       ir::Value::Immediate(0));
            break;
        default:
            pred_v = std::nullopt;
            break;
        }
        BUILDER.OpCopy(ir::Value::Predicate(dst_pred), pred_v.value());

        HANDLE_PRED_COND_END();
    }
    INST(0x3838000000000000, 0xfef8000000000000) {
        COMMENT_NOT_IMPLEMENTED("imul");
    }
    INST(0x3830000000000000, 0xfef8000000000000) {
        COMMENT_NOT_IMPLEMENTED("flo");
    }
    INST(0x3828000000000000, 0xfef8000000000000) {
        const auto type = get_operand_5c30_0(inst);
        const auto dst = GET_REG(0);
        const auto src = GET_REG(8);
        const auto shift = GET_VALUE_U32(20, 19) |
                           (GET_VALUE_U32(56, 1) << 19); // TODO: correct?
        COMMENT("shr {} {} {} 0x{:x}", type, dst, src, shift);

        HANDLE_PRED_COND_BEGIN();

        auto res = BUILDER.OpShiftRight(ir::Value::Register(src, type), shift);
        BUILDER.OpCopy(ir::Value::Register(dst, type), res);

        HANDLE_PRED_COND_END();
    }
    INST(0x3820000000000000, 0xfef8000000000000) {
        COMMENT_NOT_IMPLEMENTED("imnmx");
    }
    INST(0x3818000000000000, 0xfef8000000000000) {
        const auto dst = GET_REG(0);
        const bool negA = GET_BIT(49);
        const auto srcA = GET_REG(8);
        const bool negB = GET_BIT(48);
        const auto srcB = GET_VALUE_I32_SIGN_EXTEND(20, 20);
        const auto shift = GET_VALUE_U32(39, 5);
        COMMENT("iscadd {} {}{} {}0x{:x} 0x{:x}", dst, (negA ? "-" : ""), srcA,
                (negB ? "-" : ""), srcB, shift);

        HANDLE_PRED_COND_BEGIN();

        auto srcA_v = ir::Value::Register(srcA, DataType::I32);
        srcA_v = BUILDER.OpShiftLeft(srcA_v, shift);
        // TODO: negA

        auto res = BUILDER.OpAdd(
            srcA_v, NEG_IF(ir::Value::Immediate(srcB, DataType::I32), negB));
        BUILDER.OpCopy(ir::Value::Register(dst, DataType::I32), res);

        HANDLE_PRED_COND_END();
    }
    INST(0x3810000000000000, 0xfef8000000000000) {
        const auto dst = GET_REG(0);
        const bool negA = GET_BIT(49);
        const auto srcA = GET_REG(8);
        const bool negB = GET_BIT(48);
        const auto srcB = GET_VALUE_U32(20, 20);
        COMMENT("iadd {} {}{} {}0x{:x}", dst, (negA ? "-" : ""), srcA,
                (negB ? "-" : ""), srcB);

        HANDLE_PRED_COND_BEGIN();

        auto res = BUILDER.OpAdd(
            NEG_IF(ir::Value::Register(srcA, DataType::I32), negA),
            NEG_IF(ir::Value::Immediate(srcB, DataType::I32), negB));
        BUILDER.OpCopy(ir::Value::Register(dst, DataType::I32), res);

        HANDLE_PRED_COND_END();
    }
    INST(0x3808000000000000, 0xfef8000000000000) {
        COMMENT_NOT_IMPLEMENTED("popc");
    }
    INST(0x3800000000000000, 0xfef8000000000000) {
        const auto type = get_operand_5c30_0(inst);
        const auto dst = GET_REG(0);
        const auto src = GET_REG(8);
        const auto bf = GET_VALUE_U32(20, 19) |
                        (GET_VALUE_U32(56, 1) << 19); // TODO: correct?
        COMMENT("bfe {} {} {} 0x{:x}", type, dst, src, bf);

        HANDLE_PRED_COND_BEGIN();

        auto position = extract_bits<u32, 0, 8>(bf);
        auto size = extract_bits<u32, 8, 8>(bf);

        auto res = BUILDER.OpBitwise(
            BitwiseOp::And,
            BUILDER.OpShiftRight(ir::Value::Register(src, type), position),
            ir::Value::Immediate((1 << size) - 1,
                                 type)); // TODO: BitfieldExtract
        BUILDER.OpCopy(ir::Value::Register(dst, type), res);

        HANDLE_PRED_COND_END();
    }
    INST(0x36f8000000000000, 0xfef8000000000000) {
        COMMENT_NOT_IMPLEMENTED("shf");
    }
    INST(0x36f0000000000000, 0xfef8000000000000) {
        COMMENT_NOT_IMPLEMENTED("bfi");
    }
    INST(0x36d0000000000000, 0xfef8000000000000) {
        COMMENT_NOT_IMPLEMENTED("lea");
    }
    INST(0x36c0000000000000, 0xfef0000000000000) {
        COMMENT_NOT_IMPLEMENTED("prmt");
    }
    INST(0x36b0000000000000, 0xfef0000000000000) {
        const auto cmp = get_operand_5bb0_0(inst);
        const auto combine_bin = get_operand_5bb0_1(inst);
        const auto dst = GET_PRED(3);
        const auto combine = GET_PRED(0); // TODO: combine?
        const auto srcA = GET_REG(8);
        const auto srcB = GET_VALUE_F32();
        COMMENT("fsetp {} {} {} {} {} 0x{:08x}", cmp, combine_bin, dst, combine,
                srcA, srcB);

        HANDLE_PRED_COND_BEGIN();

        auto cmp_res =
            BUILDER.OpCompare(cmp, ir::Value::Register(srcA, DataType::F32),
                              ir::Value::Immediate(srcB, DataType::F32));
        auto bin_res = BUILDER.OpBitwise(combine_bin, cmp_res,
                                         ir::Value::Predicate(combine));
        BUILDER.OpCopy(ir::Value::Predicate(dst), bin_res);

        HANDLE_PRED_COND_END();
    }
    INST(0x36a0000000000000, 0xfef0000000000000) {
        COMMENT_NOT_IMPLEMENTED("fcmp");
    }
    INST(0x3680000000000000, 0xfef0000000000000) {
        COMMENT_NOT_IMPLEMENTED("dsetp");
    }
    INST(0x3670000000000000, 0xfef0000000000000) {
        COMMENT_NOT_IMPLEMENTED("dfma");
    }
    INST(0x3660000000000000, 0xfef0000000000000) {
        const auto cmp = get_operand_5b60_0(inst);
        const auto type = get_operand_5c30_0(inst);
        const auto combine_bin = get_operand_5bb0_1(inst);
        const auto dst = GET_PRED(3);
        const auto combine = GET_PRED(0); // TODO: combine?
        const auto srcA = GET_REG(8);
        const auto srcB = GET_VALUE_U32(20, 19) |
                          (GET_VALUE_U32(56, 1) << 19); // TODO: correct?
        // TODO: pred 39
        COMMENT("isetp {} {} {} {} {} {} {}", cmp, type, combine_bin, dst,
                combine, srcA, srcB);

        HANDLE_PRED_COND_BEGIN();

        auto cmp_res = BUILDER.OpCompare(cmp, ir::Value::Register(srcA, type),
                                         ir::Value::Immediate(srcB, type));
        auto bin_res = BUILDER.OpBitwise(combine_bin, cmp_res,
                                         ir::Value::Predicate(combine));
        BUILDER.OpCopy(ir::Value::Predicate(dst), bin_res);

        HANDLE_PRED_COND_END();
    }
    INST(0x3650000000000000, 0xfef0000000000000) {
        COMMENT_NOT_IMPLEMENTED("iset");
    }
    INST(0x3640000000000000, 0xfef0000000000000) {
        COMMENT_NOT_IMPLEMENTED("icmp");
    }
    INST(0x3600000000000000, 0xfec0000000000000) {
        COMMENT_NOT_IMPLEMENTED("xmad");
    }
    INST(0x3480000000000000, 0xfe80000000000000) {
        COMMENT_NOT_IMPLEMENTED("imadsp");
    }
    INST(0x3400000000000000, 0xfe80000000000000) {
        COMMENT_NOT_IMPLEMENTED("imad");
    }
    INST(0x3280000000000000, 0xfe80000000000000) {
        const bool saturate = GET_BIT(50);
        const auto dst = GET_REG(0);
        const auto srcA = GET_REG(8);
        const auto negB = GET_BIT(48);
        const auto srcB = GET_VALUE_F32();
        const auto negC = GET_BIT(49);
        const auto srcC = GET_REG(39);
        COMMENT("ffma {} {} {} {}0x{:08x} {}{}", (saturate ? "sat " : ""), dst,
                srcA, (negB ? "-" : ""), srcB, (negC ? "-" : ""), srcC);

        HANDLE_PRED_COND_BEGIN();

        auto res = BUILDER.OpFma(
            ir::Value::Register(srcA, DataType::F32),
            NEG_IF(ir::Value::Immediate(srcB, DataType::F32), negB),
            NEG_IF(ir::Value::Register(srcC, DataType::F32), negC));
        if (saturate)
            res = BUILDER.OpClamp(
                res,
                ir::Value::Immediate(std::bit_cast<u32>(0.0f), DataType::F32),
                ir::Value::Immediate(std::bit_cast<u32>(1.0f), DataType::F32));
        BUILDER.OpCopy(ir::Value::Register(dst, DataType::F32), res);

        HANDLE_PRED_COND_END();
    }
    INST(0x3200000000000000, 0xfe80000000000000) {
        COMMENT_NOT_IMPLEMENTED("dset");
    }
    INST(0x3000000000000000, 0xfe00000000000000) {
        const auto bool_float = GET_BIT(52);
        const auto cmp = get_operand_5bb0_0(inst);
        const auto combine_bin = get_operand_5bb0_1(inst);
        const auto dst = GET_REG(0);
        const auto srcA = GET_REG(8);
        const auto negA = GET_BIT(43);
        const auto srcB = GET_VALUE_F32();
        const auto negB = GET_BIT(53);
        // TODO: combine
        COMMENT("fset {} {} {} {} {}{} {}0x{:08x}", (bool_float ? "BF" : ""),
                cmp, combine_bin, dst, (negA ? "-" : ""), srcA,
                (negB ? "-" : ""), srcB);

        HANDLE_PRED_COND_BEGIN();

        auto cmp_res = BUILDER.OpCompare(
            cmp, NEG_IF(ir::Value::Register(srcA, DataType::F32), negA),
            NEG_IF(ir::Value::Immediate(srcB, DataType::F32), negB));
        // TODO: uncomment
        // auto bin_res = BUILDER.OpBitwise(combine_bin, cmp_res,
        //                    ir::Value::Predicate(combine));
        // TODO: use bin_res instead of cmp_res
        // TODO: simplify immediate value creation
        if (bool_float) {
            auto res = BUILDER.OpSelect(
                cmp_res,
                ir::Value::Immediate(std::bit_cast<u32>(f32(1.0f)),
                                     DataType::F32),
                ir::Value::Immediate(std::bit_cast<u32>(f32(0.0f)),
                                     DataType::F32));
            BUILDER.OpCopy(ir::Value::Register(dst, DataType::F32), res);
        } else {
            auto res = BUILDER.OpCast(cmp_res, DataType::U32);
            BUILDER.OpCopy(ir::Value::Register(dst), res);
        }

        HANDLE_PRED_COND_END();
    }
    INST(0x2c00000000000000, 0xfe00000000000000) {
        COMMENT_NOT_IMPLEMENTED("hadd2_32i");
    }
    INST(0x2a00000000000000, 0xfe00000000000000) {
        COMMENT_NOT_IMPLEMENTED("hmul2_32i");
    }
    INST(0x2800000000000000, 0xfe00000000000000) {
        COMMENT_NOT_IMPLEMENTED("hfma2_32i");
    }
    INST(0x2000000000000000, 0xfc00000000000000) {
        COMMENT_NOT_IMPLEMENTED("vadd");
    }
    INST(0x1f00000000000000, 0xff00000000000000) {
        COMMENT_NOT_IMPLEMENTED("imul32i");
    }
    INST(0x1e00000000000000, 0xff00000000000000) {
        const auto dst = GET_REG(0);
        const auto srcA = GET_REG(8);
        const auto srcB = GET_VALUE_U32(20, 32);
        COMMENT("fmul32i {} {} 0x{:08x}", dst, srcA, srcB);

        HANDLE_PRED_COND_BEGIN();

        auto srcB_v = ir::Value::Immediate(srcB, DataType::F32);
        auto res = BUILDER.OpMultiply(ir::Value::Register(srcA, DataType::F32),
                                      srcB_v);
        BUILDER.OpCopy(ir::Value::Register(dst, DataType::F32), res);

        HANDLE_PRED_COND_END();
    }
    INST(0x1d80000000000000, 0xff80000000000000) {
        COMMENT_NOT_IMPLEMENTED("iadd32i");
    }
    INST(0x1c00000000000000, 0xfe80000000000000) {
        const auto dst = GET_REG(0);
        const bool negA = GET_BIT(56);
        const auto srcA = GET_REG(8);
        const auto srcB = GET_VALUE_U32(20, 32);
        COMMENT("iadd32i {} {}{} 0x{:08x}", dst, (negA ? "-" : ""), srcA, srcB);

        HANDLE_PRED_COND_BEGIN();

        auto res = BUILDER.OpAdd(
            NEG_IF(ir::Value::Register(srcA, DataType::I32), negA),
            ir::Value::Immediate(srcB, DataType::I32));
        BUILDER.OpCopy(ir::Value::Register(dst, DataType::I32), res);

        HANDLE_PRED_COND_END();
    }
    INST(0x1800000000000000, 0xfc00000000000000) {
        COMMENT_NOT_IMPLEMENTED("lea");
    }
    INST(0x1400000000000000, 0xfc00000000000000) {
        COMMENT_NOT_IMPLEMENTED("iscadd32i");
    }
    INST(0x1000000000000000, 0xfc00000000000000) {
        COMMENT_NOT_IMPLEMENTED("imad32i");
    }
    INST(0x0c00000000000000, 0xfc00000000000000) {
        COMMENT_NOT_IMPLEMENTED("ffma32i");
    }
    INST(0x0800000000000000, 0xfc00000000000000) {
        const auto dst = GET_REG(0);
        const auto srcA = GET_REG(8);
        const auto srcB = GET_VALUE_U32(20, 32);
        COMMENT("fadd32i {} {} 0x{:08x}", dst, srcA, srcB);

        HANDLE_PRED_COND_BEGIN();

        auto res = BUILDER.OpAdd(ir::Value::Register(srcA, DataType::F32),
                                 ir::Value::Immediate(srcB, DataType::F32));
        BUILDER.OpCopy(ir::Value::Register(dst, DataType::F32), res);

        HANDLE_PRED_COND_END();
    }
    INST(0x0400000000000000, 0xfc00000000000000) {
        const auto bin = get_operand_0400_0(inst);
        const auto dst = GET_REG(0);
        const auto invA = GET_BIT(55);
        const auto srcA = GET_REG(8);
        const auto invB = GET_BIT(56);
        const auto srcB = GET_VALUE_U32(20, 32);
        COMMENT("lop32i {} {} {}{} {}0x{:08x}", bin, dst, (invA ? "!" : ""),
                srcA, (invB ? "!" : ""), srcB);

        HANDLE_PRED_COND_BEGIN();

        // TODO: inv
        auto srcB_v = ir::Value::Immediate(srcB, DataType::U32);
        auto res =
            (bin == BitwiseOp::PassB
                 ? srcB_v
                 : BUILDER.OpBitwise(
                       bin, ir::Value::Register(srcA, DataType::U32), srcB_v));
        BUILDER.OpCopy(ir::Value::Register(dst, DataType::F32), res);

        HANDLE_PRED_COND_END();
    }
    INST(0x0200000000000000, 0xfe00000000000000) {
        COMMENT_NOT_IMPLEMENTED("lop3");
    }
    INST(0x0100000000000000, 0xfff0000000000000) {
        const auto dst = GET_REG(0);
        const auto value = GET_VALUE_U32(20, 32);
        const auto todo =
            extract_bits<u32, 4, 12>(inst) >> 8; // TODO: what is this?
        COMMENT("mov32i {} 0x{:08x} 0x{:08x}", dst, value, todo);

        HANDLE_PRED_COND_BEGIN();

        BUILDER.OpCopy(ir::Value::Register(dst), ir::Value::Immediate(value));

        HANDLE_PRED_COND_END();
    }
    else {
        LOG_ERROR(ShaderDecompiler, "Unknown instruction 0x{:016x}", inst);
    }
} // namespace hydra::hw::tegra_x1::gpu::renderer::shader_decomp::decoder

} // namespace hydra::hw::tegra_x1::gpu::renderer::shader_decomp::decoder
