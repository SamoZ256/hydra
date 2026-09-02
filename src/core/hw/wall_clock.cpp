#include "core/hw/wall_clock.hpp"

namespace hydra::hw {

namespace {

u128 GetFactor(u64 num, u64 den) {
    return (static_cast<u128>(num) << 64) / den;
}

u64 MultiplyByFactor(u64 num, u128 factor) { return (num * factor) >> 64; }

} // namespace

WallClock::WallClock() {
    const auto host_freq = ztd::getSystemFrequency();
    ns_factor = GetFactor(1'000'000'000, host_freq);
    guest_factor = GetFactor(GUEST_CNTFRQ, host_freq);
    gpu_tick_factor = GetFactor(GPU_TICK_FREQ, host_freq);
}

u64 WallClock::GetTimeNs() const {
    return MultiplyByFactor(ztd::getSystemTick(), ns_factor);
}

u64 WallClock::GetCntpct() const {
    return MultiplyByFactor(ztd::getSystemTick(), guest_factor);
}

u64 WallClock::GetGpuTick() const {
    return MultiplyByFactor(ztd::getSystemTick(), gpu_tick_factor);
}

} // namespace hydra::hw
