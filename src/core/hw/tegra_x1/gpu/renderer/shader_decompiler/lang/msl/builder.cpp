#include "core/hw/tegra_x1/gpu/renderer/shader_decompiler/lang/msl/builder.hpp"

#include "core/hw/tegra_x1/gpu/renderer/shader_cache.hpp"
#include "core/hw/tegra_x1/gpu/renderer/shader_decompiler/analyzer/memory_analyzer.hpp"

namespace hydra::hw::tegra_x1::gpu::renderer::shader_decomp::Lang::MSL {

void Builder::InitializeResourceMapping() {
    for (const auto& [index, size] : memory_analyzer.GetUniformBuffers()) {
        out_resource_mapping.uniform_buffers[index] = index;
    }

    // TODO: storage buffers

    u32 texture_index = 0;
    for (const auto const_buffer_index : memory_analyzer.GetTextures()) {
        out_resource_mapping.textures[const_buffer_index] = texture_index++;
    }

    // TODO: images
}

void Builder::OpDiscard() { Write("discard_fragment();"); }

void Builder::EmitHeader() {
    Write("#include <metal_stdlib>");
    Write("using namespace metal;");
}

void Builder::EmitTypeAliases() {
    Write("using u8 = uint8_t;");
    Write("using u16 = uint16_t;");
    Write("using u32 = uint32_t;");
    Write("using i8 = int8_t;");
    Write("using i16 = int16_t;");
    Write("using i32 = int32_t;");
    Write("using f16 = half;");
    Write("using f32 = float;");
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
            if (vertex_attrib_state.type == engines::VertexAttribType::None)
                continue;

            // HACK: how are attributes disabled?
            if (vertex_attrib_state.is_fixed)
                continue;

            const auto sv = Sv(SvSemantic::UserInOut, i);
            Write("vec<{}, 4> {} {};", to_data_type(vertex_attrib_state.type),
                  GetSvName(sv), GetSvQualifierName(sv, false));
        }
        break;
    case ShaderType::Fragment:
        Write("float4 position [[position]];");
        for (const auto input : memory_analyzer.GetStageInputs()) {
            const auto sv = Sv(SvSemantic::UserInOut, input);
            // TODO: don't hardcode the type
            Write("float4 {} {};", GetSvName(sv),
                  GetSvQualifierName(sv, false));
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
    for (const auto sv_semantic : memory_analyzer.GetOutputSVs()) {
        switch (sv_semantic) {
        case SvSemantic::Position:
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
        for (const auto output : memory_analyzer.GetStageOutputs()) {
            const auto sv = Sv(SvSemantic::UserInOut, output);
            // TODO: don't hardcode the type
            Write("float4 {} {};", GetSvName(sv), GetSvQualifierName(sv, true));
        }
        break;
    case ShaderType::Fragment:
        for (u32 i = 0; i < COLOR_TARGET_COUNT; i++) {
            const auto color_target_format = state.color_target_formats[i];
            if (color_target_format == TextureFormat::Invalid)
                continue;

            const auto sv = Sv(SvSemantic::UserInOut, i);
            Write("vec<{}, 4> {} {};", to_data_type(color_target_format),
                  GetSvName(sv), GetSvQualifierName(sv, true));
        }
        break;
    default:
        break;
    }

    ExitScopeEmpty(true);
    WriteNewline();

    // Uniform buffers
    for (const auto& [index, size] : memory_analyzer.GetUniformBuffers()) {
        EnterScope("struct UBuff{}", index);

        // Data
        Write("uint data[0x{:x}];", size / sizeof(u32));

        ExitScopeEmpty(true);
    }
    WriteNewline();
}

void Builder::EmitMainPrototype() {
    switch (type) {
    case ShaderType::Vertex:
        WriteRaw("vertex ");
        break;
    case ShaderType::Fragment:
        WriteRaw("fragment ");
        break;
    default:
        WriteRaw(INVALID_VALUE " ");
        break;
    }
    WriteRaw("StageOut main_(StageIn __in [[stage_in]]");

#define ADD_ARG(f, ...) WriteRaw(", {}", fmt::format(f, __VA_ARGS__))

    // Input SVs
    // TODO

    // Uniform buffers
    for (const auto& [index, size] : memory_analyzer.GetUniformBuffers()) {
        ADD_ARG("constant UBuff{}& ubuff{} [[buffer({})]]", index, index,
                out_resource_mapping.uniform_buffers[index]);
    }

    // Storage buffers
    // TODO

    // Textures
    for (const auto const_buffer_index : memory_analyzer.GetTextures()) {
        const auto index = out_resource_mapping.textures[const_buffer_index];
        // TODO: don't hardcode texture type
        ADD_ARG("texture2d<float> tex{} [[texture({})]]", const_buffer_index,
                index);
        ADD_ARG("sampler samplr{} [[sampler({})]]", const_buffer_index, index);
    }

    // Images
    // TODO

#undef ADD_ARG

    EnterScope(")");

    // Output
    Write("StageOut __out;");
    WriteNewline();
}

void Builder::EmitExit() {
    // HACK
    if (type == ShaderType::Vertex)
        WriteStatement(
            "__out.position.z = (__out.position.z + __out.position.w) / 2.0");

    // Return
    WriteStatement("return __out");
}

std::string Builder::GetSvQualifierName(const Sv& sv, bool output) {
    switch (sv.semantic) {
    case SvSemantic::Position:
        return "[[position]]";
    case SvSemantic::UserInOut:
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

std::string Builder::EmitTextureSample(u32 const_buffer_index,
                                       const std::string_view coords) {
    return fmt::format("tex{}.sample(samplr{}, {})", const_buffer_index,
                       const_buffer_index, coords);
}

} // namespace hydra::hw::tegra_x1::gpu::renderer::shader_decomp::Lang::MSL
