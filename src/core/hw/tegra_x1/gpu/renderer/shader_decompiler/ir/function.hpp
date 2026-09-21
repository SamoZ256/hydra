#pragma once

#include "core/hw/tegra_x1/gpu/renderer/shader_decompiler/ir/block.hpp"

namespace hydra::hw::tegra_x1::gpu::renderer::shader_decomp::ir {

class Function {
  public:
    explicit Function(const std::string_view name_) : name{name_} {}

    Block& getBlock(label_t label) {
        auto it = blocks.find(label);
        if (it == blocks.end())
            return blocks.insert({label, Block(label)}).first->second;
        return it->second;
    }
    const Block& getBlock(label_t label) const { return blocks.at(label); }

  private:
    std::string name;
    std::map<label_t, Block> blocks;

  public:
    std::string_view getName() const { return name; }
    CONST_REF_GETTER(blocks, getBlocks);
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
        std::string str = fmt::format("func {} {{\n", func.getName());
        for (auto it = func.getBlocks().begin(); it != func.getBlocks().end();
             it++) {
            str += fmt::format(
                "{}{}", it->second,
                (it == std::prev(func.getBlocks().end()) ? "" : "\n"));
        }
        str += "}\n";
        return formatter<string_view>::format(str, ctx);
    }
};
