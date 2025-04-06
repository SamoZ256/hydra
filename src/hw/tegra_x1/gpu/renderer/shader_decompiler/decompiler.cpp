#include "hw/tegra_x1/gpu/renderer/shader_decompiler/decompiler.hpp"

#include "hw/tegra_x1/gpu/renderer/shader_decompiler/analyzer.hpp"
#include "hw/tegra_x1/gpu/renderer/shader_decompiler/lang/msl/builder.hpp"
#include "hw/tegra_x1/gpu/renderer/shader_decompiler/tables.hpp"

namespace Hydra::HW::TegraX1::GPU::Renderer::ShaderDecompiler {

struct ShaderHeader {
    // CommonWord0
    u32 sph_type : 5;
    u32 version : 5;
    u32 shader_type : 4;
    u32 mrt_enable : 1;
    u32 kills_pixels : 1;
    u32 does_global_store : 1;
    u32 sass_version : 4;
    u32 _reserved0_1 : 1;
    u32 _reserved0_2 : 1;
    u32 _reserved0_3 : 1;
    u32 is_fast_gs : 1;
    u32 vsh_unk_flag : 1;
    u32 does_load_or_store : 1;
    u32 does_fp64 : 1;
    u32 stream_out_mask : 4;

    // CommonWord1
    u32 sh_local_mem_lo_sz : 24;
    u32 per_patch_attrib_cnt : 8;

    // CommonWord2
    u32 sh_local_mem_hi_sz : 24;
    u32 thr_per_input_prim : 8;

    // CommonWord3
    u32 sh_local_mem_crs_sz : 24;
    u32 output_topology : 4;
    u32 _reserved3 : 4;

    // CommonWord4
    u32 max_out_vtx_cnt : 12;
    u32 store_req_start : 8;
    u32 _reserved4 : 4;
    u32 store_req_end : 8;

    // ImapSystemValuesA/ImapSystemValuesB
    u32 imap_sysvals_ab;

    union {
        struct {
            u8 imap_generic_vector[16];
            u16 imap_color;
            u16 imap_sysvals_c;
            u8 imap_fixed_fnc_tex[5];
            u8 _reserved0;
            u32 omap_sysvals_ab;
            u8 omap_generic_vector[16];
            u16 omap_color;
            u16 omap_sysvals_c;
            u8 omap_fixed_fnc_tex[5];
            u8 omap_extra;
        } __attribute__((packed)) vtg;

