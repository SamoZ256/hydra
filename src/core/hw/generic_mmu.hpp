#pragma once

namespace hydra::hw {

template <typename SubclassT, typename Impl> class GenericMMU {
  public:
    void Map(uptr base, Impl impl) {
        mapped_ranges[base] = impl;
        THIS->MapImpl(base, impl);
    }

    void Unmap(uptr base) {
        auto it = mapped_ranges.find(base);
        ASSERT_DEBUG(it != mapped_ranges.end(), MMU,
                     "Failed to unmap with base 0x{:08x}", base);
        THIS->UnmapImpl(base, it->second);
        mapped_ranges.erase(it);
    }

    /*
    void Remap(uptr base) {
        auto impl = mapped_ranges.at(base);
        THIS->UnmapImpl(base, impl);
        THIS->MapImpl(base, impl);
    }
    */

    Impl* FindAddrImplRef(uptr addr, uptr& out_base) {
        for (auto& [base, impl] : mapped_ranges) {
            if (addr >= base && addr < base + THIS->ImplGetSize(impl)) {
                out_base = base;
                return &impl;
            }
        }

        return nullptr;
    }

    const Impl& FindAddrImpl(uptr addr, uptr& out_base) const {
        auto impl =
            const_cast<GenericMMU*>(this)->FindAddrImplRef(addr, out_base);
        ASSERT_DEBUG(impl, MMU, "Failed to find impl for addr 0x{:08x}", addr);

        return *impl;
    }

    template <typename T> T Load(uptr addr) const {
        return *reinterpret_cast<T*>(THIS->UnmapAddr(addr));
    }

    template <typename T> void Store(uptr addr, T value) const {
        *reinterpret_cast<T*>(THIS->UnmapAddr(addr)) = value;
    }

  private:
    std::map<uptr, Impl> mapped_ranges;
};

} // namespace hydra::hw
