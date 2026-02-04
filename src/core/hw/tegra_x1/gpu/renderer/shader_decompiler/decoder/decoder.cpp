#include "core/hw/tegra_x1/gpu/renderer/shader_decompiler/decoder/decoder.hpp"

#include "core/hw/tegra_x1/gpu/renderer/shader_decompiler/decoder/bitfield.hpp"
#include "core/hw/tegra_x1/gpu/renderer/shader_decompiler/decoder/const.hpp"
#include "core/hw/tegra_x1/gpu/renderer/shader_decompiler/decoder/conversion.hpp"
#include "core/hw/tegra_x1/gpu/renderer/shader_decompiler/decoder/exit.hpp"
#include "core/hw/tegra_x1/gpu/renderer/shader_decompiler/decoder/float_arithmetic.hpp"
#include "core/hw/tegra_x1/gpu/renderer/shader_decompiler/decoder/float_comparison.hpp"
#include "core/hw/tegra_x1/gpu/renderer/shader_decompiler/decoder/float_min_max.hpp"
#include "core/hw/tegra_x1/gpu/renderer/shader_decompiler/decoder/half_arithmetic.hpp"
#include "core/hw/tegra_x1/gpu/renderer/shader_decompiler/decoder/integer_arithmetic.hpp"
#include "core/hw/tegra_x1/gpu/renderer/shader_decompiler/decoder/integer_comparison.hpp"
#include "core/hw/tegra_x1/gpu/renderer/shader_decompiler/decoder/integer_logical.hpp"
#include "core/hw/tegra_x1/gpu/renderer/shader_decompiler/decoder/memory.hpp"
#include "core/hw/tegra_x1/gpu/renderer/shader_decompiler/decoder/move.hpp"
#include "core/hw/tegra_x1/gpu/renderer/shader_decompiler/decoder/multifunction.hpp"
#include "core/hw/tegra_x1/gpu/renderer/shader_decompiler/decoder/predicate_comparison.hpp"
#include "core/hw/tegra_x1/gpu/renderer/shader_decompiler/decoder/shift.hpp"
#include "core/hw/tegra_x1/gpu/renderer/shader_decompiler/decoder/texture.hpp"
#include "core/hw/tegra_x1/gpu/renderer/shader_decompiler/decoder/warp.hpp"

