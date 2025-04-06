#pragma once

#include "horizon/services/nvdrv/ioctl/fd_base.hpp"
#include "hw/tegra_x1/gpu/const.hpp"

namespace Hydra::Horizon::Services::NvDrv::Ioctl {

enum class AllocSpaceFlags : u32 {
    None = 0,
    FixedOffset = BIT(0),
    Sparse = BIT(1),
};

ENABLE_ENUM_BITMASK_OPERATORS(AllocSpaceFlags)

enum class MapBufferFlags : u32 {
    None = 0,
    FixedOffset = BIT(0),
    IsCacheable = BIT(2),
    Modify = BIT(8),
};

ENABLE_ENUM_BITMASK_OPERATORS(MapBufferFlags)

class NvHostAsGpu : public FdBase {
  public:
    void Ioctl(IOCTL_PARAMS) override;

  private:
    // Ioctls
    DECLARE_IOCTL(
        AllocSpace, readonly<u32> pages; readonly<u32> page_size;
        readonly<AllocSpaceFlags> flags; readonly<u32> pad; union {
            writeonly<u64> offset;
            readonly<u64> align_or_offset;
        },
                                                            offset);
    DECLARE_IOCTL(MapBufferEx, readonly<MapBufferFlags> flags;
                  readwrite<HW::TegraX1::GPU::NvKind> kind;
                  readonly<u32> nvmap_handle_id; u32 reserved;
                  readonly<u64> buffer_offset; readonly<u64> mapping_size;
                  readwrite<u64> offset;, kind, offset);
    DECLARE_IOCTL(AllocASEx, readonly<u32> big_page_size; readonly<i32> as_fd;
                  readonly<u32> flags; readonly<u32> reserved;
                  readonly<u64> va_range_start; readonly<u64> va_range_end;
                  readonly<u64> va_range_split;);
};

} // namespace Hydra::Horizon::Services::NvDrv::Ioctl
