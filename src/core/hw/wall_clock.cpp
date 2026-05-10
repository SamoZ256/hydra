#include "core/hw/wall_clock.hpp"

namespace hydra::hw {

namespace {

u128 GetFactor(u64 num, u64 den) {
    return (static_cast<u128>(num) << 64) / den;
}

u64 MultiplyByFactor(u64 num, u128 factor) { return (num * factor) >> 64; }

} // namespace

SINGLETON_DEFINE_GET_INSTANCE(WallClock, Other)

WallClock::WallClock() {
    SINGLETON_SET_INSTANCE(WallClock, Other);

    const auto host_freq = GetSystemFrequency();
    guest_factor = GetFactor(GUEST_CNTFRQ, host_freq);
    gpu_tick_factor = GetFactor(GPU_TICK_FREQ, host_freq);
}

WallClock::~WallClock() { SINGLETON_UNSET_INSTANCE(); }

u64 WallClock::GetCntpct() const {
    return MultiplyByFactor(GetSystemTick(), guest_factor);
}

u64 WallClock::GetGpuTick() const {
    return MultiplyByFactor(GetSystemTick(), gpu_tick_factor);
}

} // namespace hydra::hw
