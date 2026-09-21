#pragma once

namespace hydra {
class System;
}

namespace hydra::horizon::kernel {

using AppletResourceUserId = u64;

constexpr AppletResourceUserId ARUID_BEGIN = 0xa000000000000000ull;
constexpr usize MAX_APPLET_RESOURCES = 0x20;

inline AppletResourceUserId toAruid(usize index) {
    ASSERT_DEBUG(index < MAX_APPLET_RESOURCES, Kernel, "Invalid index {:#x}",
                 index);
    return ARUID_BEGIN + index;
}

inline usize toIndex(AppletResourceUserId aruid) {
    ASSERT_DEBUG(aruid >= ARUID_BEGIN &&
                     aruid < ARUID_BEGIN + MAX_APPLET_RESOURCES,
                 Kernel, "Invalid aruid {:#x}", aruid);
    return aruid - ARUID_BEGIN;
}

template <typename T>
class AppletResourcePool {
    using ResourceArray = std::array<std::optional<T>, MAX_APPLET_RESOURCES>;

  public:
    explicit AppletResourcePool(System& system_) : system{system_} {}

    ResourceArray::iterator begin() { return resources.begin(); }

    ResourceArray::const_iterator begin() const {
        return resources.begin();
    }

    ResourceArray::const_iterator cbegin() const { return begin(); }

    ResourceArray::iterator end() { return resources.end(); }

    ResourceArray::const_iterator end() const {
        return resources.end();
    }

    ResourceArray::const_iterator cend() const { return end(); }

    T& createResource(kernel::AppletResourceUserId aruid) {
        auto& resource = getResourceOpt(aruid);
        ASSERT_DEBUG(!resource.has_value(), Kernel, "Aruid {:#x} already taken",
                     aruid);
        resource.emplace(system);
        return *resource;
    }

    void destroyResource(kernel::AppletResourceUserId aruid) {
        auto& resource = getResourceOpt(aruid);
        ASSERT_DEBUG(resource.has_value(), Kernel, "Invalid aruid {:#x}",
                     aruid);
        resource = std::nullopt;
    }

    T& getResource(kernel::AppletResourceUserId aruid) {
        auto& resource = getResourceOpt(aruid);
        ASSERT_DEBUG(resource.has_value(), Kernel, "Invalid aruid {:#x}",
                     aruid);
        return *resource;
    }

    const T& getResource(kernel::AppletResourceUserId aruid) const {
        auto& resource = getResourceOpt(aruid);
        ASSERT_DEBUG(resource.has_value(), Kernel, "Invalid aruid {:#x}",
                     aruid);
        return *resource;
    }

  private:
    System& system;
    ResourceArray resources{};

    // Helpers
    std::optional<T>& getResourceOpt(AppletResourceUserId aruid) {
        return resources[toIndex(aruid)];
    }

    const std::optional<T>& getResourceOpt(AppletResourceUserId aruid) const {
        return resources[toIndex(aruid)];
    }
};

} // namespace hydra::horizon::kernel
