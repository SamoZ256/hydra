#pragma once

#include "hw/tegra_x1/gpu/engines/const.hpp"
#include "hw/tegra_x1/gpu/renderer/const.hpp"
#include "hw/tegra_x1/gpu/shader_decompiler/const.hpp"

namespace Hydra::HW::TegraX1::GPU {
struct GuestShaderState;
}

namespace Hydra::HW::TegraX1::GPU::ShaderDecompiler {

class ObserverBase;
class BuilderBase;

class Decompiler final {
  public:
    Decompiler() = default;
    ~Decompiler() = default;

    void Decompile(Reader& code_reader, const Renderer::ShaderType type,
                   const GuestShaderState& state, std::vector<u8>& out_code,
                   Renderer::ResourceMapping& out_resource_mapping);

  private:
    void ParseInstruction(ObserverBase* observer, u64 inst);
    void Parse(ObserverBase* observer, Reader& code_reader);
};

} // namespace Hydra::HW::TegraX1::GPU::ShaderDecompiler
