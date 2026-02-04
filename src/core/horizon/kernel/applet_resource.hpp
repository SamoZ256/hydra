#pragma once

namespace hydra::horizon::kernel {

using AppletResourceUserId = u64;

constexpr AppletResourceUserId ARUID_BEGIN = 0xa000000000000000ull;
constexpr usize MAX_APPLET_RESOURCES = 0x20;

enum class ToAruidError {
    InvalidIndex,
};
inline AppletResourceUserId ToAruid(usize index) {
    ASSERT_THROWING_DEBUG(index < MAX_APPLET_RESOURCES, Kernel,
                          ToAruidError::InvalidIndex, "Invalid index {:#x}",
                          index);
    return ARUID_BEGIN + index;
}

enum class ToIndexError {
    InvalidAruid,
};
inline usize ToIndex(AppletResourceUserId aruid) {
    ASSERT_THROWING_DEBUG(
        aruid >= ARUID_BEGIN && aruid < ARUID_BEGIN + MAX_APPLET_RESOURCES,
        Kernel, ToIndexError::InvalidAruid, "Invalid aruid {:#x}", aruid);
    return aruid - ARUID_BEGIN;
}

template <typename T>
class AppletResourcePool {
    typedef std::array<std::optional<T>, MAX_APPLET_RESOURCES> ResourceArray;

  public:
    enum class Error {
        InvalidAruid,
        AruidAlreadyTaken,
    };

    typename ResourceArray::iterator begin() { return resources.begin(); }

    typename ResourceArray::const_iterator begin() const {
        return resources.begin();
    }

    typename ResourceArray::const_iterator cbegin() const { return begin(); }

    typename ResourceArray::iterator end() { return resources.end(); }

    typename ResourceArray::const_iterator end() const {
        return resources.end();
    }

    typename ResourceArray::const_iterator cend() const { return end(); }

    T& CreateResource(kernel::AppletResourceUserId aruid) {
        auto& resource = GetResourceOpt(aruid);
        ASSERT_THROWING_DEBUG(!resource.has_value(), Kernel,
                              Error::AruidAlreadyTaken,
                              "Aruid {:#x} already taken", aruid);
        resource.emplace();
        return *resource;
    }

    void DestroyResource(kernel::AppletResourceUserId aruid) {
        auto& resource = GetResourceOpt(aruid);
        ASSERT_THROWING_DEBUG(resource.has_value(), Kernel, Error::InvalidAruid,
                              "Invalid aruid {:#x}", aruid);
        resource = std::nullopt;
    }

    T& GetResource(kernel::AppletResourceUserId aruid) {
        auto& resource = GetResourceOpt(aruid);
        ASSERT_THROWING_DEBUG(resource.has_value(), Kernel, Error::InvalidAruid,
                              "Invalid aruid {:#x}", aruid);
        return *resource;
    }

    const T& GetResource(kernel::AppletResourceUserId aruid) const {
        auto& resource = GetResourceOpt(aruid);
        ASSERT_THROWING_DEBUG(resource.has_value(), Kernel, Error::InvalidAruid,
                              "Invalid aruid {:#x}", aruid);
        return *resource;
    }

  private:
    ResourceArray resources{};

    // Helpers
    std::optional<T>& GetResourceOpt(AppletResourceUserId aruid) {
        return resources[ToIndex(aruid)];
    }

    const std::optional<T>& GetResourceOpt(AppletResourceUserId aruid) const {
        return resources[ToIndex(aruid)];
    }
};

} // namespace hydra::horizon::kernel
