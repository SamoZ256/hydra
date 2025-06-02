#include "core/horizon/services/nvdrv/ioctl/nvhost_ctrl_gpu.hpp"

namespace hydra::horizon::services::nvdrv::ioctl {

DEFINE_IOCTL_TABLE(NvHostCtrlGpu,
                   DEFINE_IOCTL_TABLE_ENTRY(NvHostCtrlGpu, 0x47, 0x01,
                                            ZCullGetCtxSize, 0x02, ZCullGetInfo,
                                            0x05, GetCharacteristics, 0x06,
                                            GetTpcMasks, 0x14,
                                            ZbcGetActiveSlotMask))

NvResult NvHostCtrlGpu::QueryEvent(u32 event_id_u32,
                                   handle_id_t& out_handle_id) {
    switch (event_id_u32) {
    case 0x01:
        out_handle_id = error_event.id;
        break;
    case 0x02:
        out_handle_id = unknown_event.id;
        break;
    default:
        LOG_WARN(Services, "Unknown event ID: {:02x}", event_id_u32);
        out_handle_id = INVALID_HANDLE_ID;
        return NvResult::BadParameter;
    }

    return NvResult::Success;
}

NvResult NvHostCtrlGpu::ZCullGetCtxSize(u32* out_size) {
    LOG_FUNC_STUBBED(Services);

    // HACK
    *out_size = 0x20000;
    return NvResult::Success;
}

NvResult NvHostCtrlGpu::ZCullGetInfo(ZCullInfo* out_info) {
    LOG_FUNC_STUBBED(Services);

    // From Ryujinx
    *out_info = ZCullInfo{
        .width_align_pixels = 0x20,
        .height_align_pixels = 0x20,
        .pixel_squares_by_aliquots = 0x400,
        .aliquot_total = 0x800,
        .region_byte_multiplier = 0x20,
        .region_header_size = 0x20,
        .subregion_header_size = 0xc0,
        .subregion_width_align_pixels = 0x20,
        .subregion_height_align_pixels = 0x40,
        .subregion_count = 0x10,
    };
    return NvResult::Success;
}

NvResult
NvHostCtrlGpu::GetCharacteristics(InOutSingle<u64> inout_buffer_size,
                                  gpu_vaddr_t buffer_addr,
                                  GpuCharacteristics* out_characteristics) {
    ASSERT_DEBUG(inout_buffer_size != 0x0, Services,
                 "Invalid buffer size 0x{:08x}", *inout_buffer_size.data);
    inout_buffer_size = 0xa0;

    ASSERT_DEBUG(buffer_addr != 0x0, Services,
                 "Invalid buffer address 0x{:08x}", buffer_addr);

    // Write the characteristics
    *out_characteristics = {
        .arch = 0x120, // NVGPU_GPU_ARCH_GM200
        .impl = 0xb,   // NVGPU_GPU_IMPL_GM20B (or 0xE NVGPU_GPU_IMPL_GM20B_B)
        .rev = 0xa1,   // Revision A1
        .num_gpc = 0x1,
        .l2_cache_size = 0x40000,
        .on_board_video_memory_size = 0x0, // not used
        .num_tpc_per_gpc = 0x2,
        .bus_type = 0x20, // NVGPU_GPU_BUS_TYPE_AXI
        .big_page_size = 0x20000,
        .compression_page_size = 0x20000,
        .pde_coverage_bit_count = 0x1b,
        .available_big_page_sizes = 0x30000,
        .gpc_mask = 0x1,
        .sm_arch_sm_version = 0x503,  // Maxwell Generation 5.0.3
        .sm_arch_spa_version = 0x503, // Maxwell Generation 5.0.3
        .sm_arch_warp_count = 0x80,
        .gpu_va_bit_count = 0x28,
        .reserved = 0x0,
        .flags = 0x55, // TODO: (HAS_SYNCPOINTS | SUPPORT_SPARSE_ALLOCS |
                       // SUPPORT_CYCLE_STATS | SUPPORT_CYCLE_STATS_SNAPSHOT)
        .twod_class = 0x902d,             // FERMI_TWOD_A
        .threed_class = 0xb197,           // MAXWELL_B
        .compute_class = 0xb1c0,          // MAXWELL_COMPUTE_B
        .gpfifo_class = 0xb06f,           // MAXWELL_CHANNEL_GPFIFO_A
        .inline_to_memory_class = 0xa140, // KEPLER_INLINE_TO_MEMORY_B
        .dma_copy_class = 0xb0b5,         // MAXWELL_DMA_COPY_A
        .max_fbps_count = 0x1,
        .fbp_en_mask = 0x0, // disabled
        .max_ltc_per_fbp = 0x2,
        .max_lts_per_ltc = 0x1,
        .max_tex_per_tpc = 0x0, // not supported
        .max_gpc_count = 0x1,
        .rop_l2_en_mask_0 = 0x21d70, // fuse_status_opt_rop_l2_fbp_r
        .rop_l2_en_mask_1 = 0x0,
        .chipname = 0x6230326D67,       // "gm20b"
        .gr_compbit_store_base_hw = 0x0 // not supported
    };
    return NvResult::Success;
}

NvResult NvHostCtrlGpu::GetTpcMasks(u32 mask_buffer_size,
                                    std::array<u32, 3> reserved,
                                    u64* out_mask_buffer) {
    LOG_FUNC_STUBBED(Services);

    ASSERT_DEBUG(mask_buffer_size != 0x0, Services,
                 "Mask buffer size cannot be 0x0");

    // TODO: correct?
    *out_mask_buffer = 0x3;
    return NvResult::Success;
}

NvResult NvHostCtrlGpu::ZbcGetActiveSlotMask(u32* out_slot, u32* out_mask) {
    LOG_FUNC_STUBBED(Services);

    // TODO: correct?
    *out_slot = 0x07;
    *out_mask = 0x01;
    return NvResult::Success;
}

} // namespace hydra::horizon::services::nvdrv::ioctl
