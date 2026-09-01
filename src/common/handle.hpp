#pragma once

#include "common/type_aliases.hpp"

namespace hydra {

struct Handle {
  public:
    static constexpr Handle FromIndex(usize index) noexcept {
        return {static_cast<u32>(index + 1)};
    }

    constexpr Handle() noexcept = default;
    constexpr Handle(u32 raw_) noexcept : raw{raw_} {}

    bool operator==(Handle other) const noexcept { return raw == other.raw; }

    u32 GetRaw() const noexcept { return raw; }

    std::optional<usize> ToIndex() const noexcept {
        if (raw == 0)
            return std::nullopt;
        return raw - 1;
    }

    [[nodiscard]] bool IsValid() const noexcept { return raw != 0; }

  private:
    u32 raw{0};
};

constexpr Handle INVALID_HANDLE = Handle(0);

} // namespace hydra

template <>
struct fmt::formatter<hydra::Handle> : formatter<string_view> {
    template <typename FormatContext>
    auto format(hydra::Handle handle, FormatContext& ctx) const {
        if (!handle.IsValid())
            return formatter<string_view>::format("null", ctx);
        return formatter<string_view>::format(
            fmt::format("{:#x}", handle.GetRaw()), ctx);
    }
};