namespace hydra::hw::tegra_x1::gpu::renderer::shader_decomp::decoder {

void Decoder::Decode() {
    crnt_block = &blocks[0x0];
    while (crnt_block) {
        ParseNextInstruction();
    }
}

void Decoder::ParseNextInstruction() {
    const u32 pc = GetPC();
    const auto inst = context.code_stream->Read<instruction_t>();
    if ((pc % 4) == 0) // Sched
        return;

    LOG_DEBUG(ShaderDecompiler, "Instruction 0x{:016x}", inst);

#define INST0(value, mask) if ((inst & mask##ull) == value##ull)
#define INST(value, mask) else INST0(value, mask)

#define EMIT(op) Emit##op(context, std::bit_cast<Inst##op>(inst))

    INST0(0xfbe0000000000000, 0xfff8000000000000) {
        COMMENT_NOT_IMPLEMENTED("out");
    }
    INST(0xf6e0000000000000, 0xfef8000000000000) {
        COMMENT_NOT_IMPLEMENTED("out");
    }
    INST(0xf0f8000000000000, 0xfff8000000000000) { // sync
        // TODO: ccc
        const auto pred = static_cast<pred_t>(extract_bits(inst, 16, 3));
        const auto pred_inv = static_cast<bool>(extract_bits(inst, 19, 1));

        ASSERT_DEBUG(!crnt_block->sync_point_stack.empty(), ShaderDecompiler,
                     "No sync point in stack");

        const auto target = crnt_block->sync_point_stack.top();
        if (pred == PT) {
            if (!pred_inv) {
                // Pop and then inherit
                crnt_block->sync_point_stack.pop();
                InheritSyncPoints(target);

                context.builder.OpBranch(target);
                EndBlock();
            } else {
                // TODO: how does sync behave with never?
                LOG_FATAL(ShaderDecompiler, "Never sync");
            }
        } else { // Conditional
            // Inherit for the continuation block, then pop and inherit for sync
            // block
            InheritSyncPoints(pc + 1);
            // TODO: should this pop? (probably not)
            crnt_block->sync_point_stack.pop();
            InheritSyncPoints(target);

            context.builder.OpBranchConditional(
                NotIf(context.builder, ir::Value::Predicate(pred), pred_inv),
                target, pc + 1);
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
    INST(0xeff0000000000000, 0xfff8000000000000) { EMIT(Sta); }
    INST(0xefe8000000000000, 0xfff8000000000000) {
        COMMENT_NOT_IMPLEMENTED("pixld");
    }
    INST(0xefd8000000000000, 0xfff8000000000000) { EMIT(Lda); }
    INST(0xefd0000000000000, 0xfff8000000000000) {
        COMMENT_NOT_IMPLEMENTED("isberd");
    }
    INST(0xefa0000000000000, 0xfff8000000000000) {
        COMMENT_NOT_IMPLEMENTED("al2p");
    }
    INST(0xef98000000000000, 0xfff8000000000000) {
        COMMENT_NOT_IMPLEMENTED("membar");
    }
    INST(0xef90000000000000, 0xfff8000000000000) { EMIT(Ldc); }
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
    INST(0xef10000000000000, 0xfff8000000000000) { EMIT(Shfl); }
    INST(0xeef0000000000000, 0xfff0000000000000) {
        COMMENT_NOT_IMPLEMENTED("atom");
    }
    INST(0xeed8000000000000, 0xfff8000000000000) {
        COMMENT_NOT_IMPLEMENTED("stg");
    }
    INST(0xeed0000000000000, 0xfff8000000000000) { EMIT(Ldg); }
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
    INST(0xe330000000000000, 0xfff0000000000000) { EMIT(Kil); }
    INST(0xe320000000000000, 0xfff0000000000000) {
        COMMENT_NOT_IMPLEMENTED("ret");
    }
    INST(0xe310000000000000, 0xfff0000000000000) {
        COMMENT_NOT_IMPLEMENTED("longjmp");
    }
    INST(0xe300000000000000, 0xfff0000000000000) { // exit
        // TODO: ccc, keep_ref_count
        const auto pred = static_cast<pred_t>(extract_bits(inst, 16, 3));
        const auto pred_inv = static_cast<bool>(extract_bits(inst, 19, 1));

        if (pred == PT) {
            if (!pred_inv) {
                context.builder.OpExit();
                EndBlock();
            }
        } else {
            context.builder.OpBeginIf(
                NotIf(context.builder, ir::Value::Predicate(pred), pred_inv));
            context.builder.OpExit();
            context.builder.OpEndIf();
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
    INST(0xe290000000000000, 0xfff0000000000020) { // ssy
        const auto target = static_cast<u32>(
            static_cast<i32>(pc) +
            sign_extend<i32, 24>(static_cast<i32>(extract_bits(inst, 20, 24))) /
                static_cast<i32>(sizeof(instruction_t)) +
            1);

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
    INST(0xe240000000000000, 0xfff0000000000020) { // bra
        // TODO: ccc, ca, lmt, u
        const auto pred = static_cast<pred_t>(extract_bits(inst, 16, 3));
        const auto pred_inv = static_cast<bool>(extract_bits(inst, 19, 1));
        const auto target = static_cast<u32>(
            static_cast<i32>(pc) +
            sign_extend<i32, 24>(static_cast<i32>(extract_bits(inst, 20, 24))) /
                static_cast<i32>(sizeof(instruction_t)) +
            1);

        if (pred == PT) {
            if (!pred_inv) {
                InheritSyncPoints(target);
                context.builder.OpBranch(target);
                EndBlock();
            }
        } else {
            InheritSyncPoints(target);
            InheritSyncPoints(pc + 1);
            context.builder.OpBranchConditional(
                NotIf(context.builder, ir::Value::Predicate(pred), pred_inv),
                target, pc + 1);
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
    INST(0xe000000000000000, 0xff00000000000000) { EMIT(Ipa); }
    INST(0xdf60000000000000, 0xfff8000000000000) {
        COMMENT_NOT_IMPLEMENTED("tmml");
    }
    INST(0xdf58000000000000, 0xfff8000000000000) {
        COMMENT_NOT_IMPLEMENTED("tmml");
    }
    INST(0xdf50000000000000, 0xfff8000000000000) { EMIT(Txq); }
    INST(0xdf48000000000000, 0xfff8000000000000) {
        COMMENT_NOT_IMPLEMENTED("txq");
    }
    INST(0xdf40000000000000, 0xfff8000000000000) {
        COMMENT_NOT_IMPLEMENTED("txa");
    }
    INST(0xdf00000000000000, 0xff40000000000000) { EMIT(Tld4s); }
    INST(0xdef8000000000000, 0xfff8000000000000) {
        COMMENT_NOT_IMPLEMENTED("tld4");
    }
    INST(0xdeb8000000000000, 0xfff8000000000000) {
        COMMENT_NOT_IMPLEMENTED("tex");
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
    INST(0xd200000000000000, 0xf600000000000000) { EMIT(Tlds); }
    INST(0xd000000000000000, 0xf600000000000000) { EMIT(Texs); }
    INST(0xc838000000000000, 0xfc38000000000000) { EMIT(Tld4); }
    INST(0xc038000000000000, 0xfc38000000000000) { EMIT(Tex); }
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
    INST(0x7a00000000000000, 0xfe80000000000000) { EMIT(Hadd2I); }
    INST(0x7a80000000000000, 0xfe80000000000000) { EMIT(Hadd2C); }
    INST(0x7800000000000000, 0xfe80000000000000) { EMIT(Hmul2I); }
    INST(0x7880000000000000, 0xfe80000000000000) { EMIT(Hmul2C); }
    INST(0x7080000000000000, 0xf880000000000000) { EMIT(Hfma2C); }
    INST(0x7000000000000000, 0xf880000000000000) { EMIT(Hfma2I); }
    INST(0x6080000000000000, 0xf880000000000000) { EMIT(Hfma2RC); }
    INST(0x5f00000000000000, 0xff00000000000000) {
        COMMENT_NOT_IMPLEMENTED("vmad");
    }
    INST(0x5d20000000000000, 0xfff8000000000000) {
        COMMENT_NOT_IMPLEMENTED("hsetp2");
    }
    INST(0x5d18000000000000, 0xfff8000000000000) {
        COMMENT_NOT_IMPLEMENTED("hset2");
    }
    INST(0x5d10000000000000, 0xfff8000000000000) { EMIT(Hadd2R); }
    INST(0x5d08000000000000, 0xfff8000000000000) { EMIT(Hmul2R); }
    INST(0x5d00000000000000, 0xfff8000000000000) { EMIT(Hfma2R); }
    INST(0x5cf8000000000000, 0xfff8000000000000) {
        COMMENT_NOT_IMPLEMENTED("shf");
    }
    INST(0x5cf0000000000000, 0xfff8000000000000) {
        COMMENT_NOT_IMPLEMENTED("r2p");
    }
    INST(0x5ce8000000000000, 0xfff8000000000000) {
        COMMENT_NOT_IMPLEMENTED("p2r");
    }
    INST(0x5ce0000000000000, 0xfff8000000000000) { EMIT(I2iR); }
    INST(0x5cc0000000000000, 0xfff0000000000000) {
        COMMENT_NOT_IMPLEMENTED("iadd3");
    }
    INST(0x5cb8000000000000, 0xfff8000000000000) { EMIT(I2fR); }
    INST(0x5cb0000000000000, 0xfff8000000000000) { EMIT(F2iR); }
    INST(0x5ca8000000000000, 0xfff8000000000000) { EMIT(F2fR); }
    INST(0x5ca0000000000000, 0xfff8000000000000) { EMIT(SelR); }
    INST(0x5c98000000000000, 0xfff8000000000000) { EMIT(MovR); }
    INST(0x5c90000000000000, 0xfff8000000000000) { EMIT(RroR); }
    INST(0x5c88000000000000, 0xfff8000000000000) {
        COMMENT_NOT_IMPLEMENTED("fchk");
    }
    INST(0x5c80000000000000, 0xfff8000000000000) {
        COMMENT_NOT_IMPLEMENTED("dmul");
    }
    INST(0x5c70000000000000, 0xfff8000000000000) {
        COMMENT_NOT_IMPLEMENTED("dadd");
    }
    INST(0x5c68000000000000, 0xfff8000000000000) { EMIT(FmulR); }
    INST(0x5c60000000000000, 0xfff8000000000000) { EMIT(FmnmxR); }
    INST(0x5c58000000000000, 0xfff8000000000000) { EMIT(FaddR); }
    INST(0x5c50000000000000, 0xfff8000000000000) {
        COMMENT_NOT_IMPLEMENTED("dmnmx");
    }
    INST(0x5c48000000000000, 0xfff8000000000000) { EMIT(ShlR); }
    INST(0x5c40000000000000, 0xfff8000000000000) { EMIT(LopR); }
    INST(0x5c38000000000000, 0xfff8000000000000) {
        COMMENT_NOT_IMPLEMENTED("imul");
    }
    INST(0x5c30000000000000, 0xfff8000000000000) {
        COMMENT_NOT_IMPLEMENTED("flo");
    }
    INST(0x5c28000000000000, 0xfff8000000000000) { EMIT(ShrR); }
    INST(0x5c20000000000000, 0xfff8000000000000) {
        COMMENT_NOT_IMPLEMENTED("imnmx");
    }
    INST(0x5c18000000000000, 0xfff8000000000000) { EMIT(IscaddR); }
    INST(0x5c10000000000000, 0xfff8000000000000) { EMIT(IaddR); }
    INST(0x5c08000000000000, 0xfff8000000000000) {
        COMMENT_NOT_IMPLEMENTED("popc");
    }
    INST(0x5c00000000000000, 0xfff8000000000000) { EMIT(BfeR); }
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
    INST(0x5bb0000000000000, 0xfff0000000000000) { EMIT(FsetpR); }
    INST(0x5ba0000000000000, 0xfff0000000000000) {
        COMMENT_NOT_IMPLEMENTED("fcmp");
    }
    INST(0x5b80000000000000, 0xfff0000000000000) {
        COMMENT_NOT_IMPLEMENTED("dsetp");
    }
    INST(0x5b70000000000000, 0xfff0000000000000) {
        COMMENT_NOT_IMPLEMENTED("dfma");
    }
    INST(0x5b60000000000000, 0xfff0000000000000) { EMIT(IsetpR); }
    INST(0x5b50000000000000, 0xfff0000000000000) { EMIT(IsetR); }
    INST(0x5b40000000000000, 0xfff0000000000000) {
        COMMENT_NOT_IMPLEMENTED("icmp");
    }
    INST(0x5b00000000000000, 0xffc0000000000000) { EMIT(XmadR); }
    INST(0x5a80000000000000, 0xff80000000000000) {
        COMMENT_NOT_IMPLEMENTED("imadsp");
    }
    INST(0x5a00000000000000, 0xff80000000000000) {
        COMMENT_NOT_IMPLEMENTED("imad");
    }
    INST(0x5980000000000000, 0xff80000000000000) { EMIT(FfmaR); }
    INST(0x5900000000000000, 0xff80000000000000) {
        COMMENT_NOT_IMPLEMENTED("dset");
    }
    INST(0x5800000000000000, 0xff00000000000000) { EMIT(FsetR); }
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
    INST(0x5180000000000000, 0xff80000000000000) { EMIT(FfmaRC); }
    INST(0x5100000000000000, 0xff80000000000000) { EMIT(XmadRC); }
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
    INST(0x5090000000000000, 0xfff8000000000000) { EMIT(Psetp); }
    INST(0x5088000000000000, 0xfff8000000000000) { EMIT(Pset); }
    INST(0x5080000000000000, 0xfff8000000000000) { EMIT(Mufu); }
    INST(0x5000000000000000, 0xff80000000000000) {
        COMMENT_NOT_IMPLEMENTED("vabsdiff4");
    }
    INST(0x4e00000000000000, 0xfe00000000000000) { EMIT(XmadC); }
    INST(0x4cf0000000000000, 0xfff8000000000000) {
        COMMENT_NOT_IMPLEMENTED("r2p");
    }
    INST(0x4ce8000000000000, 0xfff8000000000000) {
        COMMENT_NOT_IMPLEMENTED("p2r");
    }
    INST(0x4ce0000000000000, 0xfff8000000000000) { EMIT(I2iC); }
    INST(0x4cc0000000000000, 0xfff0000000000000) {
        COMMENT_NOT_IMPLEMENTED("iadd3");
    }
    INST(0x4cb8000000000000, 0xfff8000000000000) { EMIT(I2fC); }
    INST(0x4cb0000000000000, 0xfff8000000000000) { EMIT(F2iC); }
    INST(0x4ca8000000000000, 0xfff8000000000000) { EMIT(F2fC); }
    INST(0x4ca0000000000000, 0xfff8000000000000) { EMIT(SelC); }
    INST(0x4c98000000000000, 0xfff8000000000000) { EMIT(MovC); }
    INST(0x4c90000000000000, 0xfff8000000000000) { EMIT(RroC); }
    INST(0x4c88000000000000, 0xfff8000000000000) {
        COMMENT_NOT_IMPLEMENTED("fchk");
    }
    INST(0x4c80000000000000, 0xfff8000000000000) {
        COMMENT_NOT_IMPLEMENTED("dmul");
    }
    INST(0x4c70000000000000, 0xfff8000000000000) {
        COMMENT_NOT_IMPLEMENTED("dadd");
    }
    INST(0x4c68000000000000, 0xfff8000000000000) { EMIT(FmulC); }
    INST(0x4c60000000000000, 0xfff8000000000000) { EMIT(FmnmxC); }
    INST(0x4c58000000000000, 0xfff8000000000000) { EMIT(FaddC); }
    INST(0x4c50000000000000, 0xfff8000000000000) {
        COMMENT_NOT_IMPLEMENTED("dmnmx");
    }
    INST(0x4c48000000000000, 0xfff8000000000000) { EMIT(ShlC); }
    INST(0x4c40000000000000, 0xfff8000000000000) { EMIT(LopC); }
    INST(0x4c38000000000000, 0xfff8000000000000) {
        COMMENT_NOT_IMPLEMENTED("imul");
    }
    INST(0x4c30000000000000, 0xfff8000000000000) {
        COMMENT_NOT_IMPLEMENTED("flo");
    }
    INST(0x4c28000000000000, 0xfff8000000000000) { EMIT(ShrC); }
    INST(0x4c20000000000000, 0xfff8000000000000) {
        COMMENT_NOT_IMPLEMENTED("imnmx");
    }
    INST(0x4c18000000000000, 0xfff8000000000000) { EMIT(IscaddC); }
    INST(0x4c10000000000000, 0xfff8000000000000) { EMIT(IaddC); }
    INST(0x4c08000000000000, 0xfff8000000000000) {
        COMMENT_NOT_IMPLEMENTED("popc");
    }
    INST(0x4c00000000000000, 0xfff8000000000000) { EMIT(BfeC); }
    INST(0x4bf0000000000000, 0xfff8000000000000) {
        COMMENT_NOT_IMPLEMENTED("bfi");
    }
    INST(0x4bd0000000000000, 0xfff8000000000000) {
        COMMENT_NOT_IMPLEMENTED("lea");
    }
    INST(0x4bc0000000000000, 0xfff0000000000000) {
        COMMENT_NOT_IMPLEMENTED("prmt");
    }
    INST(0x4bb0000000000000, 0xfff0000000000000) { EMIT(FsetpC); }
    INST(0x4ba0000000000000, 0xfff0000000000000) {
        COMMENT_NOT_IMPLEMENTED("fcmp");
    }
    INST(0x4b80000000000000, 0xfff0000000000000) {
        COMMENT_NOT_IMPLEMENTED("dsetp");
    }
    INST(0x4b70000000000000, 0xfff0000000000000) {
        COMMENT_NOT_IMPLEMENTED("dfma");
    }
    INST(0x4b60000000000000, 0xfff0000000000000) { EMIT(IsetpC); }
    INST(0x4b50000000000000, 0xfff0000000000000) { EMIT(IsetC); }
    INST(0x4b40000000000000, 0xfff0000000000000) {
        COMMENT_NOT_IMPLEMENTED("icmp");
    }
    INST(0x4a80000000000000, 0xff80000000000000) {
        COMMENT_NOT_IMPLEMENTED("imadsp");
    }
    INST(0x4a00000000000000, 0xff80000000000000) {
        COMMENT_NOT_IMPLEMENTED("imad");
    }
    INST(0x4980000000000000, 0xff80000000000000) { EMIT(FfmaC); }
    INST(0x4900000000000000, 0xff80000000000000) {
        COMMENT_NOT_IMPLEMENTED("dset");
    }
    INST(0x4800000000000000, 0xfe00000000000000) { EMIT(FsetC); }
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
    INST(0x38e0000000000000, 0xfef8000000000000) { EMIT(I2iI); }
    INST(0x38c0000000000000, 0xfef0000000000000) {
        COMMENT_NOT_IMPLEMENTED("iadd3");
    }
    INST(0x38b8000000000000, 0xfef8000000000000) { EMIT(I2fI); }
    INST(0x38b0000000000000, 0xfef8000000000000) { EMIT(F2iI); }
    INST(0x38a8000000000000, 0xfef8000000000000) { EMIT(F2fI); }
    INST(0x38a0000000000000, 0xfef8000000000000) { EMIT(SelI); }
    INST(0x3898000000000000, 0xfef8000000000000) { EMIT(MovI); }
    INST(0x3890000000000000, 0xfef8000000000000) { EMIT(RroI); }
    INST(0x3888000000000000, 0xfef8000000000000) {
        COMMENT_NOT_IMPLEMENTED("fchk");
    }
    INST(0x3880000000000000, 0xfef8000000000000) {
        COMMENT_NOT_IMPLEMENTED("dmul");
    }
    INST(0x3870000000000000, 0xfef8000000000000) {
        COMMENT_NOT_IMPLEMENTED("dadd");
    }
    INST(0x3868000000000000, 0xfef8000000000000) { EMIT(FmulI); }
    INST(0x3860000000000000, 0xfef8000000000000) { EMIT(FmnmxI); }
    INST(0x3858000000000000, 0xfef8000000000000) { EMIT(FaddI); }
    INST(0x3850000000000000, 0xfef8000000000000) {
        COMMENT_NOT_IMPLEMENTED("dmnmx");
    }
    INST(0x3848000000000000, 0xfef8000000000000) { EMIT(ShlI); }
    INST(0x3840000000000000, 0xfef8000000000000) { EMIT(LopI); }
    INST(0x3838000000000000, 0xfef8000000000000) {
        COMMENT_NOT_IMPLEMENTED("imul");
    }
    INST(0x3830000000000000, 0xfef8000000000000) {
        COMMENT_NOT_IMPLEMENTED("flo");
    }
    INST(0x3828000000000000, 0xfef8000000000000) { EMIT(ShrI); }
    INST(0x3820000000000000, 0xfef8000000000000) {
        COMMENT_NOT_IMPLEMENTED("imnmx");
    }
    INST(0x3818000000000000, 0xfef8000000000000) { EMIT(IscaddI); }
    INST(0x3810000000000000, 0xfef8000000000000) { EMIT(IaddI); }
    INST(0x3808000000000000, 0xfef8000000000000) {
        COMMENT_NOT_IMPLEMENTED("popc");
    }
    INST(0x3800000000000000, 0xfef8000000000000) { EMIT(BfeI); }
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
    INST(0x36b0000000000000, 0xfef0000000000000) { EMIT(FsetpI); }
    INST(0x36a0000000000000, 0xfef0000000000000) {
        COMMENT_NOT_IMPLEMENTED("fcmp");
    }
    INST(0x3680000000000000, 0xfef0000000000000) {
        COMMENT_NOT_IMPLEMENTED("dsetp");
    }
    INST(0x3670000000000000, 0xfef0000000000000) {
        COMMENT_NOT_IMPLEMENTED("dfma");
    }
    INST(0x3660000000000000, 0xfef0000000000000) { EMIT(IsetpI); }
    INST(0x3650000000000000, 0xfef0000000000000) { EMIT(IsetI); }
    INST(0x3640000000000000, 0xfef0000000000000) {
        COMMENT_NOT_IMPLEMENTED("icmp");
    }
    INST(0x3600000000000000, 0xfec0000000000000) { EMIT(XmadI); }
    INST(0x3480000000000000, 0xfe80000000000000) {
        COMMENT_NOT_IMPLEMENTED("imadsp");
    }
    INST(0x3400000000000000, 0xfe80000000000000) {
        COMMENT_NOT_IMPLEMENTED("imad");
    }
    INST(0x3280000000000000, 0xfe80000000000000) { EMIT(FfmaI); }
    INST(0x3200000000000000, 0xfe80000000000000) {
        COMMENT_NOT_IMPLEMENTED("dset");
    }
    INST(0x3000000000000000, 0xfe00000000000000) { EMIT(FsetI); }
    INST(0x2c00000000000000, 0xfe00000000000000) { EMIT(Hadd2_32I); }
    INST(0x2a00000000000000, 0xfe00000000000000) { EMIT(Hmul2_32I); }
    INST(0x2800000000000000, 0xfe00000000000000) { EMIT(Hfma2_32I); }
    INST(0x2000000000000000, 0xfc00000000000000) {
        COMMENT_NOT_IMPLEMENTED("vadd");
    }
    INST(0x1f00000000000000, 0xff00000000000000) {
        COMMENT_NOT_IMPLEMENTED("imul32i");
    }
    INST(0x1e00000000000000, 0xff00000000000000) { EMIT(Fmul32I); }
    // TODO: is this necessary?
    // INST(0x1d80000000000000, 0xff80000000000000) {
    //    COMMENT_NOT_IMPLEMENTED("iadd32i");
    //}
    INST(0x1c00000000000000, 0xfe80000000000000) { EMIT(Iadd32I); }
    INST(0x1800000000000000, 0xfc00000000000000) {
        COMMENT_NOT_IMPLEMENTED("lea");
    }
    INST(0x1400000000000000, 0xfc00000000000000) { EMIT(Iscadd32I); }
    INST(0x1000000000000000, 0xfc00000000000000) {
        COMMENT_NOT_IMPLEMENTED("imad32i");
    }
    INST(0x0c00000000000000, 0xfc00000000000000) { EMIT(Ffma32I); }
    INST(0x0800000000000000, 0xfc00000000000000) { EMIT(Fadd32I); }
    INST(0x0400000000000000, 0xfc00000000000000) { EMIT(Lop32I); }
    INST(0x0200000000000000, 0xfe00000000000000) {
        COMMENT_NOT_IMPLEMENTED("lop3");
    }
    INST(0x0100000000000000, 0xfff0000000000000) { EMIT(Mov32I); }
    else {
        LOG_ERROR(ShaderDecompiler, "Unknown instruction 0x{:016x}", inst);
    }
} // namespace hydra::hw::tegra_x1::gpu::renderer::shader_decomp::decoder

} // namespace hydra::hw::tegra_x1::gpu::renderer::shader_decomp::decoder