        struct {
            u8 imap_generic_vector[32];
            u16 imap_color;
            u16 imap_sysvals_c;
            u8 imap_fixed_fnc_tex[10];
            u16 _reserved0;
            u32 omap_target;
            u32 omap_sample_mask : 1;
            u32 omap_depth : 1;
            u32 _reserved1 : 30;
        } ps;
    };
};

void Decompiler::Decompile(Reader& code_reader, const ShaderType type,
                           const GuestShaderState& state,
                           std::vector<u8>& out_code,
                           ResourceMapping& out_resource_mapping) {
    Analyzer analyzer;

    // Builder
    BuilderBase* builder;
    // TODO: choose based on the Shader Decompiler backend
    {
        builder = new Lang::MSL::Builder(analyzer, type, state, out_code,
                                         out_resource_mapping);
    }

    // Header
    // TODO: don't read in case of compute shaders
    const ShaderHeader header = code_reader.Read<ShaderHeader>();
    // HACK: just for testing
    ASSERT_DEBUG(header.version == 3, ShaderDecompiler,
                 "Invalid shader version {}", header.version);

    u32 code_offset = code_reader.Tell();

    // Analyze
    Parse(&analyzer, code_reader);

    // Decompile
    builder->Start();
    code_reader.Seek(code_offset);
    Parse(builder, code_reader);
    builder->Finish();
    delete builder;
}

bool Decompiler::ParseInstruction(ObserverBase* observer, u64 inst) {
#define GET_REG(b) extract_bits<reg_t, b, 8>(inst)
#define GET_VALUE_U(type_bit_count, b, count)                                  \
    extract_bits<u##type_bit_count, b, count>(inst)
#define GET_VALUE_U_EXTEND(type_bit_count, b, count)                           \
    (GET_VALUE_U(type_bit_count, b, count) << (type_bit_count - count))
#define GET_VALUE_U32(b, count) GET_VALUE_U(32, b, count)
#define GET_VALUE_U32_EXTEND(b, count) GET_VALUE_U_EXTEND(32, b, count)
#define GET_VALUE_U64(b, count) GET_VALUE_U(64, b, count)
#define GET_VALUE_U64_EXTEND(b, count) GET_VALUE_U_EXTEND(64, b, count)
#define GET_AMEM() AMem{GET_REG(8), 0}
#define GET_AMEM_IDX(b)                                                        \
    AMem { GET_REG(8), extract_bits<u32, b, 10>(inst) }
// HACK: why do I have to multiply by 4?
#define GET_CMEM(b_idx, count_imm)                                             \
    CMem{GET_VALUE_U32(b_idx, 5), RZ,                                          \
         extract_bits<u32, 20, count_imm>(inst) * 4}
#define GET_CMEM_R(b_idx, b_reg, count_imm)                                    \
    CMem{GET_VALUE_U32(b_idx, 5), GET_REG(b_reg),                              \
         extract_bits<u32, 20, count_imm>(inst) * 4}

    INST0(0xfbe0000000000000, 0xfff8000000000000)
    LOG_NOT_IMPLEMENTED(ShaderDecompiler, "out");
    INST(0xf6e0000000000000, 0xfef8000000000000)
    LOG_NOT_IMPLEMENTED(ShaderDecompiler, "out");
    INST(0xf0f8000000000000, 0xfff8000000000000)
    LOG_NOT_IMPLEMENTED(ShaderDecompiler, "sync");
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
        const auto mode = get_operand_eff0_0(inst);
        auto amem = GET_AMEM_IDX(20);
        const auto src = GET_REG(0);
        const auto todo = GET_REG(39); // TODO: what is this?
        LOG_DEBUG(ShaderDecompiler, "st {} a[r{} + 0x{:08x}] r{} r{}", mode,
                  amem.reg, amem.imm, src, todo);

        for (u32 i = 0; i < get_load_store_count(mode); i++) {
            observer->OpStore(amem, src + i);
            amem.imm += sizeof(u32);
        }
    }
    INST(0xefe8000000000000, 0xfff8000000000000)
    LOG_NOT_IMPLEMENTED(ShaderDecompiler, "pixld");
    INST(0xefd8000000000000, 0xfff8000000000000) {
        const auto mode = get_operand_eff0_0(inst);
        const auto dst = GET_REG(0);
        auto amem = GET_AMEM_IDX(20);
        const auto todo = GET_REG(39); // TODO: what is this?
        LOG_DEBUG(ShaderDecompiler, "ld {} r{} a[r{} + 0x{:08x}] r{}", mode,
                  dst, amem.reg, amem.imm, todo);

        for (u32 i = 0; i < get_load_store_count(mode); i++) {
            observer->OpLoad(dst + i, Operand::AttributeMemory(amem));
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
        const auto dst = GET_REG(0);
        const auto src = GET_CMEM_R(36, 8, 16);
        LOG_DEBUG(ShaderDecompiler, "ld r{} c{}[r{} + 0x{:x}]", dst, src.idx,
                  src.reg, src.imm);

        observer->OpLoad(dst, Operand::ConstMemory(src));
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
    INST(0xe330000000000000, 0xfff0000000000000)
    LOG_NOT_IMPLEMENTED(ShaderDecompiler, "kil");
    INST(0xe320000000000000, 0xfff0000000000000)
    LOG_NOT_IMPLEMENTED(ShaderDecompiler, "ret");
    INST(0xe310000000000000, 0xfff0000000000000)
    LOG_NOT_IMPLEMENTED(ShaderDecompiler, "longjmp");
    INST(0xe300000000000000, 0xfff0000000000000) {
        // TODO: f0f8_0
        LOG_DEBUG(ShaderDecompiler, "exit");

        observer->OpExit();
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
    INST(0xe290000000000000, 0xfff0000000000020)
    LOG_NOT_IMPLEMENTED(ShaderDecompiler, "ssy");
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
    INST(0xe240000000000000, 0xfff0000000000020)
    LOG_NOT_IMPLEMENTED(ShaderDecompiler, "bra");
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
        // TODO: mode
        const auto dst = GET_REG(0);
        const auto amem = GET_AMEM_IDX(28);
        const auto interp_param = GET_REG(20);
        const auto flag1 = GET_REG(39);
        // TODO: another flag
        LOG_DEBUG(ShaderDecompiler, "ipa r{} a[r{} + 0x{:08x}] r{} r{}", dst,
                  amem.reg, amem.imm, interp_param, flag1);

        observer->OpInterpolate(dst, amem);
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
    INST(0xdeb8000000000000, 0xfff8000000000000)
    LOG_NOT_IMPLEMENTED(ShaderDecompiler, "tex");
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
        const auto todo1 = GET_REG(28);
        const auto dst = GET_REG(0);
        const auto todo2 = GET_REG(8);
        const auto coords = GET_REG(20);
        const auto addr = GET_VALUE_U32(36, 13);
        // TODO: texture type
        // TODO: component swizzle?
        LOG_DEBUG(ShaderDecompiler, "texs r{} r{} r{} r{} 0x{:08x}", todo1, dst,
                  todo2, coords, addr);

        observer->OpTextureSample(
            dst, addr - 0x1a4, coords); // TODO: how do texture addresses work?
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
    INST(0x5ce0000000000000, 0xfff8000000000000)
    LOG_NOT_IMPLEMENTED(ShaderDecompiler, "i2i");
    INST(0x5cc0000000000000, 0xfff0000000000000)
    LOG_NOT_IMPLEMENTED(ShaderDecompiler, "iadd3");
    INST(0x5cb8000000000000, 0xfff8000000000000)
    LOG_NOT_IMPLEMENTED(ShaderDecompiler, "i2f");
    INST(0x5cb0000000000000, 0xfff8000000000000)
    LOG_NOT_IMPLEMENTED(ShaderDecompiler, "f2i");
    INST(0x5ca8000000000000, 0xfff8000000000000)
    LOG_NOT_IMPLEMENTED(ShaderDecompiler, "f2f");
    INST(0x5ca0000000000000, 0xfff8000000000000)
    LOG_NOT_IMPLEMENTED(ShaderDecompiler, "sel");
    INST(0x5c98000000000000, 0xfff8000000000000) {
        const auto dst = GET_REG(0);
        const auto src = GET_REG(20);
        const auto todo = GET_VALUE_U32(39, 4); // TODO: what is this?
        LOG_DEBUG(ShaderDecompiler, "mov r{} r{} 0x{:x}", dst, src, todo);

        observer->OpMove(dst, Operand::Register(src));
    }
    INST(0x5c90000000000000, 0xfff8000000000000)
    LOG_NOT_IMPLEMENTED(ShaderDecompiler, "rro");
    INST(0x5c88000000000000, 0xfff8000000000000)
    LOG_NOT_IMPLEMENTED(ShaderDecompiler, "fchk");
    INST(0x5c80000000000000, 0xfff8000000000000)
    LOG_NOT_IMPLEMENTED(ShaderDecompiler, "dmul");
    INST(0x5c70000000000000, 0xfff8000000000000)
    LOG_NOT_IMPLEMENTED(ShaderDecompiler, "dadd");
    INST(0x5c68000000000000, 0xfff8000000000000) {
        const auto dst = GET_REG(0);
        const auto src1 = GET_REG(8);
        const auto src2 = GET_REG(20);
        LOG_DEBUG(ShaderDecompiler, "fmul r{} r{} r{}", dst, src1, src2);

        observer->OpFloatMultiply(dst, src1, Operand::Register(src2));
    }
    INST(0x5c60000000000000, 0xfff8000000000000)
    LOG_NOT_IMPLEMENTED(ShaderDecompiler, "fmnmx");
    INST(0x5c58000000000000, 0xfff8000000000000) {
        const auto dst = GET_REG(0);
        const auto src1 = GET_REG(8);
        const auto src2 = GET_REG(20);
        LOG_DEBUG(ShaderDecompiler, "fadd r{} r{} r{}", dst, src1, src2);

        observer->OpFloatAdd(dst, src1, Operand::Register(src2));
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
    INST(0x5bb0000000000000, 0xfff0000000000000)
    LOG_NOT_IMPLEMENTED(ShaderDecompiler, "fsetp");
    INST(0x5ba0000000000000, 0xfff0000000000000)
    LOG_NOT_IMPLEMENTED(ShaderDecompiler, "fcmp");
    INST(0x5b80000000000000, 0xfff0000000000000)
    LOG_NOT_IMPLEMENTED(ShaderDecompiler, "dsetp");
    INST(0x5b70000000000000, 0xfff0000000000000)
    LOG_NOT_IMPLEMENTED(ShaderDecompiler, "dfma");
    INST(0x5b60000000000000, 0xfff0000000000000)
    LOG_NOT_IMPLEMENTED(ShaderDecompiler, "isetp");
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
        const auto dst = GET_REG(0);
        const auto src1 = GET_REG(8);
        const auto src2 = GET_REG(20);
        const auto src3 = GET_REG(39);
        LOG_DEBUG(ShaderDecompiler, "ffma r{} r{} r{} r{}", dst, src1, src2,
                  src3);

        observer->OpFloatFma(dst, src1, Operand::Register(src2), src3);
    }
    INST(0x5900000000000000, 0xff80000000000000)
    LOG_NOT_IMPLEMENTED(ShaderDecompiler, "dset");
    INST(0x5800000000000000, 0xff00000000000000)
    LOG_NOT_IMPLEMENTED(ShaderDecompiler, "fset");
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
    INST(0x5180000000000000, 0xff80000000000000)
    LOG_NOT_IMPLEMENTED(ShaderDecompiler, "ffma");
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
    INST(0x5090000000000000, 0xfff8000000000000)
    LOG_NOT_IMPLEMENTED(ShaderDecompiler, "psetp");
    INST(0x5088000000000000, 0xfff8000000000000)
    LOG_NOT_IMPLEMENTED(ShaderDecompiler, "pset");
    INST(0x5080000000000000, 0xfff8000000000000) {
        const auto func = get_operand_5080_0(inst);
        const auto dst = GET_REG(0);
        const auto src = GET_REG(8);
        LOG_DEBUG(ShaderDecompiler, "mufu {} r{} r{}", func, dst, src);

        observer->OpMathFunction(func, dst, src);
    }
    INST(0x5000000000000000, 0xff80000000000000)
    LOG_NOT_IMPLEMENTED(ShaderDecompiler, "vabsdiff4");
    INST(0x4e00000000000000, 0xfe00000000000000)
    LOG_NOT_IMPLEMENTED(ShaderDecompiler, "xmad");
    INST(0x4cf0000000000000, 0xfff8000000000000)
    LOG_NOT_IMPLEMENTED(ShaderDecompiler, "r2p");
    INST(0x4ce8000000000000, 0xfff8000000000000)
    LOG_NOT_IMPLEMENTED(ShaderDecompiler, "p2r");
    INST(0x4ce0000000000000, 0xfff8000000000000)
    LOG_NOT_IMPLEMENTED(ShaderDecompiler, "i2i");
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
        const auto dst = GET_REG(0);
        const auto src = GET_CMEM(34, 14);
        const auto todo = GET_VALUE_U32(39, 4);
        LOG_DEBUG(ShaderDecompiler, "mov r{} c{}[0x{:x}] 0x{:x}", dst, src.idx,
                  src.imm, todo);

        observer->OpMove(dst, Operand::ConstMemory(src));
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
        const auto dst = GET_REG(0);
        const auto src1 = GET_REG(8);
        const auto src2 = GET_CMEM(34, 14);
        LOG_DEBUG(ShaderDecompiler, "fmul r{} r{} c{}[0x{:x}]", dst, src1,
                  src2.idx, src2.imm);

        observer->OpFloatMultiply(dst, src1, Operand::ConstMemory(src2));
    }
    INST(0x4c60000000000000, 0xfff8000000000000)
    LOG_NOT_IMPLEMENTED(ShaderDecompiler, "fmnmx");
    INST(0x4c58000000000000, 0xfff8000000000000) {
        const auto dst = GET_REG(0);
        const auto src1 = GET_REG(8);
        const auto src2 = GET_CMEM(34, 14);
        LOG_DEBUG(ShaderDecompiler, "fadd r{} r{} c{}[0x{:x}]", dst, src1,
                  src2.idx, src2.imm);

        observer->OpFloatAdd(dst, src1, Operand::ConstMemory(src2));
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
    INST(0x4bb0000000000000, 0xfff0000000000000)
    LOG_NOT_IMPLEMENTED(ShaderDecompiler, "fsetp");
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
        const auto dst = GET_REG(0);
        const auto src1 = GET_REG(8);
        const auto src2 = GET_CMEM(34, 14);
        const auto src3 = GET_REG(39);
        LOG_DEBUG(ShaderDecompiler, "ffma r{} r{} c{}[0x{:x}] r{}", dst, src1,
                  src2.idx, src2.imm, src3);

        observer->OpFloatFma(dst, src1, Operand::ConstMemory(src2), src3);
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
    INST(0x38e0000000000000, 0xfef8000000000000)
    LOG_NOT_IMPLEMENTED(ShaderDecompiler, "i2i");
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
    INST(0x3868000000000000, 0xfef8000000000000)
    LOG_NOT_IMPLEMENTED(ShaderDecompiler, "fmul");
    INST(0x3860000000000000, 0xfef8000000000000)
    LOG_NOT_IMPLEMENTED(ShaderDecompiler, "fmnmx");
    INST(0x3858000000000000, 0xfef8000000000000) {
        const auto dst = GET_REG(0);
        const auto src1 = GET_REG(8);
        const auto src2 = GET_VALUE_U32_EXTEND(20, 20); // TODO: extend?
        LOG_DEBUG(ShaderDecompiler, "fadd r{} r{} 0x{:x}", dst, src1, src2);

        observer->OpFloatAdd(dst, src1, Operand::Immediate(src2));
    }
    INST(0x3850000000000000, 0xfef8000000000000)
    LOG_NOT_IMPLEMENTED(ShaderDecompiler, "dmnmx");
    INST(0x3848000000000000, 0xfef8000000000000) {
        const auto dst = GET_REG(0);
        const auto src = GET_REG(8);
        const auto shift = GET_VALUE_U32(20, 19) |
                           (GET_VALUE_U32(56, 1) << 19); // TODO: correct?
        LOG_DEBUG(ShaderDecompiler, "shl r{} r{} 0x{:x}", dst, src, shift);

        observer->OpShiftLeft(dst, src, shift);
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
    INST(0x36b0000000000000, 0xfef0000000000000)
    LOG_NOT_IMPLEMENTED(ShaderDecompiler, "fsetp");
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
    INST(0x3280000000000000, 0xfe80000000000000)
    LOG_NOT_IMPLEMENTED(ShaderDecompiler, "ffma");
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
    INST(0x1e00000000000000, 0xff00000000000000)
    LOG_NOT_IMPLEMENTED(ShaderDecompiler, "fmul32i");
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
    INST(0x0800000000000000, 0xfc00000000000000)
    LOG_NOT_IMPLEMENTED(ShaderDecompiler, "fadd32i");
    INST(0x0400000000000000, 0xfc00000000000000)
    LOG_NOT_IMPLEMENTED(ShaderDecompiler, "lop32i");
    INST(0x0200000000000000, 0xfe00000000000000)
    LOG_NOT_IMPLEMENTED(ShaderDecompiler, "lop3");
    INST(0x0100000000000000, 0xfff0000000000000) {
        const auto dst = GET_REG(0);
        const auto value = GET_VALUE_U32_EXTEND(32, 20);
        const auto todo =
            extract_bits<u32, 4, 12>(inst) >> 8; // TODO: what is this?
        LOG_DEBUG(ShaderDecompiler, "mov32i {} 0x{:08x} 0x{:08x}", dst, value,
                  todo);

        observer->OpMove(dst, Operand::Immediate(value));
    }
    else {
        LOG_WARNING(ShaderDecompiler, "Unknown instruction 0x{:016x}", inst);
        return false;
    }

    return true;
}

void Decompiler::Parse(ObserverBase* observer, Reader& code_reader) {
    u32 index = 0;
    while (true) {
        u64 inst = code_reader.Read<u64>();
        LOG_DEBUG(ShaderDecompiler, "Instruction 0x{:016x}", inst);

        if (index % 4 == 0) {
            LOG_NOT_IMPLEMENTED(ShaderDecompiler, "sched");
        } else {
            if (!ParseInstruction(observer, inst))
                break;
        }

        // TODO: when to end

        index++;
    }
}

} // namespace Hydra::HW::TegraX1::GPU::Renderer::ShaderDecompiler
