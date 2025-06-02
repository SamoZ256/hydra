#pragma once

#include "core/hw/tegra_x1/gpu/renderer/shader_decompiler/observer_base.hpp"

namespace hydra::hw::tegra_x1::gpu::renderer::shader_decomp {

enum class ResultCode {
    None,
    Branch,
    BranchConditional,
    SyncPoint,
    EndBlock,

    Error,
};

struct result_t {
    ResultCode code;
    union {
        u32 target;
    };
};

class IteratorBase {
  public:
    IteratorBase(Reader code_reader_) : code_reader{code_reader_} {}
    virtual ~IteratorBase() = default;

    virtual void Iterate(ObserverBase* observer) = 0;

  protected:
    result_t ParseNextInstruction(ObserverBase* observer);

    void Jump(ObserverBase* observer, u32 target) {
        code_reader.Seek(target * sizeof(instruction_t));
        observer->SetPC(target);
        observer->BlockChanged();
    }

    u32 GetPC() const { return code_reader.Tell() / sizeof(instruction_t) - 1; }

  private:
    Reader code_reader;

    result_t ParseNextInstructionImpl(ObserverBase* observer, const u32 pc,
                                      const instruction_t inst);
};

} // namespace hydra::hw::tegra_x1::gpu::renderer::shader_decomp

ENABLE_ENUM_FORMATTING(
    hydra::hw::tegra_x1::gpu::renderer::shader_decomp::ResultCode, None, "none",
    Branch, "branch", BranchConditional, "branch conditional", SyncPoint,
    "sync point", EndBlock, "end block", Error, "error")
