#pragma once

#include "core/hw/tegra_x1/gpu/renderer/shader_decompiler/ir/instruction.hpp"

namespace hydra::hw::tegra_x1::gpu::renderer::shader_decomp::ir {

class Block {
  public:
    Block(const label_t label_) : label{label_} {}

    void AddInstruction(Opcode opcode,
                        const std::optional<Value> dst = std::nullopt,
                        const std::vector<Value>& operands = {}) {
        instructions.emplace_back(opcode, dst, operands);
    }

    Value CreateLocal(Type type) {
        return Value::Local(local_t{.label=label, .id=static_cast<u32>(instructions.size())}, type);
    }

  private:
    const label_t label;
    std::vector<Instruction> instructions;

  public:
    GETTER(label, GetLabel);
    CONST_REF_GETTER(instructions, GetInstructions);
};

} // namespace hydra::hw::tegra_x1::gpu::renderer::shader_decomp::ir

template <>
struct fmt::formatter<
    hydra::hw::tegra_x1::gpu::renderer::shader_decomp::ir::Block>
    : formatter<string_view> {
    template <typename FormatContext>
    auto
    format(const hydra::hw::tegra_x1::gpu::renderer::shader_decomp::ir::Block&
               block,
           FormatContext& ctx) const {
        std::string str = fmt::format("{}:\n", block.GetLabel());
        for (const auto& inst : block.GetInstructions())
            str += fmt::format("{}\n", inst);
        return formatter<string_view>::format(str, ctx);
    }
};
