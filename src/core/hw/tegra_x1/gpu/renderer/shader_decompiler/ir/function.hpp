#pragma once

#include "core/hw/tegra_x1/gpu/renderer/shader_decompiler/ir/block.hpp"

namespace hydra::hw::tegra_x1::gpu::renderer::shader_decomp::ir {

class Function {
  public:
    Function(const std::string_view name_) : name{name_} {}

    Block& GetBlock(label_t label) {
        auto it = blocks.find(label);
        if (it == blocks.end())
            return blocks.insert({label, Block(label)}).first->second;
        return it->second;
    }
    const Block& GetBlock(label_t label) const { return blocks.at(label); }

  private:
    std::string name;
    std::map<label_t, Block> blocks;

  public:
    std::string_view GetName() const { return name; }
    CONST_REF_GETTER(blocks, GetBlocks);
};

} // namespace hydra::hw::tegra_x1::gpu::renderer::shader_decomp::ir

template <>
struct fmt::formatter<
    hydra::hw::tegra_x1::gpu::renderer::shader_decomp::ir::Function>
    : formatter<string_view> {
    template <typename FormatContext>
    auto format(
        const hydra::hw::tegra_x1::gpu::renderer::shader_decomp::ir::Function&
            func,
        FormatContext& ctx) const {
        std::string str = fmt::format("func {} {{\n", func.GetName());
        for (auto it = func.GetBlocks().begin(); it != func.GetBlocks().end();
             it++) {
            str += fmt::format(
                "{}{}", it->second,
                (it == std::prev(func.GetBlocks().end()) ? "" : "\n"));
        }
        str += "}\n";
        return formatter<string_view>::format(str, ctx);
    }
};
