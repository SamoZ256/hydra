#include "core/hw/tegra_x1/gpu/renderer/shader_decompiler/decompiler.hpp"

#include "core/hw/tegra_x1/gpu/renderer/shader_decompiler/analyzer/memory_analyzer.hpp"
#include "core/hw/tegra_x1/gpu/renderer/shader_decompiler/codegen/lang/msl/emitter.hpp"
#include "core/hw/tegra_x1/gpu/renderer/shader_decompiler/decoder/decoder.hpp"
#include "core/hw/tegra_x1/gpu/renderer/shader_decompiler/ir/builder.hpp"

// HACK
#include "core/hw/tegra_x1/gpu/renderer/shader_decompiler/analyzer/cfg.hpp"

namespace hydra::hw::tegra_x1::gpu::renderer::shader_decomp {

#pragma pack(push, 1)
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
        } vtg;

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
#pragma pack(pop)

void Decompile(ztd::io::MemoryStream& code_stream, const ShaderType type,
               const GuestShaderState& state, ShaderBackend& out_backend,
               std::vector<u8>& out_code,
               ResourceMapping& out_resource_mapping) {
    // Header
    // TODO: don't read in case of compute shaders
    const auto header = code_stream.read<ShaderHeader>();
    // HACK: just for testing
    ASSERT_DEBUG(header.version == 3, ShaderDecompiler,
                 "Invalid shader version {}", header.version);

    // Context
    DecompilerContext context;
    context.type = type;
    if (type == ShaderType::Fragment) {
        for (u32 i = 0; i < PIXEL_IMAP_COUNT; i++) {
            const auto imap = header.ps.imap_generic_vector[i];
            context.frag.pixel_imaps[i] = {
                .x = static_cast<PixelImapType>(extract_bits(imap, 0, 2)),
                .y = static_cast<PixelImapType>(extract_bits(imap, 2, 2)),
                .z = static_cast<PixelImapType>(extract_bits(imap, 4, 2)),
                .w = static_cast<PixelImapType>(extract_bits(imap, 6, 2)),
            };
        }
    }

    // Build IR
    ir::Module modul;
    {
        ir::Builder builder(modul);
        ztd::io::StreamView stream(&code_stream, code_stream.getSeek());
        decoder::Decoder decoder({.decomp_context = context,
                                  .code_stream = &stream,
                                  .builder = builder});
        decoder.Decode();
    }

#define DUMP_CFG 0
#if DUMP_CFG
    {
        LOG_INFO(ShaderDecompiler, "Dumping shader CFG 0x{}",
                 (void*)code_reader.GetBase());
        std::ofstream out(
            fmt::format("/Users/samuliak/Downloads/extracted/0x{}.gv",
                        (void*)code_reader.GetBase()),
            std::ios::binary);
        const auto& func = modul.GetFunction("main");
        analyzer::CfgBuilder cfg_builder;
        auto entry_block = cfg_builder.Build(func);
        entry_block->WriteToDot(out);
    }
#endif

    // Analyze

    // Memory
    analyzer::MemoryAnalyzer mem_analyzer;
    mem_analyzer.Analyze(modul);

    // Debug
    LOG_DEBUG(ShaderDecompiler, "Module:\n{}", modul);

    // Decompile
    codegen::Emitter* emitter;
    out_backend = CONFIG_INSTANCE.GetShaderBackend();
    switch (out_backend) {
    case ShaderBackend::Msl: {
        emitter = new codegen::lang::msl::MslEmitter(
            context, mem_analyzer, state, out_code, out_resource_mapping);
        break;
    }
    // case ShaderBackend::Air:
    //     builder = new IR::AIR::Builder(analyzer, type, state, out_code,
    //                                    out_resource_mapping);
    //     break;
    default:
        LOG_FATAL(ShaderDecompiler, "Unsupported shader backend {}",
                  out_backend);
        break;
    }

    emitter->Emit(modul);
}

} // namespace hydra::hw::tegra_x1::gpu::renderer::shader_decomp
