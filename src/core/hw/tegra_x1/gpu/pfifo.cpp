#include "core/hw/tegra_x1/gpu/pfifo.hpp"

#include "core/hw/tegra_x1/gpu/const.hpp"
#include "core/hw/tegra_x1/gpu/gpu.hpp"

namespace hydra::hw::tegra_x1::gpu {

namespace {

enum class SecondaryOpcode : u32 {
    Grp0UseTert,
    IncMethod,
    Grp2UseTert,
    NonIncMethod,
    ImmDataMethod,
    OneInc,
    Reserved,
    EndPbSegment,
};

// TODO: do both GRP0 and GRP2 use the same tertiary opcodes?
enum class TertiaryOpcode : u32 {
    Grp0IncMethod = 0,
    Grp2NonIncMethod = 0,
    Grp0SetSubDevMask = 1,
    Grp0StoreSubDevMask = 2,
    Grp0UseSubDevMask = 3,
};

} // namespace

} // namespace hydra::hw::tegra_x1::gpu

ENABLE_ENUM_FORMATTING(hydra::hw::tegra_x1::gpu::SecondaryOpcode, Grp0UseTert,
                       "GRP0 use tertiary opcode", IncMethod,
                       "incrementing method", Grp2UseTert,
                       "GRP2 use tertiary opcode", NonIncMethod,
                       "non-incremental method", ImmDataMethod,
                       "immediate data method", OneInc, "one increment",
                       Reserved, "reserved", EndPbSegment, "End PB segment")

ENABLE_ENUM_FORMATTING(hydra::hw::tegra_x1::gpu::TertiaryOpcode, Grp0IncMethod,
                       "GRP0 increment method / GRP2 non-increment method",
                       Grp0SetSubDevMask, "GRP0 set subdevice mask",
                       Grp0StoreSubDevMask, "GRP0 store subdevice mask",
                       Grp0UseSubDevMask, "GRP0 use subdevice mask")

namespace hydra::hw::tegra_x1::gpu {

namespace {

struct CommandHeader {
    u32 method : 12;
    u32 reserved : 1;
    u32 subchannel : 3;
    u32 arg : 13;
    SecondaryOpcode secondary_opcode : 3;
};

} // namespace

void Pfifo::SubmitEntries(const std::vector<GpfifoEntry>& entries,
                          GpfifoFlags flags) {
    LOG_DEBUG(GPU, "Flags: {}", flags);
    RENDERER_INSTANCE->LockMutex();
    for (const auto& entry : entries) {
        SubmitEntry(entry);
    }
    RENDERER_INSTANCE->EndCommandBuffer();
    RENDERER_INSTANCE->UnlockMutex();
}

void Pfifo::SubmitEntry(const GpfifoEntry entry) {
    LOG_DEBUG(GPU,
              "Gpfifo entry (address: 0x{:08x}, size: 0x{:08x}, allow "
              "flush: {}, is push buffer: {}, sync: {})",
              entry.gpu_addr, entry.size, entry.allow_flush,
              entry.is_push_buffer, entry.sync);

    uptr gpu_addr = entry.gpu_addr;
    uptr end = entry.gpu_addr + entry.size * sizeof(u32);

    while (gpu_addr < end) {
        if (!SubmitCommand(gpu_addr))
            break;
    }
}

bool Pfifo::SubmitCommand(uptr& gpu_addr) {
    const auto header = Read<CommandHeader>(gpu_addr);
    LOG_DEBUG(GPU, "Secondary opcode: {}", header.secondary_opcode);

    // HACK
    if (header.subchannel >= SUBCHANNEL_COUNT) {
        ONCE(LOG_WARN(GPU, "Invalid subchannel {}", header.subchannel));
        return false;
    }

#define TERTIARY_OPCODE                                                        \
    static_cast<TertiaryOpcode>(                                               \
        extract_bits<u32, 16, 17>(std::bit_cast<u32>(header)))

    u32 offset = header.method;
    switch (header.secondary_opcode) {
    case SecondaryOpcode::Grp0UseTert: {
        switch (TERTIARY_OPCODE) {
        case TertiaryOpcode::Grp0IncMethod:
            // TODO: correct?
            for (u32 i = 0; i < header.arg; i++)
                ProcessMethodArg(header.subchannel, gpu_addr, offset, true);
            break;
        default:
            LOG_NOT_IMPLEMENTED(GPU, "Tertiary opcode {}", TERTIARY_OPCODE);
            break;
        }
        break;
    }
    case SecondaryOpcode::IncMethod:
        for (u32 i = 0; i < header.arg; i++)
            ProcessMethodArg(header.subchannel, gpu_addr, offset, true);
        break;
    case SecondaryOpcode::Grp2UseTert: {
        switch (TERTIARY_OPCODE) {
        case TertiaryOpcode::Grp2NonIncMethod:
            // TODO: correct?
            for (u32 i = 0; i < header.arg; i++)
                ProcessMethodArg(header.subchannel, gpu_addr, offset, false);
            break;
        default:
            LOG_NOT_IMPLEMENTED(GPU, "Tertiary opcode {}", TERTIARY_OPCODE);
            break;
        }
        break;
    }
    case SecondaryOpcode::NonIncMethod:
        for (u32 i = 0; i < header.arg; i++)
            ProcessMethodArg(header.subchannel, gpu_addr, offset, false);
        break;
    case SecondaryOpcode::ImmDataMethod:
        GPU::GetInstance().SubchannelMethod(header.subchannel, offset,
                                            header.arg);
        break;
    case SecondaryOpcode::OneInc:
        for (u32 i = 0; i < header.arg; i++)
            ProcessMethodArg(header.subchannel, gpu_addr, offset, i == 0);
        break;
    default:
        LOG_NOT_IMPLEMENTED(GPU, "Secondary opcode {}",
                            header.secondary_opcode);
        break;
    }

    // TODO: is it okay to prefetch the parameters and then execute the
    // macro?
    if (header.method >= MACRO_METHODS_REGION)
        GPU::GetInstance().SubchannelFlushMacro(header.subchannel);

    return true;
}

void Pfifo::ProcessMethodArg(u32 subchannel, uptr& gpu_addr, u32& method,
                             bool increment) {
    u32 arg = Read<u32>(gpu_addr);
    GPU::GetInstance().SubchannelMethod(subchannel, method, arg);
    if (increment)
        method++;
}

} // namespace hydra::hw::tegra_x1::gpu
