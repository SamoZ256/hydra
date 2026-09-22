#include "core/hw/tegra_x1/gpu/renderer/shader_decompiler/codegen/lang/msl/emitter.hpp"

#include "core/hw/tegra_x1/gpu/renderer/shader_cache.hpp"
#include "core/hw/tegra_x1/gpu/renderer/shader_decompiler/analyzer/memory_analyzer.hpp"
#include "core/hw/tegra_x1/gpu/renderer/shader_decompiler/codegen/helper.hpp"

namespace hydra::hw::tegra_x1::gpu::renderer::shader_decomp::codegen::lang::
    msl {

namespace {

std::string_view pixelImapTypeToStr(PixelImapType type) {
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

std::string textureTypeToStr(TextureType type, bool is_depth) {
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

std::string_view componentToStr(u8 component) {
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
std::string_view dimensionToStr(u32 dimension) {
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
    for (auto index : memory_analyzer.getConstBuffers()) {
        out_resource_mapping.uniform_buffers[index] = index;
    }

    // TODO: storage buffers

    u32 texture_index = 0;
    for (const auto& [const_buffer_index, _] : memory_analyzer.getTextures()) {
        out_resource_mapping.textures[const_buffer_index] = texture_index++;
    }

    // TODO: images
}

void MslEmitter::emitHeader() {
    write("#include <metal_stdlib>");
    write("using namespace metal;");
}

void MslEmitter::emitTypeAliases() {
    write("using u8 = uint8_t;");
    write("using u16 = uint16_t;");
    write("using u32 = uint32_t;");
    write("using i8 = int8_t;");
    write("using i16 = int16_t;");
    write("using i32 = int32_t;");
    write("using f16 = half;");
    write("using f32 = float;");
}

void MslEmitter::emitDeclarations() {
    // Stage inputs

    enterScope("struct StageIn");

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
            write("vec<{}, 4> {} [[{}]];", toType(vertex_attrib_state.type),
                  getSvStr(sv), getSvQualifierStr(sv, false));
        }
        break;
    case ShaderType::Fragment:
        write("float4 position [[position]];");
        for (const auto input : memory_analyzer.getStageInputs()) {
            const auto sv = Sv(SvSemantic::UserInOut, input);
            const auto attribute = pixelImapTypeToStr(
                context.frag.pixel_imaps[input].getFirstUsedType());
            // TODO: don't hardcode the type
            write("float4 {} [[{}{}{}]];", getSvStr(sv),
                  getSvQualifierStr(sv, false), attribute.empty() ? "" : ", ",
                  attribute);
        }
        break;
    default:
        break;
    }

    exitScopeEmpty(true);
    writeNewline();

    // Stage outputs

    enterScope("struct StageOut");

    // SVs
    // HACK: always write position in vertex shaders
    if (context.type == ShaderType::Vertex)
        write("float4 position [[position, invariant]];");
    for (const auto sv_semantic : memory_analyzer.getOutputSVs()) {
        // NOLINTNEXTLINE(readability-trivial-switch)
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
        for (const auto output : memory_analyzer.getStageOutputs()) {
            const auto sv = Sv(SvSemantic::UserInOut, output);
            // TODO: don't hardcode the type
            write("float4 {} [[{}]];", getSvStr(sv),
                  getSvQualifierStr(sv, true));
        }
        break;
    case ShaderType::Fragment:
        for (u8 i = 0; i < COLOR_TARGET_COUNT; i++) {
            const auto color_target_data_type =
                state.color_target_data_types[i];
            if (color_target_data_type == ColorDataType::Invalid)
                continue;

            const auto sv = Sv(SvSemantic::UserInOut, i);
            write("vec<{}, 4> {} [[{}]];", toType(color_target_data_type),
                  getSvStr(sv), getSvQualifierStr(sv, true));
        }
        break;
    default:
        break;
    }

    exitScopeEmpty(true);
    writeNewline();
    ;
}

void MslEmitter::emitStateBindings() {
    // Storage buffers
    // TODO

    // Textures
    for (const auto& [const_buffer_index, info] :
         memory_analyzer.getTextures()) {
        // TODO: don't hardcode type
        writeStatement("{}<float> tex{}",
                       textureTypeToStr(info.type, info.is_depth),
                       const_buffer_index);
        writeStatement("sampler samplr{}", const_buffer_index);
    }
}

void MslEmitter::emitStateBindingAssignments() {
    // Storage buffers
    // TODO

    // Textures
    for (const auto& [const_buffer_index, _] : memory_analyzer.getTextures()) {
        writeStatement("state.tex{} = tex{}", const_buffer_index,
                       const_buffer_index);
        writeStatement("state.samplr{} = samplr{}", const_buffer_index,
                       const_buffer_index);
    }
}

void MslEmitter::emitMainPrototype() {
    switch (context.type) {
    case ShaderType::Vertex:
        writeRaw("vertex ");
        break;
    case ShaderType::Fragment:
        writeRaw("fragment ");
        break;
    default:
        writeRaw(INVALID_VALUE " ");
        break;
    }
    writeRaw("StageOut main_(StageIn __in [[stage_in]]");

#define ADD_ARG(f, ...) writeRaw(", " f ZTD_PASS_VA_ARGS(__VA_ARGS__))

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
    for (auto index : memory_analyzer.getConstBuffers()) {
        ADD_ARG("constant Reg* c{} [[buffer({})]]", index, index,
                out_resource_mapping.uniform_buffers[index]);
    }

    // Storage buffers
    // TODO

    // Textures
    for (const auto& [const_buffer_index, info] :
         memory_analyzer.getTextures()) {
        const auto index = out_resource_mapping.textures[const_buffer_index];
        // TODO: don't hardcode type
        ADD_ARG("{}<float> tex{} [[texture({})]]",
                textureTypeToStr(info.type, info.is_depth), const_buffer_index,
                index);
        ADD_ARG("sampler samplr{} [[sampler({})]]", const_buffer_index, index);
    }

    // Images
    // TODO

#undef ADD_ARG

    enterScope(")");

    // Output
    write("StageOut __out;");
    writeNewline();

    emitMainFunctionPrologue();
}

void MslEmitter::emitExitReturn() {
    if (context.type == ShaderType::Vertex) {
        // Flip vertically
        // TODO: handle this with viewports?
        // WriteStatement("__out.position.y = -__out.position.y");

        // Convert depth from < -1, 1 > to < 0, 1 >
        // TODO: only if enabled?
        writeStatement(
            "__out.position.z = (__out.position.z + __out.position.w) / 2.0");
    }

    // Return
    writeStatement("return __out");
}

// Data
void MslEmitter::emitBitCast(const ir::Value& dst, const ir::Value& src) {
    storeValue(dst, "as_type<{}>({})", getTypeStr(dst.getType()),
               getValueStr(src));
}

// Math
void MslEmitter::emitIsNan(const ir::Value& dst, const ir::Value& src) {
    storeValue(dst, "isnan({})", getValueStr(src));
}

void MslEmitter::emitReciprocal(const ir::Value& dst, const ir::Value& src) {
    storeValue(dst, "(1.0 / {})", getValueStr(src));
}

void MslEmitter::emitSin(const ir::Value& dst, const ir::Value& src) {
    storeValue(dst, "sin({})", getValueStr(src));
}

void MslEmitter::emitCos(const ir::Value& dst, const ir::Value& src) {
    storeValue(dst, "cos({})", getValueStr(src));
}

void MslEmitter::emitExp2(const ir::Value& dst, const ir::Value& src) {
    storeValue(dst, "exp2({})", getValueStr(src));
}

void MslEmitter::emitLog2(const ir::Value& dst, const ir::Value& src) {
    storeValue(dst, "log2({})", getValueStr(src));
}

void MslEmitter::emitSqrt(const ir::Value& dst, const ir::Value& src) {
    storeValue(dst, "sqrt({})", getValueStr(src));
}

void MslEmitter::emitReciprocalSqrt(const ir::Value& dst,
                                    const ir::Value& src) {
    storeValue(dst, "rsqrt({})", getValueStr(src));
}

// Logical & Bitwise
void MslEmitter::emitBitfieldExtract(const ir::Value& dst,
                                     const ir::Value& src_a,
                                     const ir::Value& src_b,
                                     const ir::Value& src_c) {
    storeValue(dst, "extractBits({}, {}, {})", getValueStr(src_a),
               getValueStr(src_b), getValueStr(src_c));
}

// Texture
void MslEmitter::emitTextureSample(const ir::Value& dst, u32 const_buffer_index,
                                   TextureType type, TextureSampleFlags flags,
                                   const ir::Value& array_index,
                                   const ir::Value& coords,
                                   const ir::Value& cmp_value,
                                   const ir::Value& lod) {
    // Flags
    const auto is_array = isTextureArray(type);
    const auto int_coords = any(flags & TextureSampleFlags::IntCoords);
    const auto depth_compare = any(flags & TextureSampleFlags::DepthCompare);
    const auto has_lod = any(flags & TextureSampleFlags::Lod);

    std::string func_name;
    std::string args;
    if (int_coords) {
        func_name = "read";
        args = fmt::format("uint2({})", getValueStr(coords));
        if (depth_compare) {
            // TODO: emulate
            LOG_NOT_IMPLEMENTED(ShaderDecompiler, "Texture read depth compare");
        }
    } else {
        func_name = "sample";
        if (depth_compare)
            func_name += "_compare";
        args = fmt::format("state.samplr{}, {}", const_buffer_index,
                           getValueStr(coords));
    }

    // Args
    if (is_array)
        args += fmt::format(", uint({})", getValueStr(array_index));
    if (depth_compare)
        args += fmt::format(", {}", getValueStr(cmp_value));
    if (has_lod)
        args += fmt::format(", level({})", getValueStr(lod));

    std::string res =
        fmt::format("state.tex{}.{}({})", const_buffer_index, func_name, args);
    // HACK: construct float4 if sample_compare
    if (depth_compare)
        res = fmt::format("float4({}, 0.0, 0.0, 0.0)", res);
    storeValue(dst, "{}", res);
}

void MslEmitter::emitTextureGather(const ir::Value& dst, u32 const_buffer_index,
                                   const ir::Value& coords, u8 component) {
    storeValue(dst,
               "state.tex{}.gather(state.samplr{}, {}, int2(0), component::{})",
               const_buffer_index, const_buffer_index, getValueStr(coords),
               componentToStr(component));
}

void MslEmitter::emitTextureQueryDimension(const ir::Value& dst,
                                           u32 const_buffer_index,
                                           u32 dimension) {
    storeValue(dst, "state.tex{}.get_{}()", const_buffer_index,
               dimensionToStr(dimension));
}

// Exit
void MslEmitter::emitDiscard() { writeStatement("discard_fragment()"); }

std::string MslEmitter::getSvAccessQualifiedStr(const SvAccess& sv_access,
                                                bool output) {
    bool needs_in_out = (sv_access.sv.semantic == SvSemantic::Position ||
                         sv_access.sv.semantic == SvSemantic::UserInOut);
    bool is_vec = (sv_access.sv.semantic == SvSemantic::Position ||
                   sv_access.sv.semantic == SvSemantic::UserInOut);

    // TODO: is it okay to access components just like this?
    return fmt::format(
        "{}{}{}", (needs_in_out ? (output ? "__out." : "__in.") : ""),
        getSvStr(sv_access.sv),
        (is_vec ? fmt::format(".{}", getComponentStrFromIndex(
                                         sv_access.component_index))
                : ""));
}

std::string MslEmitter::getSvStr(const Sv& sv) {
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

std::string MslEmitter::getSvQualifierStr(const Sv& sv, bool output) {
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
