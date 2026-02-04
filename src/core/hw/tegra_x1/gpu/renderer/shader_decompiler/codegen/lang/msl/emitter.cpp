#include "core/hw/tegra_x1/gpu/renderer/shader_decompiler/codegen/lang/msl/emitter.hpp"

#include "core/hw/tegra_x1/gpu/renderer/shader_cache.hpp"
#include "core/hw/tegra_x1/gpu/renderer/shader_decompiler/analyzer/memory_analyzer.hpp"
#include "core/hw/tegra_x1/gpu/renderer/shader_decompiler/codegen/helper.hpp"

namespace hydra::hw::tegra_x1::gpu::renderer::shader_decomp::codegen::lang::
    msl {

namespace {

std::string PixelImapTypeToStr(PixelImapType type) {
    switch (type) {
    case PixelImapType::Constant:
        return "flat";
    case PixelImapType::Perspective:
        return "";
    case PixelImapType::ScreenLinear:
        return "center_no_perspective";
    default:
        LOG_ERROR(ShaderDecompiler, "Pixel imap unused");
        return "";
    }
}

std::string TextureTypeToStr(TextureType type, bool is_depth) {
    // TODO: check if depth can be used with the type
    std::string prefix = is_depth ? "depth" : "texture";
    switch (type) {
    case TextureType::_1D:
        return prefix + "1d";
    case TextureType::_1DArray:
        return prefix + "1d_array";
    case TextureType::_2D:
        return prefix + "2d";
    case TextureType::_2DArray:
        return prefix + "2d_array";
    case TextureType::_3D:
        return prefix + "3d";
    case TextureType::_3DArray:
        return prefix + "3d_array";
    case TextureType::Cube:
        return prefix + "cube";
    case TextureType::CubeArray:
        return prefix + "cube_array";
    }
}

std::string ComponentToStr(u8 component) {
    switch (component) {
    case 0:
        return "x";
    case 1:
        return "y";
    case 2:
        return "z";
    case 3:
        return "w";
    default:
        return INVALID_VALUE;
    }
}

// TODO: adjust for individual texture types
std::string DimensionToStr(u32 dimension) {
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
    for (auto index : memory_analyzer.GetConstBuffers()) {
        out_resource_mapping.uniform_buffers[index] = index;
    }

    // TODO: storage buffers

    u32 texture_index = 0;
    for (const auto& [const_buffer_index, _] : memory_analyzer.GetTextures()) {
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
        for (u8 i = 0; i < VERTEX_ATTRIB_COUNT; i++) {
            const auto vertex_attrib_state = state.vertex_attrib_states[i];
            if (vertex_attrib_state.type == engines::VertexAttribType::None)
                continue;

            // HACK: how are attributes disabled?
            if (vertex_attrib_state.is_fixed)
                continue;

            const auto sv = Sv(SvSemantic::UserInOut, i);
            Write("vec<{}, 4> {} [[{}]];", ToType(vertex_attrib_state.type),
                  GetSvStr(sv), GetSvQualifierStr(sv, false));
        }
        break;
    case ShaderType::Fragment:
        Write("float4 position [[position]];");
        for (const auto input : memory_analyzer.GetStageInputs()) {
            const auto sv = Sv(SvSemantic::UserInOut, input);
            std::string attribute = PixelImapTypeToStr(
                context.frag.pixel_imaps[input].GetFirstUsedType());
            // TODO: don't hardcode the type
            Write("float4 {} [[{}{}{}]];", GetSvStr(sv),
                  GetSvQualifierStr(sv, false), attribute.empty() ? "" : ", ",
                  attribute);
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
        Write("float4 position [[position, invariant]];");
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
            Write("float4 {} [[{}]];", GetSvStr(sv),
                  GetSvQualifierStr(sv, true));
        }
        break;
    case ShaderType::Fragment:
        for (u8 i = 0; i < COLOR_TARGET_COUNT; i++) {
            const auto color_target_data_type =
                state.color_target_data_types[i];
            if (color_target_data_type == ColorDataType::Invalid)
                continue;

            const auto sv = Sv(SvSemantic::UserInOut, i);
            Write("vec<{}, 4> {} [[{}]];", ToType(color_target_data_type),
                  GetSvStr(sv), GetSvQualifierStr(sv, true));
        }
        break;
    default:
        break;
    }

    ExitScopeEmpty(true);
    WriteNewline();
    ;
}

void MslEmitter::EmitStateBindings() {
    // Storage buffers
    // TODO

    // Textures
    for (const auto& [const_buffer_index, info] :
         memory_analyzer.GetTextures()) {
        // TODO: don't hardcode type
        WriteStatement("{}<float> tex{}",
                       TextureTypeToStr(info.type, info.is_depth),
                       const_buffer_index);
        WriteStatement("sampler samplr{}", const_buffer_index);
    }
}

void MslEmitter::EmitStateBindingAssignments() {
    // Storage buffers
    // TODO

    // Textures
    for (const auto& [const_buffer_index, _] : memory_analyzer.GetTextures()) {
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
    for (auto index : memory_analyzer.GetConstBuffers()) {
        ADD_ARG("constant Reg* c{} [[buffer({})]]", index, index,
                out_resource_mapping.uniform_buffers[index]);
    }

    // Storage buffers
    // TODO

    // Textures
    for (const auto& [const_buffer_index, info] :
         memory_analyzer.GetTextures()) {
        const auto index = out_resource_mapping.textures[const_buffer_index];
        // TODO: don't hardcode type
        ADD_ARG("{}<float> tex{} [[texture({})]]",
                TextureTypeToStr(info.type, info.is_depth), const_buffer_index,
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

// Data
void MslEmitter::EmitBitCast(const ir::Value& dst, const ir::Value& src) {
    StoreValue(dst, "as_type<{}>({})", GetTypeStr(dst.GetType()),
               GetValueStr(src));
}

// Math
void MslEmitter::EmitIsNan(const ir::Value& dst, const ir::Value& src) {
    StoreValue(dst, "isnan({})", GetValueStr(src));
}

void MslEmitter::EmitReciprocal(const ir::Value& dst, const ir::Value& src) {
    StoreValue(dst, "(1.0 / {})", GetValueStr(src));
}

void MslEmitter::EmitSin(const ir::Value& dst, const ir::Value& src) {
    StoreValue(dst, "sin({})", GetValueStr(src));
}

void MslEmitter::EmitCos(const ir::Value& dst, const ir::Value& src) {
    StoreValue(dst, "cos({})", GetValueStr(src));
}

void MslEmitter::EmitExp2(const ir::Value& dst, const ir::Value& src) {
    StoreValue(dst, "exp2({})", GetValueStr(src));
}

void MslEmitter::EmitLog2(const ir::Value& dst, const ir::Value& src) {
    StoreValue(dst, "log2({})", GetValueStr(src));
}

void MslEmitter::EmitSqrt(const ir::Value& dst, const ir::Value& src) {
    StoreValue(dst, "sqrt({})", GetValueStr(src));
}

void MslEmitter::EmitReciprocalSqrt(const ir::Value& dst,
                                    const ir::Value& src) {
    StoreValue(dst, "rsqrt({})", GetValueStr(src));
}

// Logical & Bitwise
void MslEmitter::EmitBitfieldExtract(const ir::Value& dst,
                                     const ir::Value& src_a,
                                     const ir::Value& src_b,
                                     const ir::Value& src_c) {
    StoreValue(dst, "extract_bits({}, {}, {})", GetValueStr(src_a),
               GetValueStr(src_b), GetValueStr(src_c));
}

// Texture
void MslEmitter::EmitTextureSample(const ir::Value& dst, u32 const_buffer_index,
                                   TextureType type, TextureSampleFlags flags,
                                   const ir::Value& array_index,
                                   const ir::Value& coords,
                                   const ir::Value& cmp_value,
                                   const ir::Value& lod) {
    // Flags
    const auto is_array = IsTextureArray(type);
    const auto int_coords = any(flags & TextureSampleFlags::IntCoords);
    const auto depth_compare = any(flags & TextureSampleFlags::DepthCompare);
    const auto has_lod = any(flags & TextureSampleFlags::Lod);

    std::string func_name;
    std::string args;
    if (int_coords) {
        func_name = "read";
        args = fmt::format("uint2({})", GetValueStr(coords));
        if (depth_compare) {
            // TODO: emulate
            LOG_NOT_IMPLEMENTED(ShaderDecompiler, "Texture read depth compare");
        }
    } else {
        func_name = "sample";
        if (depth_compare)
            func_name += "_compare";
        args = fmt::format("state.samplr{}, {}", const_buffer_index,
                           GetValueStr(coords));
    }

    // Args
    if (is_array)
        args += fmt::format(", uint({})", GetValueStr(array_index));
    if (depth_compare)
        args += fmt::format(", {}", GetValueStr(cmp_value));
    if (has_lod)
        args += fmt::format(", level({})", GetValueStr(lod));

    std::string res =
        fmt::format("state.tex{}.{}({})", const_buffer_index, func_name, args);
    // HACK: construct float4 if sample_compare
    if (depth_compare)
        res = fmt::format("float4({}, 0.0, 0.0, 0.0)", res);
    StoreValue(dst, "{}", res);
}

void MslEmitter::EmitTextureGather(const ir::Value& dst, u32 const_buffer_index,
                                   const ir::Value& coords, u8 component) {
    StoreValue(dst,
               "state.tex{}.gather(state.samplr{}, {}, int2(0), component::{})",
               const_buffer_index, const_buffer_index, GetValueStr(coords),
               ComponentToStr(component));
}

void MslEmitter::EmitTextureQueryDimension(const ir::Value& dst,
                                           u32 const_buffer_index,
                                           u32 dimension) {
    StoreValue(dst, "state.tex{}.get_{}()", const_buffer_index,
               DimensionToStr(dimension));
}

// Exit
void MslEmitter::EmitDiscard() { WriteStatement("discard_fragment()"); }

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
        return "position";
    case SvSemantic::UserInOut:
        switch (context.type) {
        case ShaderType::Vertex:
            if (output)
                return fmt::format("user(locn{})", sv.index);
            else
                return fmt::format("attribute({})", sv.index);
        case ShaderType::Fragment:
            if (output)
                return fmt::format("color({})", sv.index);
            else
                return fmt::format("user(locn{})", sv.index);
        default:
            return INVALID_VALUE;
        }
    case SvSemantic::InstanceID:
        return "instance_id";
    case SvSemantic::VertexID:
        return "vertex_id";
    default:
        LOG_ERROR(ShaderDecompiler, "Unknown SV semantic {}", sv.semantic);
        return INVALID_VALUE;
    }
}

} // namespace
  // hydra::hw::tegra_x1::gpu::renderer::shader_decomp::codegen::lang::msl
