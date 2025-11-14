#include "core/hw/tegra_x1/gpu/renderer/shader_decompiler/codegen/lang/msl/emitter.hpp"

#include "core/hw/tegra_x1/gpu/renderer/shader_cache.hpp"
#include "core/hw/tegra_x1/gpu/renderer/shader_decompiler/analyzer/memory_analyzer.hpp"

namespace hydra::hw::tegra_x1::gpu::renderer::shader_decomp::codegen::lang::
    msl {

namespace {

// TODO: adjust for individual texture types
std::string dimension_to_str(u32 dimension) {
    switch (dimension) {
    case 0:
        return "width";
    case 1:
        return "height";
    case 2:
        return "depth";
    default:
        return INVALID_VALUE;
    }
}

} // namespace

MslEmitter::MslEmitter(const DecompilerContext& context,
                       const analyzer::MemoryAnalyzer& memory_analyzer,
                       const GuestShaderState& state, std::vector<u8>& out_code,
                       ResourceMapping& out_resource_mapping)
    : LangEmitter(context, memory_analyzer, state, out_code,
                  out_resource_mapping) {
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

void MslEmitter::EmitHeader() {
    Write("#include <metal_stdlib>");
    Write("using namespace metal;");
}

void MslEmitter::EmitTypeAliases() {
    Write("using u8 = uint8_t;");
    Write("using u16 = uint16_t;");
    Write("using u32 = uint32_t;");
    Write("using i8 = int8_t;");
    Write("using i16 = int16_t;");
    Write("using i32 = int32_t;");
    Write("using f16 = half;");
    Write("using f32 = float;");
}

void MslEmitter::EmitDeclarations() {
    // Stage inputs

    EnterScope("struct StageIn");

    // SVs
    // Handled in GetMainArgs

    // Stage inputs
    switch (context.type) {
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
                  GetSvStr(sv), GetSvQualifierStr(sv, false));
        }
        break;
    case ShaderType::Fragment:
        Write("float4 position [[position]];");
        for (const auto input : memory_analyzer.GetStageInputs()) {
            const auto sv = Sv(SvSemantic::UserInOut, input);
            // TODO: don't hardcode the type
            Write("float4 {} {};", GetSvStr(sv), GetSvQualifierStr(sv, false));
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
    // HACK: always write position in vertex shaders
    if (context.type == ShaderType::Vertex)
        Write("float4 position [[position]];");
    for (const auto sv_semantic : memory_analyzer.GetOutputSVs()) {
        switch (sv_semantic) {
        case SvSemantic::Position:
            // Write("float4 position [[position]];");
            break;
        default:
            LOG_NOT_IMPLEMENTED(ShaderDecompiler, "Output SV semantic {}",
                                sv_semantic);
            break;
        }
    }

    // Stage outputs
    switch (context.type) {
    case ShaderType::Vertex:
        for (const auto output : memory_analyzer.GetStageOutputs()) {
            const auto sv = Sv(SvSemantic::UserInOut, output);
            // TODO: don't hardcode the type
            Write("float4 {} {};", GetSvStr(sv), GetSvQualifierStr(sv, true));
        }
        break;
    case ShaderType::Fragment:
        for (u32 i = 0; i < COLOR_TARGET_COUNT; i++) {
            const auto color_target_format = state.color_target_formats[i];
            if (color_target_format == TextureFormat::Invalid)
                continue;

            const auto sv = Sv(SvSemantic::UserInOut, i);
            Write("vec<{}, 4> {} {};", to_data_type(color_target_format),
                  GetSvStr(sv), GetSvQualifierStr(sv, true));
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

void MslEmitter::EmitStateBindings() {
    // Storage buffers
    // TODO

    // Textures
    for (const auto const_buffer_index : memory_analyzer.GetTextures()) {
        // TODO: don't hardcode texture type
        WriteStatement("texture2d<float> tex{}", const_buffer_index);
        WriteStatement("sampler samplr{}", const_buffer_index);
    }
}

void MslEmitter::EmitStateBindingAssignments() {
    // Storage buffers
    // TODO

    // Textures
    for (const auto const_buffer_index : memory_analyzer.GetTextures()) {
        WriteStatement("state.tex{} = tex{}", const_buffer_index,
                       const_buffer_index);
        WriteStatement("state.samplr{} = samplr{}", const_buffer_index,
                       const_buffer_index);
    }
}

void MslEmitter::EmitMainPrototype() {
    switch (context.type) {
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

#define ADD_ARG(f, ...) WriteRaw(", " f PASS_VA_ARGS(__VA_ARGS__))

    // Input SVs
    switch (context.type) {
    case ShaderType::Vertex:
        ADD_ARG("uint iid [[instance_id]]");
        ADD_ARG("uint vid [[vertex_id]]");
        break;
    case ShaderType::Fragment:
        break;
    default:
        break;
    }

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

    EmitMainFunctionPrologue();
}

void MslEmitter::EmitExitReturn() {
    if (context.type == ShaderType::Vertex) {
        // Flip vertically
        // TODO: handle this with viewports?
        // WriteStatement("__out.position.y = -__out.position.y");

        // Convert depth from < -1, 1 > to < 0, 1 >
        // TODO: only if enabled?
        WriteStatement(
            "__out.position.z = (__out.position.z + __out.position.w) / 2.0");
    }

    // Return
    WriteStatement("return __out");
}

void MslEmitter::EmitDiscard() { WriteStatement("discard_fragment()"); }

void MslEmitter::EmitTextureSample(const ir::Value& dst, u32 const_buffer_index,
                                   const ir::Value& coords) {
    StoreValue(dst, "state.tex{}.sample(state.samplr{}, {})",
               const_buffer_index, const_buffer_index, GetValueStr(coords));
}

void MslEmitter::EmitTextureRead(const ir::Value& dst, u32 const_buffer_index,
                                 const ir::Value& coords) {
    StoreValue(dst, "state.tex{}.read(uint2({}))", const_buffer_index,
               GetValueStr(coords));
}

void MslEmitter::EmitTextureQueryDimension(const ir::Value& dst,
                                           u32 const_buffer_index,
                                           u32 dimension) {
    StoreValue(dst, "state.tex{}.get_{}()", const_buffer_index,
               dimension_to_str(dimension));
}

std::string MslEmitter::GetSvAccessQualifiedStr(const SvAccess& sv_access,
                                                bool output) {
    bool needs_in_out = (sv_access.sv.semantic == SvSemantic::Position ||
                         sv_access.sv.semantic == SvSemantic::UserInOut);
    bool is_vec = (sv_access.sv.semantic == SvSemantic::Position ||
                   sv_access.sv.semantic == SvSemantic::UserInOut);

    // TODO: is it okay to access components just like this?
    return fmt::format(
        "{}{}{}", (needs_in_out ? (output ? "__out." : "__in.") : ""),
        GetSvStr(sv_access.sv),
        (is_vec ? fmt::format(".{}", GetComponentStrFromIndex(
                                         sv_access.component_index))
                : ""));
}

std::string MslEmitter::GetSvStr(const Sv& sv) {
    switch (sv.semantic) {
    case SvSemantic::Position:
        return "position";
    case SvSemantic::UserInOut:
        return fmt::format("user{}", sv.index);
    case SvSemantic::InstanceID:
        return "iid";
    case SvSemantic::VertexID:
        return "vid";
    default:
        LOG_NOT_IMPLEMENTED(ShaderDecompiler, "SV {} (index: {})", sv.semantic,
                            sv.index);
        return INVALID_VALUE;
    }
}

std::string MslEmitter::GetSvQualifierStr(const Sv& sv, bool output) {
    switch (sv.semantic) {
    case SvSemantic::Position:
        return "[[position]]";
    case SvSemantic::UserInOut:
        switch (context.type) {
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
    case SvSemantic::InstanceID:
        return "[[instance_id]]";
    case SvSemantic::VertexID:
        return "[[vertex_id]]";
    default:
        LOG_ERROR(ShaderDecompiler, "Unknown SV semantic {}", sv.semantic);
        return INVALID_VALUE;
    }
}

} // namespace
  // hydra::hw::tegra_x1::gpu::renderer::shader_decomp::codegen::lang::msl
