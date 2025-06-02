#include "core/hw/tegra_x1/gpu/renderer/shader_decompiler/decompiler.hpp"

#include "core/hw/tegra_x1/gpu/renderer/shader_decompiler/all_paths_iterator.hpp"
#include "core/hw/tegra_x1/gpu/renderer/shader_decompiler/analyzer/cfg_builder.hpp"
#include "core/hw/tegra_x1/gpu/renderer/shader_decompiler/analyzer/memory_analyzer.hpp"
#include "core/hw/tegra_x1/gpu/renderer/shader_decompiler/lang/structured_iterator.hpp"
#include "core/hw/tegra_x1/gpu/renderer/shader_decompiler/lang/structurizer.hpp"
// #include "core/hw/tegra_x1/gpu/renderer/shader_decompiler/ir/air/builder.hpp"
#include "core/hw/tegra_x1/gpu/renderer/shader_decompiler/lang/msl/builder.hpp"

namespace hydra::hw::tegra_x1::gpu::renderer::shader_decomp {

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
        } PACKED vtg;

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
                           ShaderBackend& out_backend,
                           std::vector<u8>& out_code,
                           ResourceMapping& out_resource_mapping) {
    // Header
    // TODO: don't read in case of compute shaders
    const ShaderHeader header = code_reader.Read<ShaderHeader>();
    // HACK: just for testing
    ASSERT_DEBUG(header.version == 3, ShaderDecompiler,
                 "Invalid shader version {}", header.version);

#define DUMP_SHADERS 0
#if DUMP_SHADERS
    std::ofstream out(
        fmt::format("/Users/samuliak/Downloads/extracted/0x{}.bin",
                    (void*)code_reader.GetBase()),
        std::ios::binary);
    out.write(reinterpret_cast<const char*>(code_reader.GetPtr()), 0x1000);
    out.close();
#endif

    // Analyze
    Analyzer::MemoryAnalyzer memory_analyzer;
    {
        AllPathsIterator iterator(code_reader.CreateSubReader());
        iterator.Iterate(&memory_analyzer);
    }

    Analyzer::CfgBuilder cfg_builder;
    {
        AllPathsIterator iterator(code_reader.CreateSubReader());
        iterator.Iterate(&cfg_builder);
    }

    // Debug
    cfg_builder.LogBlocks();

    // Decompile
    BuilderBase* builder;
    IteratorBase* iterator;
    out_backend = CONFIG_INSTANCE.GetShaderBackend();
    switch (out_backend) {
    case ShaderBackend::Msl: {
        builder = new Lang::MSL::Builder(memory_analyzer, type, state, out_code,
                                         out_resource_mapping);
        auto root_block = Lang::Structurize(cfg_builder.GetEntryBlock());

        // Debug
        root_block->Log();

        iterator = new Lang::StructuredIterator(code_reader.CreateSubReader(),
                                                root_block);
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

    builder->InitializeResourceMapping();
    builder->Start();
    iterator->Iterate(builder);
    builder->Finish();
    delete iterator;
    delete builder;
}

} // namespace hydra::hw::tegra_x1::gpu::renderer::shader_decomp
