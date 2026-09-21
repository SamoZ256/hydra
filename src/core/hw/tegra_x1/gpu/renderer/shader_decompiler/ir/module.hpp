#pragma once

#include "core/hw/tegra_x1/gpu/renderer/shader_decompiler/ir/function.hpp"

namespace hydra::hw::tegra_x1::gpu::renderer::shader_decomp::ir {

class Module {
  public:
    // TODO: is validation needed?
    static void validate() { LOG_FUNC_NOT_IMPLEMENTED(ShaderDecompiler); }

    Function& getFunction(const std::string& name) {
        auto it = functions.find(name);
        if (it == functions.end())
            return functions.insert({name, Function(name)}).first->second;
        return it->second;
    }

  private:
    std::map<std::string, Function> functions;

  public:
    CONST_REF_GETTER(functions, getFunctions);
};

} // namespace hydra::hw::tegra_x1::gpu::renderer::shader_decomp::ir

template <>
struct fmt::formatter<
    hydra::hw::tegra_x1::gpu::renderer::shader_decomp::ir::Module>
    : formatter<string_view> {
    template <typename FormatContext>
    auto
    format(const hydra::hw::tegra_x1::gpu::renderer::shader_decomp::ir::Module&
               modul,
           FormatContext& ctx) const {
        std::string str;
        for (const auto& [name, func] : modul.getFunctions())
            str += fmt::format("{}\n", func);
        return formatter<string_view>::format(str, ctx);
    }
};
