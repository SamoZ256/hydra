#pragma once

#include "core/hw/tegra_x1/gpu/engines/const.hpp"

#define METHOD_CASE(method_begin, method_count, func, arg_type)                \
    case method_begin ...(method_begin + method_count - 1):                    \
        func(method - method_begin, std::bit_cast<arg_type>(arg));             \
        break;

#define DEFINE_METHOD_TABLE(type, ...)                                         \
    void type::Method(u32 method, u32 arg) {                                   \
        if (method >= MACRO_METHODS_REGION) {                                  \
            Macro(method, arg);                                                \
            return;                                                            \
        }                                                                      \
        switch (method) {                                                      \
            FOR_EACH_0_4(METHOD_CASE, __VA_ARGS__)                             \
        default:                                                               \
            WriteReg(method, arg);                                             \
            break;                                                             \
        }                                                                      \
    }

namespace hydra::hw::tegra_x1::gpu::engines {

class EngineBase {
  public:
    virtual ~EngineBase() = default;

    virtual void Method(u32 method, u32 arg) = 0;

    virtual void FlushMacro() {
        LOG_ERROR(Engines, "This engine does not support macros");
    }

  protected:
    virtual void Macro(u32 method, u32 arg) {
        LOG_ERROR(Engines,
                  "This engine does not support macros (method: 0x{:08x})",
                  method);
    }
};

template <typename RegsT> class EngineWithRegsBase : public EngineBase {
  public:
#define REG_COUNT (sizeof(RegsT) / sizeof(u32))

    u32 GetReg(u32 reg) const {
        ASSERT_DEBUG(reg < REG_COUNT, Macro, "Invalid register 0x{:08x}", reg);
        return regs_raw[reg];
    }

  protected:
    union {
        RegsT regs{};
        u32 regs_raw[REG_COUNT];
    };

    void WriteReg(u32 reg, u32 value) {
        ASSERT_DEBUG(reg < REG_COUNT, Engines, "Invalid reg 0x{:08x}", reg);
        LOG_DEBUG(Engines, "Writing to reg 0x{:03x} (value: 0x{:08x})", reg,
                  value);
        regs_raw[reg] = value;
    }

#undef REG_COUNT
};

} // namespace hydra::hw::tegra_x1::gpu::engines
