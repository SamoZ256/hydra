#pragma once

#include "core/horizon/services/const.hpp"

namespace hydra::horizon::services::mmnv {

enum class Module : u32 {
    Cpu = 0,
    Gpu = 1,
    Emc = 2,
    SysBus = 3,
    Mselect = 4,
    Nvdec = 5,
    Nvenc = 6,
    Nvjpg = 7,
    Test = 8,
};

class IRequest : public IService {
  protected:
    result_t requestImpl([[maybe_unused]] RequestContext& context,
                         u32 id) override;

  private:
    // Commands
    result_t initializeOld(Module module, u32 priority, u32 event_clear_mode);
    result_t finalizeOld(Module module);
    result_t setAndWaitOld(Module module, u32 setting_min, u32 setting_max);
    result_t getOld(Module module, u32* out_setting_current);
    result_t initialize(Module module, u32 priority, u32 event_clear_mode,
                        u32* out_request_id);
    result_t finalize(u32 request_id);
    result_t setAndWait(u32 request_id, u32 setting_min, u32 setting_max);
    result_t get(u32 request_id, u32* out_setting_current);
};

} // namespace hydra::horizon::services::mmnv

ENABLE_ENUM_FORMATTING(hydra::horizon::services::mmnv::Module, Cpu, "CPU", Gpu,
                       "GPU", Emc, "EMC", SysBus, "sysbus", Mselect, "M select",
                       Nvdec, "NV dec", Nvenc, "NV enc", Nvjpg, "NV JPG", Test,
                       "test")
