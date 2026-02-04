#include "core/horizon/services/nvdrv/ioctl/nvhost_as_gpu.hpp"

#include "core/horizon/kernel/process.hpp"
#include "core/hw/tegra_x1/cpu/mmu.hpp"
#include "core/hw/tegra_x1/gpu/gpu.hpp"

namespace hydra::horizon::services::nvdrv::ioctl {

DEFINE_IOCTL_TABLE(NvHostAsGpu,
                   DEFINE_IOCTL_TABLE_ENTRY(NvHostAsGpu, 0x41, 0x01,
                                            BindChannel, 0x02, AllocSpace, 0x03,
                                            FreeSpace, 0x05, UnmapBuffer, 0x06,
                                            MapBufferEX, 0x08, GetVaRegions,
                                            0x09, AllocAsEX, 0x14, Remap))

DEFINE_IOCTL3_TABLE(NvHostAsGpu, DEFINE_IOCTL3_TABLE_ENTRY(NvHostAsGpu, 0x41,
                                                           0x08, GetVaRegions))

NvResult NvHostAsGpu::BindChannel(u32 fd_id) {
    LOG_FUNC_WITH_ARGS_STUBBED(Services, "FD: {}", fd_id);
    return NvResult::Success;
}

NvResult NvHostAsGpu::AllocSpace(kernel::Process* process, u32 pages,
                                 u32 page_size,
                                 aligned<AllocSpaceFlags, 8> flags,
                                 InOut<u64, gpu_vaddr_t> align_and_offset) {
    uptr gpu_addr = invalid<uptr>();
    if (any(flags & AllocSpaceFlags::FixedOffset))
        gpu_addr = align_and_offset; // TODO: is it really align?

    align_and_offset = process->GetGMmu()->AllocatePrivateAddressSpace(
        static_cast<usize>(pages) * static_cast<usize>(page_size), gpu_addr);
    return NvResult::Success;
}

NvResult NvHostAsGpu::FreeSpace(vaddr_t offset, u32 pages, u32 page_size) {
    LOG_FUNC_WITH_ARGS_STUBBED(Services,
                               "offset: {:#x}, pages: {}, page_size: {:#x}",
                               offset, pages, page_size);
    return NvResult::Success;
}

NvResult NvHostAsGpu::UnmapBuffer(gpu_vaddr_t addr) {
    LOG_FUNC_WITH_ARGS_STUBBED(Services, "address: {:#x}", addr);
    return NvResult::Success;
}

// TODO: kind
NvResult NvHostAsGpu::MapBufferEX(kernel::Process* process,
                                  MapBufferFlags flags,
                                  hw::tegra_x1::gpu::NvKind kind,
                                  handle_id_t nvmap_handle_id,
                                  [[maybe_unused]] u32 reserved,
                                  u64 buffer_offset, u64 mapping_size,
                                  InOutSingle<gpu_vaddr_t> inout_addr) {
    (void)kind;

    if (any(flags & MapBufferFlags::Modify)) {
        LOG_NOT_IMPLEMENTED(
            Services, "Address space modifying (Gpu addr: {:#x}, size: {:#x})",
            *inout_addr.data, mapping_size);
        return NvResult::Success;
    }

    const auto& map = GPU_INSTANCE.GetMap(nvmap_handle_id);

    usize size = mapping_size;
    if (size == 0x0)
        size = map.size; // TODO: correct?

    gpu_vaddr_t addr = invalid<uptr>();
    if (any(flags & MapBufferFlags::FixedOffset))
        addr = inout_addr;

    inout_addr = process->GetGMmu()->MapBufferToAddressSpace(
        Range<vaddr_t>::FromSize(map.addr + buffer_offset, size), addr);
    return NvResult::Success;
}

NvResult NvHostAsGpu::GetVaRegions(gpu_vaddr_t buffer_addr,
                                   InOutSingle<u32> inout_buffer_size,
                                   [[maybe_unused]] u32 reserved,
                                   std::array<VaRegion, 2>* out_va_regions) {
    LOG_FUNC_WITH_ARGS_STUBBED(Services,
                               "buffer address: {:#x}, buffer size: {}",
                               buffer_addr, inout_buffer_size);

    inout_buffer_size = 2 * sizeof(VaRegion);
    // HACK
    (*out_va_regions)[0] =
        VaRegion{.addr = 0x0, .page_size = hw::tegra_x1::gpu::GPU_PAGE_SIZE};
    (*out_va_regions)[1] = (*out_va_regions)[0];
    return NvResult::Success;
}

NvResult NvHostAsGpu::AllocAsEX(kernel::Process* process, u32 big_page_size,
                                i32 as_fd, u32 flags,
                                [[maybe_unused]] u32 reserved,
                                u64 va_range_start, u64 va_range_end,
                                u64 va_range_split) {
    LOG_FUNC_WITH_ARGS_STUBBED(Services,
                               "big page size: {}, as fd: {}, flags: {}, "
                               "start: {:#x}, end: {:#x}, split: {:#x}",
                               big_page_size, as_fd, flags, va_range_start,
                               va_range_end, va_range_split);

    // TODO: why does nouveau pass 0x0 for all of these?

    // TODO: what is split for?
    process->GetGMmu()->AllocatePrivateAddressSpace(
        va_range_end - va_range_start, va_range_start);
    return NvResult::Success;
}

NvResult NvHostAsGpu::Remap(const RemapOp* entries) {
    // TODO: what should the entry count be?
    std::span<const RemapOp> entries_span(entries, 1);
    LOG_FUNC_WITH_ARGS_STUBBED(Services, "entries: {}",
                               fmt::join(entries_span, ", "));
    return NvResult::Success;
}

} // namespace hydra::horizon::services::nvdrv::ioctl
