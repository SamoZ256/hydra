#include "hw/tegra_x1/gpu/renderer/shader_decompiler/lang/msl/builder.hpp"

#include "hw/tegra_x1/gpu/renderer/shader_cache.hpp"
#include "hw/tegra_x1/gpu/renderer/shader_decompiler/analyzer.hpp"

namespace Hydra::HW::TegraX1::GPU::Renderer::ShaderDecompiler::Lang::MSL {

void Builder::InitializeResourceMapping() {
    for (const auto& [index, size] : analyzer.GetUniformBuffers()) {
        out_resource_mapping.uniform_buffers[index] = index;
    }
    // TODO: storage buffers
    for (const auto index : analyzer.GetTextures()) {
        out_resource_mapping.textures[index] = index;
        out_resource_mapping.samplers[index] = index;
    }
    // TODO: images
}

void Builder::EmitHeader() {
    Write("#include <metal_stdlib>");
    Write("using namespace metal;");
}

void Builder::EmitTypeAliases() {
    // Do nothing
}

void Builder::EmitDeclarations() {
    // Stage inputs

    EnterScope("struct StageIn");

    // SVs
    // Handled in GetMainArgs

    // Stage inputs
    switch (type) {
    case ShaderType::Vertex:
        for (u32 i = 0; i < VERTEX_ATTRIB_COUNT; i++) {
            const auto vertex_attrib_state = state.vertex_attrib_states[i];
            if (vertex_attrib_state.type == Engines::VertexAttribType::None)
                continue;

            // HACK: how are attributes disabled?
            if (vertex_attrib_state.is_fixed)
                continue;

            const auto sv = SV(SVSemantic::UserInOut, i);
            Write("{}4 {} {};", to_data_type(vertex_attrib_state.type),
                  GetSVName(sv), GetSVQualifierName(sv, false));
        }
        break;
    case ShaderType::Fragment:
        Write("float4 position [[position]];");
        for (const auto input : analyzer.GetStageInputs()) {
            const auto sv = SV(SVSemantic::UserInOut, input);
            // TODO: don't hardcode the type
            Write("float4 {} {};", GetSVName(sv),
                  GetSVQualifierName(sv, false));
        }
        break;
    default:
        break;
    }

    ExitScopeEmpty(true);
    WriteNewline();

    // Stage outputs

    EnterScope("struct StageOut");

    // SVs
    for (const auto sv_semantic : analyzer.GetOutputSVs()) {
        switch (sv_semantic) {
        case SVSemantic::Position:
            Write("float4 position [[position]];");
            break;
        default:
            LOG_NOT_IMPLEMENTED(ShaderDecompiler, "Output SV semantic {}",
                                sv_semantic);
            break;
        }
    }

    // Stage outputs
    switch (type) {
    case ShaderType::Vertex:
        for (const auto output : analyzer.GetStageOutputs()) {
            const auto sv = SV(SVSemantic::UserInOut, output);
            // TODO: don't hardcode the type
            Write("float4 {} {};", GetSVName(sv), GetSVQualifierName(sv, true));
        }
        break;
    case ShaderType::Fragment:
        for (u32 i = 0; i < COLOR_TARGET_COUNT; i++) {
            const auto color_target_format = state.color_target_formats[i];
            if (color_target_format == TextureFormat::Invalid)
                continue;

            const auto sv = SV(SVSemantic::UserInOut, i);
            Write("{}4 {} {};", to_data_type(color_target_format),
                  GetSVName(sv), GetSVQualifierName(sv, true));
        }
        break;
    default:
        break;
    }

    ExitScopeEmpty(true);
    WriteNewline();

    // Uniform buffers
    for (const auto& [index, size] : analyzer.GetUniformBuffers()) {
        EnterScope("struct UBuff{}", index);

        // Data
        Write("uint data[{}];", size / sizeof(u32));

        ExitScopeEmpty(true);
    }
    WriteNewline();
}

void Builder::EmitMainPrototype() {
    switch (type) {
    case ShaderType::Vertex:
        Write("vertex");
        break;
    case ShaderType::Fragment:
        Write("fragment");
        break;
    default:
        Write(INVALID_VALUE);
        break;
    }
    WriteRaw("StageOut main_(StageIn __in [[stage_in]]");

#define ADD_ARG(f, ...) WriteRaw(", {}", fmt::format(f, __VA_ARGS__))

    // Input SVs
    // TODO

    // Uniform buffers
    for (const auto& [index, size] : analyzer.GetUniformBuffers()) {
        ADD_ARG("constant UBuff{}& ubuff{} [[buffer({})]]", index, index,
                out_resource_mapping.uniform_buffers[index]);
    }

    // Storage buffers
    // TODO

    // Textures
    for (const auto index : analyzer.GetTextures()) {
        // TODO: don't hardcode texture type
        ADD_ARG("texture2d<float> tex{} [[texture({})]]", index,
                out_resource_mapping.textures[index]);
        ADD_ARG("sampler samplr{} [[sampler({})]]", index,
                out_resource_mapping.samplers[index]);
    }

    // Images
    // TODO

#undef ADD_ARG

    EnterScope(")");
}

std::string Builder::GetSVQualifierName(const SV sv, bool output) {
    switch (sv.semantic) {
    case SVSemantic::Position:
        return "[[position]]";
    case SVSemantic::UserInOut:
        switch (type) {
        case ShaderType::Vertex:
            if (output)
                return fmt::format("[[user(locn{})]]", sv.index);
            else
                return fmt::format("[[attribute({})]]", sv.index);
        case ShaderType::Fragment:
            if (output)
                return fmt::format("[[color({})]]", sv.index);
            else
                return fmt::format("[[user(locn{})]]", sv.index);
        default:
            return INVALID_VALUE;
        }
    default:
        LOG_ERROR(ShaderDecompiler, "Unknown SV semantic {}", sv.semantic);
        return INVALID_VALUE;
    }
}

std::string Builder::EmitTextureSample(u32 index, const std::string& coords) {
    return fmt::format("tex{}.sample(samplr{}, {})", index, index, coords);
}

} // namespace Hydra::HW::TegraX1::GPU::Renderer::ShaderDecompiler::Lang::MSL
