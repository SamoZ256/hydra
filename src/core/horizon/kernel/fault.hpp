#pragma once

namespace hydra::horizon::kernel {

void reportFaultOnThisThread(std::string_view reason);

} // namespace hydra::horizon::kernel

#define FAULT_ASSERT(condition, f, ...)                                        \
    if (!(condition)) {                                                        \
        hydra::horizon::kernel::reportFaultOnThisThread(                       \
            fmt::format(f ZTD_PASS_VA_ARGS(__VA_ARGS__)));                     \
    }

#ifdef HYDRA_DEBUG
#define FAULT_ASSERT_DEBUG(condition, ...) FAULT_ASSERT(condition, __VA_ARGS__)
#else
#define FAULT_ASSERT_DEBUG(condition, ...)                                     \
    if (condition) {                                                           \
    }
#endif
