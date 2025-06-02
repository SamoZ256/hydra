#include "core/hw/tegra_x1/gpu/renderer/shader_decompiler/iterator_base.hpp"

#include "core/hw/tegra_x1/gpu/renderer/shader_decompiler/tables.hpp"

namespace hydra::hw::tegra_x1::gpu::renderer::shader_decomp {

result_t IteratorBase::ParseNextInstruction(ObserverBase* o) {
    const auto inst = code_reader.Read<instruction_t>();
    const u32 pc = GetPC();
    if ((pc % 4) == 0) // Sched
        return {ResultCode::None};

    o->SetPC(pc);
    LOG_DEBUG(ShaderDecompiler, "Instruction 0x{:016x}", inst);

    const auto res = ParseNextInstructionImpl(o, pc, inst);
    o->ClearPredCond();

    return res;
}

result_t IteratorBase::ParseNextInstructionImpl(ObserverBase* o, const u32 pc,
                                                const instruction_t inst) {
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
#define GET_VALUE_I_SIGN_EXTEND(type_bit_count, b, count)                      \
    sign_extend<i##type_bit_count, count>(GET_VALUE_U(type_bit_count, b, count))
#define GET_VALUE_I32_SIGN_EXTEND(b, count)                                    \
    GET_VALUE_I_SIGN_EXTEND(32, b, count)
#define GET_VALUE_I64_SIGN_EXTEND(b, count)                                    \
    GET_VALUE_I_SIGN_EXTEND(64, b, count)
    // TODO: correct?
#define GET_VALUE_F32() ((GET_BIT(56) << 31) | (GET_VALUE_U32(20, 19) << 12))

#define GET_BIT(b) extract_bits<u32, b, 1>(inst)

#define GET_BTARG()                                                            \
    static_cast<u32>(pc +                                                      \
                     std::bit_cast<i32>(GET_VALUE_I32_SIGN_EXTEND(20, 24)) /   \
                         sizeof(instruction_t) +                               \
                     1)

#define GET_AMEM()                                                             \
    AMem { GET_REG(8), 0 }
#define GET_AMEM_IDX(b)                                                        \
    AMem { GET_REG(8), extract_bits<u32, b, 10>(inst) }

// HACK: why do I have to multiply by 4?
#define GET_CMEM(b_idx, count_imm)                                             \
    CMem{GET_VALUE_U32(b_idx, 5), RZ,                                          \
         extract_bits<u32, 20, count_imm>(inst) * 4}
#define GET_CMEM_R(b_idx, b_reg, count_imm)                                    \
    CMem {                                                                     \
        GET_VALUE_U32(b_idx, 5), GET_REG(b_reg),                               \
            extract_bits<u32, 20, count_imm>(inst) * 4                         \
    }

#define HANDLE_PRED_COND()                                                     \
    bool conditional = false;                                                  \
    if ((inst & 0x00000000000f0000) == 0x0000000000070000) { /* nothing */     \
    } else if ((inst & 0x00000000000f0000) ==                                  \
               0x00000000000f0000) { /* never */                               \
        LOG_DEBUG(ShaderDecompiler, "never");                                  \
        throw; /* TODO: implement */                                           \
    } else {   /* conditional */                                               \
        const auto pred = GET_PRED(16);                                        \
        const bool not_ = GET_BIT(19);                                         \
        LOG_DEBUG(ShaderDecompiler, "if {}p{}", not_ ? "!" : "", pred);        \
        o->SetPredCond({pred, not_});                                          \
        conditional = true;                                                    \
    }

    INST0(0xfbe0000000000000, 0xfff8000000000000)
    LOG_NOT_IMPLEMENTED(ShaderDecompiler, "out");
    INST(0xf6e0000000000000, 0xfef8000000000000)
    LOG_NOT_IMPLEMENTED(ShaderDecompiler, "out");
    INST(0xf0f8000000000000, 0xfff8000000000000) {
        HANDLE_PRED_COND();

        // TODO: f0f8_0
        LOG_DEBUG(ShaderDecompiler, "sync");

        o->OpSync();

        return {ResultCode::EndBlock};
    }
    INST(0xf0f0000000000000, 0xfff8000000000000)
    LOG_NOT_IMPLEMENTED(ShaderDecompiler, "depbar");
    INST(0xf0c8000000000000, 0xfff8000000000000)
    LOG_NOT_IMPLEMENTED(ShaderDecompiler, "mov");
    INST(0xf0c0000000000000, 0xfff8000000000000)
    LOG_NOT_IMPLEMENTED(ShaderDecompiler, "r2b");
    INST(0xf0b8000000000000, 0xfff8000000000000)
    LOG_NOT_IMPLEMENTED(ShaderDecompiler, "b2r");
    INST(0xf0a8000000000000, 0xfff8000000000000)
    LOG_NOT_IMPLEMENTED(ShaderDecompiler, "bar");
    INST(0xeff0000000000000, 0xfff8000000000000) {
        HANDLE_PRED_COND();

        const auto mode = get_operand_eff0_0(inst);
        auto amem = GET_AMEM_IDX(20);
        const auto src = GET_REG(0);
        const auto todo = GET_REG(39); // TODO: what is this?
        LOG_DEBUG(ShaderDecompiler, "st {} a[r{} + 0x{:08x}] r{} r{}", mode,
                  amem.reg, amem.imm, src, todo);

        for (u32 i = 0; i < get_load_store_count(mode); i++) {
            o->OpMove(o->OpAttributeMemory(false, amem),
                      o->OpRegister(true, src + i));
            amem.imm += sizeof(u32);
        }
    }
    INST(0xefe8000000000000, 0xfff8000000000000)
    LOG_NOT_IMPLEMENTED(ShaderDecompiler, "pixld");
    INST(0xefd8000000000000, 0xfff8000000000000) {
        HANDLE_PRED_COND();

        const auto mode = get_operand_eff0_0(inst);
        const auto dst = GET_REG(0);
        auto amem = GET_AMEM_IDX(20);
        const auto todo = GET_REG(39); // TODO: what is this?
        LOG_DEBUG(ShaderDecompiler, "ld {} r{} a[r{} + 0x{:08x}] r{}", mode,
                  dst, amem.reg, amem.imm, todo);

        for (u32 i = 0; i < get_load_store_count(mode); i++) {
            o->OpMove(o->OpRegister(false, dst + i),
                      o->OpAttributeMemory(true, amem));
            amem.imm += sizeof(u32);
        }
    }
    INST(0xefd0000000000000, 0xfff8000000000000)
    LOG_NOT_IMPLEMENTED(ShaderDecompiler, "isberd");
    INST(0xefa0000000000000, 0xfff8000000000000)
    LOG_NOT_IMPLEMENTED(ShaderDecompiler, "al2p");
    INST(0xef98000000000000, 0xfff8000000000000)
    LOG_NOT_IMPLEMENTED(ShaderDecompiler, "membar");
    INST(0xef90000000000000, 0xfff8000000000000) {
        HANDLE_PRED_COND();

        const auto dst = GET_REG(0);
        const auto src = GET_CMEM_R(36, 8, 16);
        LOG_DEBUG(ShaderDecompiler, "ld r{} c{}[r{} + 0x{:x}]", dst, src.idx,
                  src.reg, src.imm);

        o->OpMove(o->OpRegister(false, dst), o->OpConstMemoryL(src));
    }
    INST(0xef80000000000000, 0xffe0000000000000)
    LOG_NOT_IMPLEMENTED(ShaderDecompiler, "cctll");
    INST(0xef60000000000000, 0xffe0000000000000)
    LOG_NOT_IMPLEMENTED(ShaderDecompiler, "cctl");
    INST(0xef58000000000000, 0xfff8000000000000)
    LOG_NOT_IMPLEMENTED(ShaderDecompiler, "st");
    INST(0xef50000000000000, 0xfff8000000000000)
    LOG_NOT_IMPLEMENTED(ShaderDecompiler, "st");
    INST(0xef48000000000000, 0xfff8000000000000)
    LOG_NOT_IMPLEMENTED(ShaderDecompiler, "ld");
    INST(0xef40000000000000, 0xfff8000000000000)
    LOG_NOT_IMPLEMENTED(ShaderDecompiler, "ld");
    INST(0xef10000000000000, 0xfff8000000000000)
    LOG_NOT_IMPLEMENTED(ShaderDecompiler, "shfl");
    INST(0xeef0000000000000, 0xfff0000000000000)
    LOG_NOT_IMPLEMENTED(ShaderDecompiler, "atom");
    INST(0xeed8000000000000, 0xfff8000000000000)
    LOG_NOT_IMPLEMENTED(ShaderDecompiler, "stg");
    INST(0xeed0000000000000, 0xfff8000000000000)
    LOG_NOT_IMPLEMENTED(ShaderDecompiler, "ldg");
    INST(0xeec8000000000000, 0xfff8000000000000)
    LOG_NOT_IMPLEMENTED(ShaderDecompiler, "ldg");
    INST(0xeea0000000000000, 0xfff8000000000000)
    LOG_NOT_IMPLEMENTED(ShaderDecompiler, "stp");
    INST(0xee00000000000000, 0xff80000000000000)
    LOG_NOT_IMPLEMENTED(ShaderDecompiler, "atoms");
    INST(0xed00000000000000, 0xff00000000000000)
    LOG_NOT_IMPLEMENTED(ShaderDecompiler, "atom");
    INST(0xec00000000000000, 0xff00000000000000)
    LOG_NOT_IMPLEMENTED(ShaderDecompiler, "atoms");
    INST(0xebf8000000000000, 0xfff8000000000000)
    LOG_NOT_IMPLEMENTED(ShaderDecompiler, "red");
    INST(0xebf0000000000000, 0xfff9000000000000)
    LOG_NOT_IMPLEMENTED(ShaderDecompiler, "cctlt");
    INST(0xebe0000000000000, 0xfff8000000000000)
    LOG_NOT_IMPLEMENTED(ShaderDecompiler, "out");
    INST(0xeb40000000000000, 0xffe0000000000000)
    LOG_NOT_IMPLEMENTED(ShaderDecompiler, "sured");
    INST(0xeb20000000000000, 0xffe0000000000000)
    LOG_NOT_IMPLEMENTED(ShaderDecompiler, "sust");
    INST(0xeb00000000000000, 0xffe0000000000000)
    LOG_NOT_IMPLEMENTED(ShaderDecompiler, "suld");
    INST(0xeac0000000000000, 0xffe0000000000000)
    LOG_NOT_IMPLEMENTED(ShaderDecompiler, "suatom");
    INST(0xea80000000000000, 0xffc0000000000000)
    LOG_NOT_IMPLEMENTED(ShaderDecompiler, "suatom");
    INST(0xea60000000000000, 0xffe0000000000000)
    LOG_NOT_IMPLEMENTED(ShaderDecompiler, "suatom");
    INST(0xea00000000000000, 0xffc0000000000000)
    LOG_NOT_IMPLEMENTED(ShaderDecompiler, "suatom");
    INST(0xe3a0000000000000, 0xfff0000000000000)
    LOG_NOT_IMPLEMENTED(ShaderDecompiler, "bpt");
    INST(0xe390000000000000, 0xfff0000000000000)
    LOG_NOT_IMPLEMENTED(ShaderDecompiler, "ide");
    INST(0xe380000000000000, 0xfff0000000000000)
    LOG_NOT_IMPLEMENTED(ShaderDecompiler, "ram");
    INST(0xe370000000000000, 0xfff0000000000000)
    LOG_NOT_IMPLEMENTED(ShaderDecompiler, "sam");
    INST(0xe360000000000000, 0xfff0000000000000)
    LOG_NOT_IMPLEMENTED(ShaderDecompiler, "rtt");
    INST(0xe350000000000000, 0xfff0000000000000)
    LOG_NOT_IMPLEMENTED(ShaderDecompiler, "cont");
    INST(0xe340000000000000, 0xfff0000000000000)
    LOG_NOT_IMPLEMENTED(ShaderDecompiler, "brk");
    INST(0xe330000000000000, 0xfff0000000000000) {
        HANDLE_PRED_COND();

        // TODO: f0f8_0
        LOG_DEBUG(ShaderDecompiler, "kil");

        o->OpDiscard();
    }
    INST(0xe320000000000000, 0xfff0000000000000)
    LOG_NOT_IMPLEMENTED(ShaderDecompiler, "ret");
    INST(0xe310000000000000, 0xfff0000000000000)
    LOG_NOT_IMPLEMENTED(ShaderDecompiler, "longjmp");
    INST(0xe300000000000000, 0xfff0000000000000) {
        HANDLE_PRED_COND();

        // TODO: f0f8_0
        LOG_DEBUG(ShaderDecompiler, "exit");

        o->OpExit();

        return {ResultCode::EndBlock};
    }
    INST(0xe2f0000000000000, 0xfff0000000000000)
    LOG_NOT_IMPLEMENTED(ShaderDecompiler, "setlmembase");
    INST(0xe2e0000000000000, 0xfff0000000000000)
    LOG_NOT_IMPLEMENTED(ShaderDecompiler, "setcrsptr");
    INST(0xe2d0000000000000, 0xfff0000000000000)
    LOG_NOT_IMPLEMENTED(ShaderDecompiler, "getlmembase");
    INST(0xe2c0000000000000, 0xfff0000000000000)
    LOG_NOT_IMPLEMENTED(ShaderDecompiler, "getcrsptr");
    INST(0xe2b0000000000020, 0xfff0000000000020)
    LOG_NOT_IMPLEMENTED(ShaderDecompiler, "pcnt");
    INST(0xe2b0000000000000, 0xfff0000000000020)
    LOG_NOT_IMPLEMENTED(ShaderDecompiler, "pcnt");
    INST(0xe2a0000000000020, 0xfff0000000000020)
    LOG_NOT_IMPLEMENTED(ShaderDecompiler, "pbk");
    INST(0xe2a0000000000000, 0xfff0000000000020)
    LOG_NOT_IMPLEMENTED(ShaderDecompiler, "pbk");
    INST(0xe290000000000020, 0xfff0000000000020)
    LOG_NOT_IMPLEMENTED(ShaderDecompiler, "ssy");
    INST(0xe290000000000000, 0xfff0000000000020) {
        const auto target = GET_BTARG();
        LOG_DEBUG(ShaderDecompiler, "ssy 0x{:x}", target);

        o->OpSetSync(target);

        return {ResultCode::SyncPoint, target};
    }
    INST(0xe280000000000020, 0xfff0000000000020)
    LOG_NOT_IMPLEMENTED(ShaderDecompiler, "plongjmp");
    INST(0xe280000000000000, 0xfff0000000000020)
    LOG_NOT_IMPLEMENTED(ShaderDecompiler, "plongjmp");
    INST(0xe270000000000020, 0xfff0000000000020)
    LOG_NOT_IMPLEMENTED(ShaderDecompiler, "pret");
    INST(0xe270000000000000, 0xfff0000000000020)
    LOG_NOT_IMPLEMENTED(ShaderDecompiler, "pret");
    INST(0xe260000000000020, 0xfff0000000000020)
    LOG_NOT_IMPLEMENTED(ShaderDecompiler, "cal");
    INST(0xe260000000000000, 0xfff0000000000020)
    LOG_NOT_IMPLEMENTED(ShaderDecompiler, "cal");
    INST(0xe250000000000020, 0xfff0000000000020)
    LOG_NOT_IMPLEMENTED(ShaderDecompiler, "brx");
    INST(0xe250000000000000, 0xfff0000000000000)
    LOG_NOT_IMPLEMENTED(ShaderDecompiler, "brx");
    INST(0xe240000000000020, 0xfff0000000000020)
    LOG_NOT_IMPLEMENTED(ShaderDecompiler, "bra");
    INST(0xe240000000000000, 0xfff0000000000020) {
        HANDLE_PRED_COND();

        // TODO: f0f8_0
        const auto target = GET_BTARG();
        LOG_DEBUG(ShaderDecompiler, "bra 0x{:x}", target);

        o->OpBranch(target);

        return {conditional ? ResultCode::BranchConditional
                            : ResultCode::Branch,
                target};
    }
    INST(0xe230000000000000, 0xfff0000000000000)
    LOG_NOT_IMPLEMENTED(ShaderDecompiler, "pexit");
    INST(0xe220000000000020, 0xfff0000000000020)
    LOG_NOT_IMPLEMENTED(ShaderDecompiler, "jcal");
    INST(0xe220000000000000, 0xfff0000000000020)
    LOG_NOT_IMPLEMENTED(ShaderDecompiler, "jcal");
    INST(0xe210000000000020, 0xfff0000000000020)
    LOG_NOT_IMPLEMENTED(ShaderDecompiler, "jmp");
    INST(0xe210000000000000, 0xfff0000000000000)
    LOG_NOT_IMPLEMENTED(ShaderDecompiler, "jmp");
    INST(0xe200000000000020, 0xfff0000000000020)
    LOG_NOT_IMPLEMENTED(ShaderDecompiler, "jmx");
    INST(0xe200000000000000, 0xfff0000000000020)
    LOG_NOT_IMPLEMENTED(ShaderDecompiler, "jmx");
    INST(0xe00000000000ff00, 0xff0000400000ff00) {
        HANDLE_PRED_COND();

        // TODO: mode
        const auto dst = GET_REG(0);
        const auto amem = GET_AMEM_IDX(28);
        const auto interp_param = GET_REG(20);
        const auto flag1 = GET_REG(39);
        // TODO: another flag
        LOG_DEBUG(ShaderDecompiler, "ipa r{} a[r{} + 0x{:08x}] r{} r{}", dst,
                  amem.reg, amem.imm, interp_param, flag1);

        auto res = o->OpInterpolate(o->OpAttributeMemory(true, amem));
        o->OpMove(o->OpRegister(false, dst), res);
    }
    INST(0xe000004000000000, 0xff00004000000000)
    LOG_NOT_IMPLEMENTED(ShaderDecompiler, "ipa");
    INST(0xdf60000000000000, 0xfff8000000000000)
    LOG_NOT_IMPLEMENTED(ShaderDecompiler, "tmml");
    INST(0xdf58000000000000, 0xfff8000000000000)
    LOG_NOT_IMPLEMENTED(ShaderDecompiler, "tmml");
    INST(0xdf50000000000000, 0xfff8000000000000)
    LOG_NOT_IMPLEMENTED(ShaderDecompiler, "txq");
    INST(0xdf48000000000000, 0xfff8000000000000)
    LOG_NOT_IMPLEMENTED(ShaderDecompiler, "txq");
    INST(0xdf40000000000000, 0xfff8000000000000)
    LOG_NOT_IMPLEMENTED(ShaderDecompiler, "txa");
    INST(0xdf00000000000000, 0xff40000000000000)
    LOG_NOT_IMPLEMENTED(ShaderDecompiler, "tld4s");
    INST(0xdef8000000000000, 0xfff8000000000000)
    LOG_NOT_IMPLEMENTED(ShaderDecompiler, "tld4");
    INST(0xdeb8000000000000, 0xfff8000000000000) {
        HANDLE_PRED_COND();

        // TODO: more stuff
        // TOOD: correct?
        const auto dst = GET_REG(0);
        const auto coords_x = GET_REG(8);
        const auto coords_y = GET_REG(20);
        const auto todo =
            GET_VALUE_U32(31, 4); // TODO: what is this? (some sort of mask)
        LOG_DEBUG(ShaderDecompiler, "tex r{} r{} r{} 0x{:x}", dst, coords_x,
                  coords_y, todo);

        // TODO: how does bindless work?
        LOG_NOT_IMPLEMENTED(ShaderDecompiler, "Bindless");
        // o->OpTextureSample(
        //     o->OpRegister(false, dst), o->OpRegister(false, dst + 1),
        //     o->OpRegister(false, dst + 2), o->OpRegister(false, dst + 3),
        //     const_buffer_index, o->OpRegister(true, coords_x),
        //     o->OpRegister(true, coords_y));
    }
    INST(0xde78000000000000, 0xfffc000000000000)
    LOG_NOT_IMPLEMENTED(ShaderDecompiler, "txd");
    INST(0xde38000000000000, 0xfffc000000000000)
    LOG_NOT_IMPLEMENTED(ShaderDecompiler, "txd");
    INST(0xdd38000000000000, 0xff38000000000000)
    LOG_NOT_IMPLEMENTED(ShaderDecompiler, "tld");
    INST(0xdc38000000000000, 0xff38000000000000)
    LOG_NOT_IMPLEMENTED(ShaderDecompiler, "tld");
    INST(0xd200000000000000, 0xf600000000000000)
    LOG_NOT_IMPLEMENTED(ShaderDecompiler, "tlds");
    INST(0xd000000000000000, 0xf600000000000000) {
        HANDLE_PRED_COND();

        // TODO: is the dst correct?
        const auto dst1 = GET_REG(28);
        const auto dst0 = GET_REG(0);
        const auto coords_x = GET_REG(8);
        const auto coords_y = GET_REG(20);
        const auto const_buffer_index = GET_VALUE_U32(36, 13);
        // TODO: texture type
        // TODO: component swizzle?
        LOG_DEBUG(ShaderDecompiler, "texs r{} r{} r{} r{} 0x{:08x}", dst1, dst0,
                  coords_x, coords_y, const_buffer_index);

        o->OpTextureSample(
            o->OpRegister(false, dst0), o->OpRegister(false, dst0 + 1),
            o->OpRegister(false, dst1), o->OpRegister(false, dst1 + 1),
            const_buffer_index, o->OpRegister(true, coords_x),
            o->OpRegister(true, coords_y));
    }
    INST(0xc838000000000000, 0xfc38000000000000)
    LOG_NOT_IMPLEMENTED(ShaderDecompiler, "tld4");
    INST(0xc038000000000000, 0xfc38000000000000)
    LOG_NOT_IMPLEMENTED(ShaderDecompiler, "tex");
    INST(0xa000000000000000, 0xe000000000000000)
    LOG_NOT_IMPLEMENTED(ShaderDecompiler, "st");
    INST(0x8000000000000000, 0xe000000000000000)
    LOG_NOT_IMPLEMENTED(ShaderDecompiler, "ld");
    INST(0x7e80000000000000, 0xfe80000000000000)
    LOG_NOT_IMPLEMENTED(ShaderDecompiler, "hsetp2");
    INST(0x7e00000000000000, 0xfe80000000000000)
    LOG_NOT_IMPLEMENTED(ShaderDecompiler, "hsetp2");
    INST(0x7c80000000000000, 0xfe80000000000000)
    LOG_NOT_IMPLEMENTED(ShaderDecompiler, "hset2");
    INST(0x7c00000000000000, 0xfe80000000000000)
    LOG_NOT_IMPLEMENTED(ShaderDecompiler, "hset2");
    INST(0x7a00000000000000, 0xfe80000000000000)
    LOG_NOT_IMPLEMENTED(ShaderDecompiler, "hadd2");
    INST(0x7a80000000000000, 0xfe80000000000000)
    LOG_NOT_IMPLEMENTED(ShaderDecompiler, "hadd2");
    INST(0x7800000000000000, 0xfe80000000000000)
    LOG_NOT_IMPLEMENTED(ShaderDecompiler, "hmul2");
    INST(0x7880000000000000, 0xfe80000000000000)
    LOG_NOT_IMPLEMENTED(ShaderDecompiler, "hmul2");
    INST(0x7080000000000000, 0xf880000000000000)
    LOG_NOT_IMPLEMENTED(ShaderDecompiler, "hfma2");
    INST(0x7000000000000000, 0xf880000000000000)
    LOG_NOT_IMPLEMENTED(ShaderDecompiler, "hfma2");
    INST(0x6080000000000000, 0xf880000000000000)
    LOG_NOT_IMPLEMENTED(ShaderDecompiler, "hfma2");
    INST(0x5f00000000000000, 0xff00000000000000)
    LOG_NOT_IMPLEMENTED(ShaderDecompiler, "vmad");
    INST(0x5d20000000000000, 0xfff8000000000000)
    LOG_NOT_IMPLEMENTED(ShaderDecompiler, "hsetp2");
    INST(0x5d18000000000000, 0xfff8000000000000)
    LOG_NOT_IMPLEMENTED(ShaderDecompiler, "hset2");
    INST(0x5d10000000000000, 0xfff8000000000000)
    LOG_NOT_IMPLEMENTED(ShaderDecompiler, "hadd2");
    INST(0x5d08000000000000, 0xfff8000000000000)
    LOG_NOT_IMPLEMENTED(ShaderDecompiler, "hmul2");
    INST(0x5d00000000000000, 0xfff8000000000000)
    LOG_NOT_IMPLEMENTED(ShaderDecompiler, "hfma2");
    INST(0x5cf8000000000000, 0xfff8000000000000)
    LOG_NOT_IMPLEMENTED(ShaderDecompiler, "shf");
    INST(0x5cf0000000000000, 0xfff8000000000000)
    LOG_NOT_IMPLEMENTED(ShaderDecompiler, "r2p");
    INST(0x5ce8000000000000, 0xfff8000000000000)
    LOG_NOT_IMPLEMENTED(ShaderDecompiler, "p2r");
    INST(0x5ce0000000000000, 0xfff8000000000000) {
        HANDLE_PRED_COND();

        // TODO: is dst and src type correct?
        const auto dst_type = get_operand_5ce0_0(inst);
        const auto src_type = get_operand_5ce0_1(inst);
        const auto dst = GET_REG(0);
        const auto src = GET_REG(20);
        LOG_DEBUG(ShaderDecompiler, "i2i {} {} r{} r{}", dst_type, src_type,
                  dst, src);

        auto res = o->OpCast(o->OpRegister(true, src, src_type), dst_type);
        o->OpMove(o->OpRegister(false, dst, dst_type), res);
    }
    INST(0x5cc0000000000000, 0xfff0000000000000)
    LOG_NOT_IMPLEMENTED(ShaderDecompiler, "iadd3");
    INST(0x5cb8000000000000, 0xfff8000000000000) {
        HANDLE_PRED_COND();

        const auto dst_type = get_operand_5cb8_0(inst);
        const auto src_type = get_operand_5cb8_1(inst);
        // TODO: 5cb8_2
        const auto dst = GET_REG(0);
        const auto src = GET_REG(20);
        LOG_DEBUG(ShaderDecompiler, "i2f {} {} r{} r{}", dst_type, src_type,
                  dst, src);

        auto res = o->OpCast(o->OpRegister(true, src, src_type), dst_type);
        o->OpMove(o->OpRegister(false, dst, dst_type), res);
    }
    INST(0x5cb0000000000000, 0xfff8000000000000)
    LOG_NOT_IMPLEMENTED(ShaderDecompiler, "f2i");
    INST(0x5ca8000000000000, 0xfff8000000000000) {
        HANDLE_PRED_COND();

        const auto dst_type = get_operand_5cb0_0(inst);
        const auto src_type = get_operand_5cb8_0(inst);
        // TODO: 5ca8_0
        const auto dst = GET_REG(0);
        const auto src = GET_REG(20);
        LOG_DEBUG(ShaderDecompiler, "f2f {} {} r{} r{}", dst_type, src_type,
                  dst, src);

        auto res = o->OpCast(o->OpRegister(true, src, src_type), dst_type);
        o->OpMove(o->OpRegister(false, dst, dst_type), res);
    }
    INST(0x5ca0000000000000, 0xfff8000000000000)
    LOG_NOT_IMPLEMENTED(ShaderDecompiler, "sel");
    INST(0x5c98000000000000, 0xfff8000000000000) {
        HANDLE_PRED_COND();

        const auto dst = GET_REG(0);
        const auto src = GET_REG(20);
        const auto todo = GET_VALUE_U32(39, 4); // TODO: what is this?
        LOG_DEBUG(ShaderDecompiler, "mov r{} r{} 0x{:x}", dst, src, todo);

        o->OpMove(o->OpRegister(false, dst), o->OpRegister(true, src));
    }
    INST(0x5c90000000000000, 0xfff8000000000000) {
        HANDLE_PRED_COND();

        // TODO: mode
        const auto dst = GET_REG(0);
        const auto src = GET_REG(20);
        LOG_DEBUG(ShaderDecompiler, "rro r{} r{}", dst, src);

        // TODO: is it okay to just move?
        o->OpMove(o->OpRegister(false, dst), o->OpRegister(true, src));
    }
    INST(0x5c88000000000000, 0xfff8000000000000)
    LOG_NOT_IMPLEMENTED(ShaderDecompiler, "fchk");
    INST(0x5c80000000000000, 0xfff8000000000000)
    LOG_NOT_IMPLEMENTED(ShaderDecompiler, "dmul");
    INST(0x5c70000000000000, 0xfff8000000000000)
    LOG_NOT_IMPLEMENTED(ShaderDecompiler, "dadd");
    INST(0x5c68000000000000, 0xfff8000000000000) {
        HANDLE_PRED_COND();

        const auto dst = GET_REG(0);
        const auto srcA = GET_REG(8);
        const auto srcB = GET_REG(20);
        LOG_DEBUG(ShaderDecompiler, "fmul r{} r{} r{}", dst, srcA, srcB);

        auto res = o->OpMultiply(o->OpRegister(true, srcA, DataType::F32),
                                 o->OpRegister(true, srcB, DataType::F32));
        o->OpMove(o->OpRegister(false, dst, DataType::F32), res);
    }
    INST(0x5c60000000000000, 0xfff8000000000000) {
        HANDLE_PRED_COND();

        const auto dst = GET_REG(0);
        const auto srcA = GET_REG(8);
        const auto srcB = GET_REG(20);
        const auto pred = GET_PRED(39);
        LOG_DEBUG(ShaderDecompiler, "fmnmx r{} r{} r{} p{}", dst, srcA, srcB,
                  pred);

        auto srcA_v = o->OpRegister(false, srcA, DataType::F32);
        auto srcB_v = o->OpRegister(false, srcB, DataType::F32);
        auto min_v = o->OpMin(srcA_v, srcB_v);
        auto max_v = o->OpMax(srcA_v, srcB_v);
        auto res = o->OpSelect(o->OpPredicate(true, pred), max_v,
                               min_v); // TODO: correct?
        o->OpMove(o->OpRegister(false, dst, DataType::F32), res);
    }
    INST(0x5c58000000000000, 0xfff8000000000000) {
        HANDLE_PRED_COND();

        const auto dst = GET_REG(0);
        const bool negA = GET_BIT(48);
        const auto srcA = GET_REG(8);
        const bool negB = GET_BIT(45);
        const auto srcB = GET_REG(20);
        LOG_DEBUG(ShaderDecompiler, "fadd r{} {}r{} {}r{}", dst,
                  negA ? "-" : "", srcA, negB ? "-" : "", srcB);

        auto res = o->OpAdd(o->OpRegister(true, srcA, DataType::F32, negA),
                            o->OpRegister(true, srcB, DataType::F32, negB));
        o->OpMove(o->OpRegister(false, dst, DataType::F32), res);
    }
    INST(0x5c50000000000000, 0xfff8000000000000)
    LOG_NOT_IMPLEMENTED(ShaderDecompiler, "dmnmx");
    INST(0x5c48000000000000, 0xfff8000000000000)
    LOG_NOT_IMPLEMENTED(ShaderDecompiler, "shl");
    INST(0x5c40000000000000, 0xfff8000000000000)
    LOG_NOT_IMPLEMENTED(ShaderDecompiler, "lop");
    INST(0x5c38000000000000, 0xfff8000000000000)
    LOG_NOT_IMPLEMENTED(ShaderDecompiler, "imul");
    INST(0x5c30000000000000, 0xfff8000000000000)
    LOG_NOT_IMPLEMENTED(ShaderDecompiler, "flo");
    INST(0x5c28000000000000, 0xfff8000000000000)
    LOG_NOT_IMPLEMENTED(ShaderDecompiler, "shr");
    INST(0x5c20000000000000, 0xfff8000000000000)
    LOG_NOT_IMPLEMENTED(ShaderDecompiler, "imnmx");
    INST(0x5c18000000000000, 0xfff8000000000000)
    LOG_NOT_IMPLEMENTED(ShaderDecompiler, "iscadd");
    INST(0x5c10000000000000, 0xfff8000000000000)
    LOG_NOT_IMPLEMENTED(ShaderDecompiler, "iadd");
    INST(0x5c08000000000000, 0xfff8000000000000)
    LOG_NOT_IMPLEMENTED(ShaderDecompiler, "popc");
    INST(0x5c00000000000000, 0xfff8000000000000)
    LOG_NOT_IMPLEMENTED(ShaderDecompiler, "bfe");
    INST(0x5bf8000000000000, 0xfff8000000000000)
    LOG_NOT_IMPLEMENTED(ShaderDecompiler, "shf");
    INST(0x5bf0000000000000, 0xfff8000000000000)
    LOG_NOT_IMPLEMENTED(ShaderDecompiler, "bfi");
    INST(0x5be0000000000000, 0xfff8000000000000)
    LOG_NOT_IMPLEMENTED(ShaderDecompiler, "lop3");
    INST(0x5bd8000000000000, 0xfff8000000000000)
    LOG_NOT_IMPLEMENTED(ShaderDecompiler, "lea");
    INST(0x5bd0000000000000, 0xfff8000000000000)
    LOG_NOT_IMPLEMENTED(ShaderDecompiler, "lea");
    INST(0x5bc0000000000000, 0xfff0000000000000)
    LOG_NOT_IMPLEMENTED(ShaderDecompiler, "prmt");
    INST(0x5bb0000000000000, 0xfff0000000000000) {
        HANDLE_PRED_COND();

        const auto cmp = get_operand_5bb0_0(inst);
        const auto combine_bin = get_operand_5bb0_1(inst);
        const auto dst = GET_PRED(3);
        const auto combine = GET_PRED(0); // TODO: combine?
        const auto srcA = GET_REG(8);
        const auto srcB = GET_REG(20);
        // TODO: pred 39
        LOG_DEBUG(ShaderDecompiler, "fsetp {} {} p{} p{} r{} r{}", cmp,
                  combine_bin, dst, combine, srcA, srcB);

        auto cmp_res =
            o->OpCompare(cmp, o->OpRegister(true, srcA, DataType::F32),
                         o->OpRegister(true, srcB, DataType::F32));
        auto bin_res =
            o->OpBinary(combine_bin, cmp_res, o->OpPredicate(true, combine));
        o->OpMove(o->OpPredicate(false, dst), bin_res);
    }
    INST(0x5ba0000000000000, 0xfff0000000000000)
    LOG_NOT_IMPLEMENTED(ShaderDecompiler, "fcmp");
    INST(0x5b80000000000000, 0xfff0000000000000)
    LOG_NOT_IMPLEMENTED(ShaderDecompiler, "dsetp");
    INST(0x5b70000000000000, 0xfff0000000000000)
    LOG_NOT_IMPLEMENTED(ShaderDecompiler, "dfma");
    INST(0x5b60000000000000, 0xfff0000000000000) {
        HANDLE_PRED_COND();

        const auto cmp = get_operand_5b60_0(inst);
        const auto type = get_operand_5c30_0(inst);
        const auto combine_bin = get_operand_5bb0_1(inst);
        const auto dst = GET_PRED(3);
        const auto combine = GET_PRED(0); // TODO: combine?
        const auto srcA = GET_REG(8);
        const auto srcB = GET_REG(20);
        // TODO: pred 39
        LOG_DEBUG(ShaderDecompiler, "isetp {} {} {} p{} p{} r{} r{}", cmp, type,
                  combine_bin, dst, combine, srcA, srcB);

        auto cmp_res = o->OpCompare(cmp, o->OpRegister(true, srcA, type),
                                    o->OpRegister(true, srcB, type));
        auto bin_res =
            o->OpBinary(combine_bin, cmp_res, o->OpPredicate(true, combine));
        o->OpMove(o->OpPredicate(false, dst), bin_res);
    }
    INST(0x5b50000000000000, 0xfff0000000000000)
    LOG_NOT_IMPLEMENTED(ShaderDecompiler, "iset");
    INST(0x5b40000000000000, 0xfff0000000000000)
    LOG_NOT_IMPLEMENTED(ShaderDecompiler, "icmp");
    INST(0x5b00000000000000, 0xffc0000000000000)
    LOG_NOT_IMPLEMENTED(ShaderDecompiler, "xmad");
    INST(0x5a80000000000000, 0xff80000000000000)
    LOG_NOT_IMPLEMENTED(ShaderDecompiler, "imadsp");
    INST(0x5a00000000000000, 0xff80000000000000)
    LOG_NOT_IMPLEMENTED(ShaderDecompiler, "imad");
    INST(0x5980000000000000, 0xff80000000000000) {
        HANDLE_PRED_COND();

        const auto dst = GET_REG(0);
        const auto srcA = GET_REG(8);
        const auto negB = GET_BIT(48);
        const auto srcB = GET_REG(20);
        const auto negC = GET_BIT(49);
        const auto srcC = GET_REG(39);
        LOG_DEBUG(ShaderDecompiler, "ffma r{} r{} {}r{} {}r{}", dst, srcA,
                  (negB ? "-" : ""), srcB, (negC ? "-" : ""), srcC);

        auto res =
            o->OpFloatFma(o->OpRegister(true, srcA, DataType::F32),
                          o->OpRegister(true, srcB, DataType::F32, negB),
                          o->OpRegister(true, srcC, DataType::F32, negC));
        o->OpMove(o->OpRegister(false, dst, DataType::F32), res);
    }
    INST(0x5900000000000000, 0xff80000000000000)
    LOG_NOT_IMPLEMENTED(ShaderDecompiler, "dset");
    INST(0x5800000000000000, 0xff00000000000000) {
        HANDLE_PRED_COND();

        // TODO: some sort of boolean flag at bit 52 to indicate whether to the
        // result is a float or an integer
        const auto cmp = get_operand_5bb0_0(inst);
        const auto combine_bin = get_operand_5bb0_1(inst);
        const auto dst = GET_REG(0);
        const auto srcA = GET_REG(8);
        const auto negA = GET_BIT(43);
        const auto srcB = GET_REG(20);
        const auto negB = GET_BIT(53);
        // TODO: combine
        LOG_DEBUG(ShaderDecompiler, "fset {} {} r{} {}r{} {}r{}", cmp,
                  combine_bin, dst, negA ? "-" : "", srcA, negB ? "-" : "",
                  srcB);

        auto cmp_res =
            o->OpCompare(cmp, o->OpRegister(true, srcA, DataType::F32, negA),
                         o->OpRegister(true, srcB, DataType::F32, negB));
        // TODO: uncomment
        // auto bin_res = o->OpBinary(combine_bin, cmp_res,
        //                    o->OpPredicate(true, combine));
        // TODO: if float flags
        // TODO: use bin_res instead of cmp_res
        // TODO: simplify immediate value creation
        if (false) {
            auto res = o->OpSelect(
                cmp_res,
                o->OpImmediateL(std::bit_cast<u32>(f32(1.0f)), DataType::F32),
                o->OpImmediateL(std::bit_cast<u32>(f32(0.0f)), DataType::F32));
            o->OpMove(o->OpRegister(false, dst, DataType::F32), res);
        } else {
            auto res = o->OpCast(cmp_res, DataType::U32);
            o->OpMove(o->OpRegister(false, dst), res);
        }
    }
    INST(0x5700000000000000, 0xff00000000000000)
    LOG_NOT_IMPLEMENTED(ShaderDecompiler, "vshl");
    INST(0x5600000000000000, 0xff00000000000000)
    LOG_NOT_IMPLEMENTED(ShaderDecompiler, "vshr");
    INST(0x5400000000000000, 0xff00000000000000)
    LOG_NOT_IMPLEMENTED(ShaderDecompiler, "vabsdiff");
    INST(0x53f8000000000000, 0xfff8000000000000)
    LOG_NOT_IMPLEMENTED(ShaderDecompiler, "idp");
    INST(0x53f0000000000000, 0xfff8000000000000)
    LOG_NOT_IMPLEMENTED(ShaderDecompiler, "bfi");
    INST(0x53d8000000000000, 0xfff8000000000000)
    LOG_NOT_IMPLEMENTED(ShaderDecompiler, "idp");
    INST(0x53c0000000000000, 0xfff0000000000000)
    LOG_NOT_IMPLEMENTED(ShaderDecompiler, "prmt");
    INST(0x53a0000000000000, 0xfff0000000000000)
    LOG_NOT_IMPLEMENTED(ShaderDecompiler, "fcmp");
    INST(0x5370000000000000, 0xfff0000000000000)
    LOG_NOT_IMPLEMENTED(ShaderDecompiler, "dfma");
    INST(0x5340000000000000, 0xfff0000000000000)
    LOG_NOT_IMPLEMENTED(ShaderDecompiler, "icmp");
    INST(0x5280000000000000, 0xff80000000000000)
    LOG_NOT_IMPLEMENTED(ShaderDecompiler, "imadsp");
    INST(0x5200000000000000, 0xff80000000000000)
    LOG_NOT_IMPLEMENTED(ShaderDecompiler, "imad");
    INST(0x5180000000000000, 0xff80000000000000) {
        HANDLE_PRED_COND();

        const auto dst = GET_REG(0);
        const auto srcA = GET_REG(8);
        const auto negB = GET_BIT(48);
        const auto srcB = GET_REG(39);
        const auto negC = GET_BIT(49);
        const auto srcC = GET_CMEM(34, 14);
        LOG_DEBUG(ShaderDecompiler, "ffma r{} r{} {}r{} {}c{}[0x{:x}]", dst,
                  srcA, (negB ? "-" : ""), srcB, (negC ? "-" : ""), srcC.idx,
                  srcC.imm);

        auto res = o->OpFloatFma(o->OpRegister(true, srcA, DataType::F32),
                                 o->OpRegister(true, srcB, DataType::F32, negB),
                                 o->OpConstMemoryL(srcC, DataType::F32, negC));
        o->OpMove(o->OpRegister(false, dst, DataType::F32), res);
    }
    INST(0x5100000000000000, 0xff80000000000000)
    LOG_NOT_IMPLEMENTED(ShaderDecompiler, "xmad");
    INST(0x50f8000000000000, 0xfff8000000000000)
    LOG_NOT_IMPLEMENTED(ShaderDecompiler, "fswzadd");
    INST(0x50f0000000000000, 0xfff8000000000000)
    LOG_NOT_IMPLEMENTED(ShaderDecompiler, "vsetp");
    INST(0x50e0000000000000, 0xfff8000000000000)
    LOG_NOT_IMPLEMENTED(ShaderDecompiler, "vote");
    INST(0x50d8000000000000, 0xfff8000000000000)
    LOG_NOT_IMPLEMENTED(ShaderDecompiler, "vote");
    INST(0x50d0000000000000, 0xfff8000000000000)
    LOG_NOT_IMPLEMENTED(ShaderDecompiler, "lepc");
    INST(0x50c8000000000000, 0xfff8000000000000)
    LOG_NOT_IMPLEMENTED(ShaderDecompiler, "cs2r");
    INST(0x50b0000000000000, 0xfff8000000000000) {
        LOG_DEBUG(ShaderDecompiler, "nop");
    }
    INST(0x50a0000000000000, 0xfff8000000000000)
    LOG_NOT_IMPLEMENTED(ShaderDecompiler, "csetp");
    INST(0x5098000000000000, 0xfff8000000000000)
    LOG_NOT_IMPLEMENTED(ShaderDecompiler, "cset");
    INST(0x5090000000000000, 0xfff8000000000000) {
        HANDLE_PRED_COND();

        const auto bin = get_operand_5090_0(inst);
        const auto combine_bin = get_operand_5bb0_1(inst);
        const auto dst = GET_PRED(3);
        const auto combine = GET_PRED(0); // TODO: combine?
        const auto srcA = GET_PRED(12);
        const auto srcB = GET_PRED(29);
        const auto srcC = GET_PRED(39);
        LOG_DEBUG(ShaderDecompiler, "psetp {} {} p{} p{} p{} p{} p{}", bin,
                  combine_bin, dst, combine, srcA, srcB, srcC);

        auto bin1_res = o->OpBinary(bin, o->OpPredicate(true, srcA),
                                    o->OpPredicate(true, srcB));
        auto bin2_res = o->OpBinary(bin, bin1_res, o->OpPredicate(true, srcC));
        auto bin3_res =
            o->OpBinary(combine_bin, bin2_res, o->OpPredicate(true, combine));
        o->OpMove(o->OpPredicate(false, dst), bin3_res);
    }
    INST(0x5088000000000000, 0xfff8000000000000)
    LOG_NOT_IMPLEMENTED(ShaderDecompiler, "pset");
    INST(0x5080000000000000, 0xfff8000000000000) {
        HANDLE_PRED_COND();

        const auto func = get_operand_5080_0(inst);
        const auto dst = GET_REG(0);
        const auto src = GET_REG(8);
        LOG_DEBUG(ShaderDecompiler, "mufu {} r{} r{}", func, dst, src);

        auto res =
            o->OpMathFunction(func, o->OpRegister(true, src, DataType::F32));
        o->OpMove(o->OpRegister(false, dst, DataType::F32), res);
    }
    INST(0x5000000000000000, 0xff80000000000000)
    LOG_NOT_IMPLEMENTED(ShaderDecompiler, "vabsdiff4");
    INST(0x4e00000000000000, 0xfe00000000000000)
    LOG_NOT_IMPLEMENTED(ShaderDecompiler, "xmad");
    INST(0x4cf0000000000000, 0xfff8000000000000)
    LOG_NOT_IMPLEMENTED(ShaderDecompiler, "r2p");
    INST(0x4ce8000000000000, 0xfff8000000000000)
    LOG_NOT_IMPLEMENTED(ShaderDecompiler, "p2r");
    INST(0x4ce0000000000000, 0xfff8000000000000) {
        HANDLE_PRED_COND();

        // TODO: is dst and src type correct?
        const auto dst_type = get_operand_5ce0_0(inst);
        const auto src_type = get_operand_5ce0_1(inst);
        const auto dst = GET_REG(0);
        const auto src = GET_CMEM(34, 14);
        LOG_DEBUG(ShaderDecompiler, "i2i {} {} r{} c{}[0x{:x}]", dst_type,
                  src_type, dst, src.idx, src.imm);

        auto res = o->OpCast(o->OpConstMemoryL(src, src_type), dst_type);
        o->OpMove(o->OpRegister(false, dst, dst_type), res);
    }
    INST(0x4cc0000000000000, 0xfff0000000000000)
    LOG_NOT_IMPLEMENTED(ShaderDecompiler, "iadd3");
    INST(0x4cb8000000000000, 0xfff8000000000000)
    LOG_NOT_IMPLEMENTED(ShaderDecompiler, "i2f");
    INST(0x4cb0000000000000, 0xfff8000000000000)
    LOG_NOT_IMPLEMENTED(ShaderDecompiler, "f2i");
    INST(0x4ca8000000000000, 0xfff8000000000000)
    LOG_NOT_IMPLEMENTED(ShaderDecompiler, "f2f");
    INST(0x4ca0000000000000, 0xfff8000000000000)
    LOG_NOT_IMPLEMENTED(ShaderDecompiler, "sel");
    INST(0x4c98000000000000, 0xfff8000000000000) {
        HANDLE_PRED_COND();

        const auto dst = GET_REG(0);
        const auto src = GET_CMEM(34, 14);
        const auto todo = GET_VALUE_U32(39, 4);
        LOG_DEBUG(ShaderDecompiler, "mov r{} c{}[0x{:x}] 0x{:x}", dst, src.idx,
                  src.imm, todo);

        o->OpMove(o->OpRegister(false, dst), o->OpConstMemoryL(src));
    }
    INST(0x4c90000000000000, 0xfff8000000000000)
    LOG_NOT_IMPLEMENTED(ShaderDecompiler, "rro");
    INST(0x4c88000000000000, 0xfff8000000000000)
    LOG_NOT_IMPLEMENTED(ShaderDecompiler, "fchk");
    INST(0x4c80000000000000, 0xfff8000000000000)
    LOG_NOT_IMPLEMENTED(ShaderDecompiler, "dmul");
    INST(0x4c70000000000000, 0xfff8000000000000)
    LOG_NOT_IMPLEMENTED(ShaderDecompiler, "dadd");
    INST(0x4c68000000000000, 0xfff8000000000000) {
        HANDLE_PRED_COND();

        const auto dst = GET_REG(0);
        const auto srcA = GET_REG(8);
        const auto srcB = GET_CMEM(34, 14);
        LOG_DEBUG(ShaderDecompiler, "fmul r{} r{} c{}[0x{:x}]", dst, srcA,
                  srcB.idx, srcB.imm);

        auto res = o->OpMultiply(o->OpRegister(false, srcA, DataType::F32),
                                 o->OpConstMemoryL(srcB, DataType::F32));
        o->OpMove(o->OpRegister(false, dst, DataType::F32), res);
    }
    INST(0x4c60000000000000, 0xfff8000000000000)
    LOG_NOT_IMPLEMENTED(ShaderDecompiler, "fmnmx");
    INST(0x4c58000000000000, 0xfff8000000000000) {
        HANDLE_PRED_COND();

        const auto dst = GET_REG(0);
        const bool negA = GET_BIT(48);
        const auto srcA = GET_REG(8);
        const bool negB = GET_BIT(45);
        const auto srcB = GET_CMEM(34, 14);
        LOG_DEBUG(ShaderDecompiler, "fadd r{} {}r{} {}c{}[0x{:x}]", dst,
                  (negA ? "-" : ""), srcA, (negB ? "-" : ""), srcB.idx,
                  srcB.imm);

        auto res = o->OpAdd(o->OpRegister(false, srcA, DataType::F32, negA),
                            o->OpConstMemoryL(srcB, DataType::F32, negB));
        o->OpMove(o->OpRegister(false, dst, DataType::F32), res);
    }
    INST(0x4c50000000000000, 0xfff8000000000000)
    LOG_NOT_IMPLEMENTED(ShaderDecompiler, "dmnmx");
    INST(0x4c48000000000000, 0xfff8000000000000)
    LOG_NOT_IMPLEMENTED(ShaderDecompiler, "shl");
    INST(0x4c40000000000000, 0xfff8000000000000)
    LOG_NOT_IMPLEMENTED(ShaderDecompiler, "lop");
    INST(0x4c38000000000000, 0xfff8000000000000)
    LOG_NOT_IMPLEMENTED(ShaderDecompiler, "imul");
    INST(0x4c30000000000000, 0xfff8000000000000)
    LOG_NOT_IMPLEMENTED(ShaderDecompiler, "flo");
    INST(0x4c28000000000000, 0xfff8000000000000)
    LOG_NOT_IMPLEMENTED(ShaderDecompiler, "shr");
    INST(0x4c20000000000000, 0xfff8000000000000)
    LOG_NOT_IMPLEMENTED(ShaderDecompiler, "imnmx");
    INST(0x4c18000000000000, 0xfff8000000000000)
    LOG_NOT_IMPLEMENTED(ShaderDecompiler, "iscadd");
    INST(0x4c10000000000000, 0xfff8000000000000)
    LOG_NOT_IMPLEMENTED(ShaderDecompiler, "iadd");
    INST(0x4c08000000000000, 0xfff8000000000000)
    LOG_NOT_IMPLEMENTED(ShaderDecompiler, "popc");
    INST(0x4c00000000000000, 0xfff8000000000000)
    LOG_NOT_IMPLEMENTED(ShaderDecompiler, "bfe");
    INST(0x4bf0000000000000, 0xfff8000000000000)
    LOG_NOT_IMPLEMENTED(ShaderDecompiler, "bfi");
    INST(0x4bd0000000000000, 0xfff8000000000000)
    LOG_NOT_IMPLEMENTED(ShaderDecompiler, "lea");
    INST(0x4bc0000000000000, 0xfff0000000000000)
    LOG_NOT_IMPLEMENTED(ShaderDecompiler, "prmt");
    INST(0x4bb0000000000000, 0xfff0000000000000) {
        HANDLE_PRED_COND();

        const auto cmp = get_operand_5bb0_0(inst);
        const auto combine_bin = get_operand_5bb0_1(inst);
        const auto dst = GET_PRED(3);
        const auto combine = GET_PRED(0); // TODO: combine?
        const auto srcA = GET_REG(8);
        const auto srcB = GET_CMEM(34, 14);
        LOG_DEBUG(ShaderDecompiler, "fsetp {} {} p{} p{} r{} c{}[0x{:x}]", cmp,
                  combine_bin, dst, combine, srcA, srcB.idx, srcB.imm);

        auto cmp_res =
            o->OpCompare(cmp, o->OpRegister(true, srcA, DataType::F32),
                         o->OpConstMemoryL(srcB, DataType::F32));
        auto bin_res =
            o->OpBinary(combine_bin, cmp_res, o->OpPredicate(true, combine));
        o->OpMove(o->OpPredicate(false, dst), bin_res);
    }
    INST(0x4ba0000000000000, 0xfff0000000000000)
    LOG_NOT_IMPLEMENTED(ShaderDecompiler, "fcmp");
    INST(0x4b80000000000000, 0xfff0000000000000)
    LOG_NOT_IMPLEMENTED(ShaderDecompiler, "dsetp");
    INST(0x4b70000000000000, 0xfff0000000000000)
    LOG_NOT_IMPLEMENTED(ShaderDecompiler, "dfma");
    INST(0x4b60000000000000, 0xfff0000000000000)
    LOG_NOT_IMPLEMENTED(ShaderDecompiler, "isetp");
    INST(0x4b50000000000000, 0xfff0000000000000)
    LOG_NOT_IMPLEMENTED(ShaderDecompiler, "iset");
    INST(0x4b40000000000000, 0xfff0000000000000)
    LOG_NOT_IMPLEMENTED(ShaderDecompiler, "icmp");
    INST(0x4a80000000000000, 0xff80000000000000)
    LOG_NOT_IMPLEMENTED(ShaderDecompiler, "imadsp");
    INST(0x4a00000000000000, 0xff80000000000000)
    LOG_NOT_IMPLEMENTED(ShaderDecompiler, "imad");
    INST(0x4980000000000000, 0xff80000000000000) {
        HANDLE_PRED_COND();

        const auto dst = GET_REG(0);
        const auto srcA = GET_REG(8);
        const auto negB = GET_BIT(48);
        const auto srcB = GET_CMEM(34, 14);
        const auto negC = GET_BIT(49);
        const auto srcC = GET_REG(39);
        LOG_DEBUG(ShaderDecompiler, "ffma r{} r{} {}c{}[0x{:x}] {}r{}", dst,
                  srcA, (negB ? "-" : ""), srcB.idx, srcB.imm,
                  (negC ? "-" : ""), srcC);

        auto res =
            o->OpFloatFma(o->OpRegister(true, srcA, DataType::F32),
                          o->OpConstMemoryL(srcB, DataType::F32, negB),
                          o->OpRegister(true, srcC, DataType::F32, negC));
        o->OpMove(o->OpRegister(false, dst, DataType::F32), res);
    }
    INST(0x4900000000000000, 0xff80000000000000)
    LOG_NOT_IMPLEMENTED(ShaderDecompiler, "dset");
    INST(0x4800000000000000, 0xfe00000000000000)
    LOG_NOT_IMPLEMENTED(ShaderDecompiler, "fset");
    INST(0x4000000000000000, 0xfe00000000000000)
    LOG_NOT_IMPLEMENTED(ShaderDecompiler, "vset");
    INST(0x3c00000000000000, 0xfc00000000000000)
    LOG_NOT_IMPLEMENTED(ShaderDecompiler, "lop3");
    INST(0x3a00000000000000, 0xfe00000000000000)
    LOG_NOT_IMPLEMENTED(ShaderDecompiler, "vmnmx");
    INST(0x38f8000000000000, 0xfef8000000000000)
    LOG_NOT_IMPLEMENTED(ShaderDecompiler, "shf");
    INST(0x38f0000000000000, 0xfef8000000000000)
    LOG_NOT_IMPLEMENTED(ShaderDecompiler, "r2p");
    INST(0x38e8000000000000, 0xfef8000000000000)
    LOG_NOT_IMPLEMENTED(ShaderDecompiler, "p2r");
    INST(0x38e0000000000000, 0xfef8000000000000) {
        HANDLE_PRED_COND();

        // TODO: is dst and src type correct?
        const auto dst_type = get_operand_5ce0_0(inst);
        const auto src_type = get_operand_5ce0_1(inst);
        const auto dst = GET_REG(0);
        const auto src = GET_VALUE_U32(20, 19) |
                         (GET_VALUE_U32(56, 1) << 19); // TODO: correct?
        LOG_DEBUG(ShaderDecompiler, "i2i {} {} r{} 0x{:x}", dst_type, src_type,
                  dst, src);

        auto res = o->OpCast(o->OpImmediateL(src, src_type), dst_type);
        o->OpMove(o->OpRegister(false, dst, dst_type), res);
    }
    INST(0x38c0000000000000, 0xfef0000000000000)
    LOG_NOT_IMPLEMENTED(ShaderDecompiler, "iadd3");
    INST(0x38b8000000000000, 0xfef8000000000000)
    LOG_NOT_IMPLEMENTED(ShaderDecompiler, "i2f");
    INST(0x38b0000000000000, 0xfef8000000000000)
    LOG_NOT_IMPLEMENTED(ShaderDecompiler, "f2i");
    INST(0x38a8000000000000, 0xfef8000000000000)
    LOG_NOT_IMPLEMENTED(ShaderDecompiler, "f2f");
    INST(0x38a0000000000000, 0xfef8000000000000)
    LOG_NOT_IMPLEMENTED(ShaderDecompiler, "f2f");
    INST(0x3898000000000000, 0xfef8000000000000)
    LOG_NOT_IMPLEMENTED(ShaderDecompiler, "mov");
    INST(0x3890000000000000, 0xfef8000000000000)
    LOG_NOT_IMPLEMENTED(ShaderDecompiler, "rro");
    INST(0x3888000000000000, 0xfef8000000000000)
    LOG_NOT_IMPLEMENTED(ShaderDecompiler, "fchk");
    INST(0x3880000000000000, 0xfef8000000000000)
    LOG_NOT_IMPLEMENTED(ShaderDecompiler, "dmul");
    INST(0x3870000000000000, 0xfef8000000000000)
    LOG_NOT_IMPLEMENTED(ShaderDecompiler, "dadd");
    INST(0x3868000000000000, 0xfef8000000000000) {
        HANDLE_PRED_COND();

        const auto dst = GET_REG(0);
        const auto srcA = GET_REG(8);
        const auto srcB = GET_VALUE_F32();
        LOG_DEBUG(ShaderDecompiler, "fmul r{} r{} 0x{:08x}", dst, srcA, srcB);

        auto res = o->OpMultiply(o->OpRegister(false, srcA, DataType::F32),
                                 o->OpImmediateL(srcB, DataType::F32));
        o->OpMove(o->OpRegister(false, dst, DataType::F32), res);
    }
    INST(0x3860000000000000, 0xfef8000000000000) {
        HANDLE_PRED_COND();

        const auto dst = GET_REG(0);
        const auto srcA = GET_REG(8);
        const auto srcB = GET_VALUE_F32();
        const auto pred = GET_PRED(39);
        LOG_DEBUG(ShaderDecompiler, "fmnmx r{} r{} 0x{:08x} p{}", dst, srcA,
                  srcB, pred);

        auto srcA_v = o->OpRegister(false, srcA, DataType::F32);
        auto srcB_v = o->OpImmediateL(srcB, DataType::F32);
        auto min_v = o->OpMin(srcA_v, srcB_v);
        auto max_v = o->OpMax(srcA_v, srcB_v);
        auto res = o->OpSelect(o->OpPredicate(true, pred), max_v,
                               min_v); // TODO: correct?
        o->OpMove(o->OpRegister(false, dst, DataType::F32), res);
    }
    INST(0x3858000000000000, 0xfef8000000000000) {
        HANDLE_PRED_COND();

        const auto dst = GET_REG(0);
        const bool negA = GET_BIT(48);
        const auto srcA = GET_REG(8);
        const bool negB = GET_BIT(45);
        const auto srcB = GET_VALUE_F32();
        LOG_DEBUG(ShaderDecompiler, "fadd r{} {}r{} {}0x{:x}", dst,
                  negA ? "-" : "", srcA, negB ? "-" : "", srcB);

        auto res = o->OpAdd(o->OpRegister(false, srcA, DataType::F32, negA),
                            o->OpImmediateL(srcB, DataType::F32, negB));
        o->OpMove(o->OpRegister(false, dst, DataType::F32), res);
    }
    INST(0x3850000000000000, 0xfef8000000000000)
    LOG_NOT_IMPLEMENTED(ShaderDecompiler, "dmnmx");
    INST(0x3848000000000000, 0xfef8000000000000) {
        HANDLE_PRED_COND();

        const auto dst = GET_REG(0);
        const auto src = GET_REG(8);
        const auto shift = GET_VALUE_U32(20, 19) |
                           (GET_VALUE_U32(56, 1) << 19); // TODO: correct?
        LOG_DEBUG(ShaderDecompiler, "shl r{} r{} 0x{:x}", dst, src, shift);

        auto res = o->OpShiftLeft(o->OpRegister(true, src), shift);
        o->OpMove(o->OpRegister(false, dst), res);
    }
    INST(0x3840000000000000, 0xfef8000000000000)
    LOG_NOT_IMPLEMENTED(ShaderDecompiler, "lop");
    INST(0x3838000000000000, 0xfef8000000000000)
    LOG_NOT_IMPLEMENTED(ShaderDecompiler, "imul");
    INST(0x3830000000000000, 0xfef8000000000000)
    LOG_NOT_IMPLEMENTED(ShaderDecompiler, "flo");
    INST(0x3828000000000000, 0xfef8000000000000)
    LOG_NOT_IMPLEMENTED(ShaderDecompiler, "shr");
    INST(0x3820000000000000, 0xfef8000000000000)
    LOG_NOT_IMPLEMENTED(ShaderDecompiler, "imnmx");
    INST(0x3818000000000000, 0xfef8000000000000)
    LOG_NOT_IMPLEMENTED(ShaderDecompiler, "iscadd");
    INST(0x3810000000000000, 0xfef8000000000000)
    LOG_NOT_IMPLEMENTED(ShaderDecompiler, "iadd");
    INST(0x3808000000000000, 0xfef8000000000000)
    LOG_NOT_IMPLEMENTED(ShaderDecompiler, "popc");
    INST(0x3800000000000000, 0xfef8000000000000)
    LOG_NOT_IMPLEMENTED(ShaderDecompiler, "bfe");
    INST(0x36f8000000000000, 0xfef8000000000000)
    LOG_NOT_IMPLEMENTED(ShaderDecompiler, "shf");
    INST(0x36f0000000000000, 0xfef8000000000000)
    LOG_NOT_IMPLEMENTED(ShaderDecompiler, "bfi");
    INST(0x36d0000000000000, 0xfef8000000000000)
    LOG_NOT_IMPLEMENTED(ShaderDecompiler, "lea");
    INST(0x36c0000000000000, 0xfef0000000000000)
    LOG_NOT_IMPLEMENTED(ShaderDecompiler, "prmt");
    INST(0x36b0000000000000, 0xfef0000000000000) {
        HANDLE_PRED_COND();

        const auto cmp = get_operand_5bb0_0(inst);
        const auto combine_bin = get_operand_5bb0_1(inst);
        const auto dst = GET_PRED(3);
        const auto combine = GET_PRED(0); // TODO: combine?
        const auto srcA = GET_REG(8);
        const auto srcB = GET_VALUE_F32();
        LOG_DEBUG(ShaderDecompiler, "fsetp {} {} p{} p{} r{} 0x{:08x}", cmp,
                  combine_bin, dst, combine, srcA, srcB);

        auto cmp_res =
            o->OpCompare(cmp, o->OpRegister(true, srcA, DataType::F32),
                         o->OpImmediateL(srcB, DataType::F32));
        auto bin_res =
            o->OpBinary(combine_bin, cmp_res, o->OpPredicate(true, combine));
        o->OpMove(o->OpPredicate(false, dst), bin_res);
    }
    INST(0x36a0000000000000, 0xfef0000000000000)
    LOG_NOT_IMPLEMENTED(ShaderDecompiler, "fcmp");
    INST(0x3680000000000000, 0xfef0000000000000)
    LOG_NOT_IMPLEMENTED(ShaderDecompiler, "dsetp");
    INST(0x3670000000000000, 0xfef0000000000000)
    LOG_NOT_IMPLEMENTED(ShaderDecompiler, "dfma");
    INST(0x3660000000000000, 0xfef0000000000000)
    LOG_NOT_IMPLEMENTED(ShaderDecompiler, "isetp");
    INST(0x3650000000000000, 0xfef0000000000000)
    LOG_NOT_IMPLEMENTED(ShaderDecompiler, "iset");
    INST(0x3640000000000000, 0xfef0000000000000)
    LOG_NOT_IMPLEMENTED(ShaderDecompiler, "icmp");
    INST(0x3600000000000000, 0xfec0000000000000)
    LOG_NOT_IMPLEMENTED(ShaderDecompiler, "xmad");
    INST(0x3480000000000000, 0xfe80000000000000)
    LOG_NOT_IMPLEMENTED(ShaderDecompiler, "imadsp");
    INST(0x3400000000000000, 0xfe80000000000000)
    LOG_NOT_IMPLEMENTED(ShaderDecompiler, "imad");
    INST(0x3280000000000000, 0xfe80000000000000) {
        HANDLE_PRED_COND();

        const auto dst = GET_REG(0);
        const auto srcA = GET_REG(8);
        const auto negB = GET_BIT(48);
        const auto srcB = GET_VALUE_F32();
        const auto negC = GET_BIT(49);
        const auto srcC = GET_REG(39);
        LOG_DEBUG(ShaderDecompiler, "ffma r{} r{} {}0x{:08x} {}r{}", dst, srcA,
                  (negB ? "-" : ""), srcB, (negC ? "-" : ""), srcC);

        auto res =
            o->OpFloatFma(o->OpRegister(true, srcA, DataType::F32),
                          o->OpImmediateL(srcB, DataType::F32, negB),
                          o->OpRegister(true, srcC, DataType::F32, negC));
        o->OpMove(o->OpRegister(false, dst, DataType::F32), res);
    }
    INST(0x3200000000000000, 0xfe80000000000000)
    LOG_NOT_IMPLEMENTED(ShaderDecompiler, "dset");
    INST(0x3000000000000000, 0xfe00000000000000)
    LOG_NOT_IMPLEMENTED(ShaderDecompiler, "fset");
    INST(0x2c00000000000000, 0xfe00000000000000)
    LOG_NOT_IMPLEMENTED(ShaderDecompiler, "hadd2_32i");
    INST(0x2a00000000000000, 0xfe00000000000000)
    LOG_NOT_IMPLEMENTED(ShaderDecompiler, "hmul2_32i");
    INST(0x2800000000000000, 0xfe00000000000000)
    LOG_NOT_IMPLEMENTED(ShaderDecompiler, "hfma2_32i");
    INST(0x2000000000000000, 0xfc00000000000000)
    LOG_NOT_IMPLEMENTED(ShaderDecompiler, "vadd");
    INST(0x1f00000000000000, 0xff00000000000000)
    LOG_NOT_IMPLEMENTED(ShaderDecompiler, "imul32i");
    INST(0x1e00000000000000, 0xff00000000000000) {
        HANDLE_PRED_COND();

        const auto dst = GET_REG(0);
        const auto srcA = GET_REG(8);
        const auto srcB = GET_VALUE_U32_EXTEND(20, 32);
        LOG_DEBUG(ShaderDecompiler, "fmul32i r{} r{} 0x{:08x}", dst, srcA,
                  srcB);

        auto srcB_v = o->OpImmediateL(srcB, DataType::F32);
        auto res =
            o->OpMultiply(o->OpRegister(false, srcA, DataType::F32), srcB_v);
        o->OpMove(o->OpRegister(false, dst, DataType::F32), res);
    }
    INST(0x1d80000000000000, 0xff80000000000000)
    LOG_NOT_IMPLEMENTED(ShaderDecompiler, "iadd32i");
    INST(0x1c00000000000000, 0xfe80000000000000)
    LOG_NOT_IMPLEMENTED(ShaderDecompiler, "iadd32i");
    INST(0x1800000000000000, 0xfc00000000000000)
    LOG_NOT_IMPLEMENTED(ShaderDecompiler, "lea");
    INST(0x1400000000000000, 0xfc00000000000000)
    LOG_NOT_IMPLEMENTED(ShaderDecompiler, "iscadd32i");
    INST(0x1000000000000000, 0xfc00000000000000)
    LOG_NOT_IMPLEMENTED(ShaderDecompiler, "imad32i");
    INST(0x0c00000000000000, 0xfc00000000000000)
    LOG_NOT_IMPLEMENTED(ShaderDecompiler, "ffma32i");
    INST(0x0800000000000000, 0xfc00000000000000) {
        HANDLE_PRED_COND();

        const auto dst = GET_REG(0);
        const auto srcA = GET_REG(8);
        const auto srcB = GET_VALUE_U32_EXTEND(20, 32);
        LOG_DEBUG(ShaderDecompiler, "fadd32i r{} r{} 0x{:08x}", dst, srcA,
                  srcB);

        auto srcB_v = o->OpImmediateL(srcB, DataType::F32);
        auto res = o->OpAdd(o->OpRegister(false, srcA, DataType::F32), srcB_v);
        o->OpMove(o->OpRegister(false, dst, DataType::F32), res);
    }
    INST(0x0400000000000000, 0xfc00000000000000)
    LOG_NOT_IMPLEMENTED(ShaderDecompiler, "lop32i");
    INST(0x0200000000000000, 0xfe00000000000000)
    LOG_NOT_IMPLEMENTED(ShaderDecompiler, "lop3");
    INST(0x0100000000000000, 0xfff0000000000000) {
        HANDLE_PRED_COND();

        const auto dst = GET_REG(0);
        const auto value = GET_VALUE_U32_EXTEND(32, 20);
        const auto todo =
            extract_bits<u32, 4, 12>(inst) >> 8; // TODO: what is this?
        LOG_DEBUG(ShaderDecompiler, "mov32i {} 0x{:08x} 0x{:08x}", dst, value,
                  todo);

        o->OpMove(o->OpRegister(false, dst), o->OpImmediateL(value));
    }
    else {
        LOG_WARN(ShaderDecompiler, "Unknown instruction 0x{:016x}", inst);
        return {ResultCode::Error};
    }

    return {ResultCode::None};
}

} // namespace hydra::hw::tegra_x1::gpu::renderer::shader_decomp
