#include "core/hw/wall_clock.hpp"

namespace hydra::hw {

namespace {

u128 getFactor(u64 num, u64 den) {
    return (static_cast<u128>(num) << 64) / den;
}

u64 multiplyByFactor(u64 num, u128 factor) { return (num * factor) >> 64; }

} // namespace

WallClock::WallClock() {
    const auto host_freq = ztd::getSystemFrequency();
    ns_factor = getFactor(1'000'000'000, host_freq);
    guest_factor = getFactor(GUEST_CNTFRQ, host_freq);
    gpu_tick_factor = getFactor(GPU_TICK_FREQ, host_freq);
}

u64 WallClock::getTimeNs() const {
    return multiplyByFactor(ztd::getSystemTick(), ns_factor);
}

u64 WallClock::getCntpct() const {
    return multiplyByFactor(ztd::getSystemTick(), guest_factor);
}

u64 WallClock::getGpuTick() const {
    return multiplyByFactor(ztd::getSystemTick(), gpu_tick_factor);
}

} // namespace hydra::hw
