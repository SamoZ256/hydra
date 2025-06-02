#include "core/hw/tegra_x1/gpu/engines/3d.hpp"

#include "core/hw/tegra_x1/gpu/gpu.hpp"
#include "core/hw/tegra_x1/gpu/macro/interpreter/driver.hpp"
#include "core/hw/tegra_x1/gpu/renderer/buffer_base.hpp"
#include "core/hw/tegra_x1/gpu/renderer/render_pass_base.hpp"
#include "core/hw/tegra_x1/gpu/renderer/sampler_base.hpp"
#include "core/hw/tegra_x1/gpu/renderer/shader_base.hpp"
#include "core/hw/tegra_x1/gpu/renderer/texture_base.hpp"

namespace hydra::hw::tegra_x1::gpu::engines {

namespace {

u32 get_image_handle(u32 handle) { return extract_bits<u32, 0, 20>(handle); }
u32 get_sampler_handle(u32 handle) { return extract_bits<u32, 20, 12>(handle); }

constexpr u32 GL_MIN = 0x8007;
constexpr u32 GL_MAX = 0x8008;
constexpr u32 GL_FUNC_ADD = 0x8006;
constexpr u32 GL_FUNC_SUBTRACT = 0x800A;
constexpr u32 GL_FUNC_REVERSE_SUBTRACT = 0x800B;

constexpr u32 D3D11_BLEND_OP_ADD = 1;
constexpr u32 D3D11_BLEND_OP_SUB = 2;
constexpr u32 D3D11_BLEND_OP_REV_SUB = 3;
constexpr u32 D3D11_BLEND_OP_MIN = 4;
constexpr u32 D3D11_BLEND_OP_MAX = 5;

renderer::BlendOperation get_blend_operation(u32 blend_op) {
    switch (blend_op) {
        // GL
    case GL_MIN:
        return renderer::BlendOperation::Min;
    case GL_MAX:
        return renderer::BlendOperation::Max;
    case GL_FUNC_ADD:
        return renderer::BlendOperation::Add;
    case GL_FUNC_SUBTRACT:
        return renderer::BlendOperation::Sub;
    case GL_FUNC_REVERSE_SUBTRACT:
        return renderer::BlendOperation::RevSub;

    // DX11
    case D3D11_BLEND_OP_ADD:
        return renderer::BlendOperation::Add;
    case D3D11_BLEND_OP_SUB:
        return renderer::BlendOperation::Sub;
    case D3D11_BLEND_OP_REV_SUB:
        return renderer::BlendOperation::RevSub;
    case D3D11_BLEND_OP_MIN:
        return renderer::BlendOperation::Min;
    case D3D11_BLEND_OP_MAX:
        return renderer::BlendOperation::Max;
    default:
        LOG_ERROR(Engines, "Unknown blend operation 0x{:04x}", blend_op);
        return renderer::BlendOperation::Add;
    }
}

constexpr u32 GL_ZERO = 0;
constexpr u32 GL_ONE = 1;
constexpr u32 GL_SRC_COLOR = 0x0300;
constexpr u32 GL_ONE_MINUS_SRC_COLOR = 0x0301;
constexpr u32 GL_SRC_ALPHA = 0x0302;
constexpr u32 GL_ONE_MINUS_SRC_ALPHA = 0x0303;
constexpr u32 GL_DST_ALPHA = 0x0304;
constexpr u32 GL_ONE_MINUS_DST_ALPHA = 0x0305;
constexpr u32 GL_DST_COLOR = 0x0306;
constexpr u32 GL_ONE_MINUS_DST_COLOR = 0x0307;
constexpr u32 GL_SRC_ALPHA_SATURATE = 0x0308;
// TODO: more

constexpr u32 D3D11_BLEND_FACTOR_ZERO = 1;
constexpr u32 D3D11_BLEND_FACTOR_ONE = 2;
constexpr u32 D3D11_BLEND_FACTOR_SRC_COLOR = 3;
constexpr u32 D3D11_BLEND_FACTOR_INV_SRC_COLOR = 4;
constexpr u32 D3D11_BLEND_FACTOR_SRC_ALPHA = 5;
constexpr u32 D3D11_BLEND_FACTOR_INV_SRC_ALPHA = 6;
constexpr u32 D3D11_BLEND_FACTOR_DST_ALPHA = 7;
constexpr u32 D3D11_BLEND_FACTOR_INV_DST_ALPHA = 8;
constexpr u32 D3D11_BLEND_FACTOR_DST_COLOR = 9;
constexpr u32 D3D11_BLEND_FACTOR_INV_DST_COLOR = 10;
constexpr u32 D3D11_BLEND_FACTOR_SRC_ALPHA_SATURATE = 11;
constexpr u32 D3D11_BLEND_FACTOR_SRC1_COLOR = 16;
constexpr u32 D3D11_BLEND_FACTOR_INV_SRC1_COLOR = 17;
constexpr u32 D3D11_BLEND_FACTOR_SRC1_ALPHA = 18;
constexpr u32 D3D11_BLEND_FACTOR_INV_SRC1_ALPHA = 19;
// TODO: const color and alpha?

constexpr u32 GL_BLEND_FACTOR_BIT = 0x4000;

renderer::BlendFactor get_blend_factor(u32 blend_factor) {
    if (blend_factor & GL_BLEND_FACTOR_BIT) { // GL
        u32 gl_blend_factor = blend_factor & ~GL_BLEND_FACTOR_BIT;
        switch (gl_blend_factor) {
        case GL_ZERO:
            return renderer::BlendFactor::Zero;
        case GL_ONE:
            return renderer::BlendFactor::One;
        case GL_SRC_COLOR:
            return renderer::BlendFactor::SrcColor;
        case GL_ONE_MINUS_SRC_COLOR:
            return renderer::BlendFactor::InvSrcColor;
        case GL_SRC_ALPHA:
            return renderer::BlendFactor::SrcAlpha;
        case GL_ONE_MINUS_SRC_ALPHA:
            return renderer::BlendFactor::InvSrcAlpha;
        case GL_DST_ALPHA:
            return renderer::BlendFactor::DstAlpha;
        case GL_ONE_MINUS_DST_ALPHA:
            return renderer::BlendFactor::InvDstAlpha;
        case GL_DST_COLOR:
            return renderer::BlendFactor::DstColor;
        case GL_ONE_MINUS_DST_COLOR:
            return renderer::BlendFactor::InvDstColor;
        case GL_SRC_ALPHA_SATURATE:
            return renderer::BlendFactor::SrcAlphaSaturate;
        default:
            LOG_ERROR(Engines, "Unknown GL blend factor 0x{:04x}",
                      gl_blend_factor);
            return renderer::BlendFactor::One;
        }
    } else { // D3D11
        switch (blend_factor) {
        case D3D11_BLEND_FACTOR_ZERO:
            return renderer::BlendFactor::Zero;
        case D3D11_BLEND_FACTOR_ONE:
            return renderer::BlendFactor::One;
        case D3D11_BLEND_FACTOR_SRC_COLOR:
            return renderer::BlendFactor::SrcColor;
        case D3D11_BLEND_FACTOR_INV_SRC_COLOR:
            return renderer::BlendFactor::InvSrcColor;
        case D3D11_BLEND_FACTOR_SRC_ALPHA:
            return renderer::BlendFactor::SrcAlpha;
        case D3D11_BLEND_FACTOR_INV_SRC_ALPHA:
            return renderer::BlendFactor::InvSrcAlpha;
        case D3D11_BLEND_FACTOR_DST_ALPHA:
            return renderer::BlendFactor::DstAlpha;
        case D3D11_BLEND_FACTOR_INV_DST_ALPHA:
            return renderer::BlendFactor::InvDstAlpha;
        case D3D11_BLEND_FACTOR_DST_COLOR:
            return renderer::BlendFactor::DstColor;
        case D3D11_BLEND_FACTOR_INV_DST_COLOR:
            return renderer::BlendFactor::InvDstColor;
        case D3D11_BLEND_FACTOR_SRC_ALPHA_SATURATE:
            return renderer::BlendFactor::SrcAlphaSaturate;
        case D3D11_BLEND_FACTOR_SRC1_COLOR:
            return renderer::BlendFactor::Src1Color;
        case D3D11_BLEND_FACTOR_INV_SRC1_COLOR:
            return renderer::BlendFactor::InvSrc1Color;
        case D3D11_BLEND_FACTOR_SRC1_ALPHA:
            return renderer::BlendFactor::Src1Alpha;
        case D3D11_BLEND_FACTOR_INV_SRC1_ALPHA:
            return renderer::BlendFactor::InvSrc1Alpha;
        default:
            LOG_ERROR(Engines, "Unknown D3D11 blend factor 0x{:04x}",
                      blend_factor);
            return renderer::BlendFactor::One;
        }
    }
}

} // namespace

DEFINE_METHOD_TABLE(ThreeD, INLINE_ENGINE_TABLE, 0x45, 1,
                    LoadMmeInstructionRamPointer, u32, 0x46, 1,
                    LoadMmeInstructionRam, u32, 0x47, 1,
                    LoadMmeStartAddressRamPointer, u32, 0x48, 1,
                    LoadMmeStartAddressRam, u32, 0x35e, 1, DrawVertexArray, u32,
                    0x5f8, 1, DrawVertexElements, u32, 0x674, 1, ClearBuffer,
                    ClearBufferData, 0x6c3, 1, SetReportSemaphore, u32, 0x8c4,
                    1, FirmwareCall4, u32, 0x8e4, 16, LoadConstBuffer, u32,
                    0x900, 5 * 8, BindGroup, u32)

SINGLETON_DEFINE_GET_INSTANCE(ThreeD, Engines)

ThreeD::ThreeD() {
    SINGLETON_SET_INSTANCE(ThreeD, Engines);

    // TODO: choose based on Macro backend
    { macro_driver = new macro::interpreter::Driver(this); }

    // HACK
    regs.shader_programs[(u32)ShaderStage::VertexB].config.enable = true;
}

ThreeD::~ThreeD() {
    delete macro_driver;

    SINGLETON_UNSET_INSTANCE();
}

void ThreeD::FlushMacro() { macro_driver->Execute(); }

void ThreeD::Macro(u32 method, u32 arg) {
    u32 index = (method - MACRO_METHODS_REGION) >> 1;
    LOG_DEBUG(Engines, "Macro (index: 0x{:08x})", index);
    if ((method & 0x1) == 0x0) {
        LOG_DEBUG(Engines, "Parameter1: 0x{:08x}", arg);
        macro_driver->SetIndex(index);
        macro_driver->LoadParam1(arg);
    } else {
        LOG_DEBUG(Engines, "ParameterN: 0x{:08x}", arg);
        macro_driver->LoadParam(arg);
    }
}

void ThreeD::LoadMmeInstructionRamPointer(const u32 index, const u32 ptr) {
    macro_driver->LoadInstructionRamPointer(ptr);
}

void ThreeD::LoadMmeInstructionRam(const u32 index, const u32 data) {
    macro_driver->LoadInstructionRam(data);
}

void ThreeD::LoadMmeStartAddressRamPointer(const u32 index, const u32 ptr) {
    macro_driver->LoadStartAddressRamPointer(ptr);
}

void ThreeD::LoadMmeStartAddressRam(const u32 index, const u32 data) {
    macro_driver->LoadStartAddressRam(data);
}

void ThreeD::DrawVertexArray(const u32 index, u32 count) {
    if (!DrawInternal())
        return;

    auto index_type = IndexType::None;
    auto primitive_type = regs.begin.primitive_type;
    renderer::BufferBase* index_buffer =
        RENDERER_INSTANCE->GetIndexCache().Decode(
            {.type = index_type,
             .primitive_type = primitive_type,
             .count = count,
             .src_index_buffer = nullptr},
            index_type, primitive_type, count);
    if (index_buffer)
        RENDERER_INSTANCE->BindIndexBuffer(index_buffer, index_type);

    // TODO: instance count
    RENDERER_INSTANCE->Draw(primitive_type, regs.vertex_array_start, count, 0,
                            regs.base_instance, 1, index_buffer != nullptr);
}

void ThreeD::DrawVertexElements(const u32 index, u32 count) {
    if (!DrawInternal())
        return;

    // Index buffer
    gpu_vaddr_t index_buffer_ptr = UNMAP_ADDR(regs.index_buffer_addr);
    // TODO: uncomment?
    usize index_buffer_size =
        count * get_index_type_size(
                    regs.index_type); // MAKE_ADDR(regs.index_buffer_limit_addr)
                                      // - MAKE_ADDR(regs.index_buffer_addr);
    auto index_buffer = RENDERER_INSTANCE->GetBufferCache().Find(
        {index_buffer_ptr, index_buffer_size});

    auto index_type = regs.index_type;
    auto primitive_type = regs.begin.primitive_type;
    index_buffer = RENDERER_INSTANCE->GetIndexCache().Decode(
        {.type = index_type,
         .primitive_type = primitive_type,
         .count = count,
         .src_index_buffer = index_buffer},
        index_type, primitive_type, count);
    ASSERT_DEBUG(index_buffer, GPU, "Index buffer not found");
    RENDERER_INSTANCE->BindIndexBuffer(index_buffer, index_type);

    // Draw
    RENDERER_INSTANCE->Draw(primitive_type, regs.vertex_elements_start, count,
                            regs.base_vertex, regs.base_instance, 1, true);
}

void ThreeD::ClearBuffer(const u32 index, const ClearBufferData data) {
    LOG_DEBUG(GPU,
              "Depth: {}, stencil: {}, color mask: 0x{:x}, target id: {}, "
              "layer id: {}",
              data.depth, data.stencil, data.color_mask, data.target_id,
              data.layer_id);

    // Deferred clear
    // TODO: implement

    // Regular clear
    RENDERER_INSTANCE->BindRenderPass(GetRenderPass());

    if (data.color_mask != 0x0)
        RENDERER_INSTANCE->ClearColor(data.target_id, data.layer_id,
                                      data.color_mask, regs.clear_color);

    if (data.depth)
        RENDERER_INSTANCE->ClearDepth(data.layer_id, regs.clear_depth);

    if (data.stencil)
        RENDERER_INSTANCE->ClearStencil(data.layer_id, regs.clear_stencil);
}

void ThreeD::SetReportSemaphore(const u32 index, const u32 data) {
    ONCE(LOG_FUNC_STUBBED(Engines));

    const uptr ptr = UNMAP_ADDR(regs.report_semaphore_addr);

    // HACK
    *reinterpret_cast<u32*>(ptr) = regs.report_semaphore_payload;
}

void ThreeD::FirmwareCall4(const u32 index, const u32 data) {
    ONCE(LOG_FUNC_STUBBED(Engines));

    // TODO: find out what this does
    regs.mme_scratch[0] = 0x1;
}

void ThreeD::LoadConstBuffer(const u32 index, const u32 data) {
    const uptr const_buffer_gpu_addr = MAKE_ADDR(regs.const_buffer_selector);
    const uptr gpu_addr = const_buffer_gpu_addr + regs.load_const_buffer_offset;

    GPU::GetInstance().GetGPUMMU().Store(gpu_addr, data);

    regs.load_const_buffer_offset += sizeof(u32);
}

void ThreeD::BindGroup(const u32 index, const u32 data) {
    const auto shader_stage = static_cast<ShaderStage>(index / 0x8 + 1);
    const auto group = index % 0x8;

    switch (group) {
    case 0x0 ... 0x3:
        LOG_WARN(Engines, "Reserved");
        break;
    case 0x4: {
        const auto index = extract_bits<u32, 4, 5>(data);
        bool valid = data & 0x1;
        if (valid) {
            const uptr const_buffer_gpu_ptr =
                UNMAP_ADDR(regs.const_buffer_selector);

            const auto buffer = RENDERER_INSTANCE->GetBufferCache().Find(
                {const_buffer_gpu_ptr, regs.const_buffer_selector_size});

            bound_const_buffers[index] = const_buffer_gpu_ptr;
            RENDERER_INSTANCE->BindUniformBuffer(
                buffer, to_renderer_shader_type(shader_stage), index);
        } else {
            bound_const_buffers[index] = 0x0;
            RENDERER_INSTANCE->BindUniformBuffer(
                nullptr, to_renderer_shader_type(shader_stage), index);
        }
        break;
    }
    default:
        LOG_WARN(Engines, "Unknown group {}", group);
        break;
    }
}

renderer::BufferBase* ThreeD::GetVertexBuffer(u32 vertex_array_index) const {
    const auto& vertex_array = regs.vertex_arrays[vertex_array_index];

    // HACK
    if (MAKE_ADDR(vertex_array.addr) == 0x0) {
        ONCE(LOG_ERROR(Engines, "Invalid vertex buffer"));
        return nullptr;
    }

    const renderer::BufferDescriptor descriptor{
        .ptr = UNMAP_ADDR(vertex_array.addr),
        .size = MAKE_ADDR(regs.vertex_array_limits[vertex_array_index]) -
                MAKE_ADDR(vertex_array.addr),
    };

    return RENDERER_INSTANCE->GetBufferCache().Find(descriptor);
}

renderer::TextureBase*
ThreeD::GetTexture(const TextureImageControl& tic) const {
    // HACK
    if (tic.hdr_version == TicHdrVersion::_1DBuffer) {
        LOG_ERROR(Engines, "1D buffer");
        return nullptr;
    }

    const uptr gpu_addr = make_addr(tic.addr_lo, tic.addr_hi);
    if (gpu_addr == 0x0) {
        LOG_ERROR(Engines, "Texture address is NULL");
        return nullptr;
    }

    NvKind kind;
    switch (tic.hdr_version) {
    case TicHdrVersion::Pitch:
        kind = NvKind::Pitch;
        break;
    case TicHdrVersion::BlockLinear:
        kind = NvKind::Generic_16BX2;
        break;
    default:
        LOG_NOT_IMPLEMENTED(Engines, "TIC HDR version {}", tic.hdr_version);
        kind = NvKind::Pitch;
        break;
    }

    const auto format = renderer::to_texture_format(tic.format_word);
    const renderer::TextureDescriptor descriptor(
        GPU::GetInstance().GetGPUMMU().UnmapAddr(gpu_addr), format, kind,
        static_cast<usize>(tic.width_minus_one + 1),
        static_cast<usize>(tic.height_minus_one + 1),
        tic.tile_height_gobs_log2, // TODO: correct?
        get_texture_format_stride(format, tic.width_minus_one + 1),
        {tic.format_word.swizzle_x, tic.format_word.swizzle_y,
         tic.format_word.swizzle_z, tic.format_word.swizzle_w});

    return RENDERER_INSTANCE->GetTextureCache().GetTextureView(descriptor);
}

renderer::SamplerBase*
ThreeD::GetSampler(const TextureSamplerControl& tsc) const {
    const renderer::SamplerDescriptor descriptor{
        .min_filter = static_cast<renderer::SamplerFilter>(tsc.min_filter),
        .mag_filter = static_cast<renderer::SamplerFilter>(tsc.mag_filter),
        .mip_filter = static_cast<renderer::SamplerMipFilter>(tsc.mip_filter),
        .address_mode_r =
            static_cast<renderer::SamplerAddressMode>(tsc.address_u),
        .address_mode_s =
            static_cast<renderer::SamplerAddressMode>(tsc.address_v),
        .address_mode_t =
            static_cast<renderer::SamplerAddressMode>(tsc.address_p),
    };

    return RENDERER_INSTANCE->GetSamplerCache().Find(descriptor);
}

renderer::TextureBase*
ThreeD::GetColorTargetTexture(u32 render_target_index) const {
    const auto& render_target = regs.color_targets[render_target_index];

    const auto gpu_addr = MAKE_ADDR(render_target.addr);
    if (gpu_addr == 0x0) {
        // TODO: is this really an error?
        LOG_ERROR(Engines, "Invalid color render target at index {}",
                  render_target_index);
        return nullptr;
    }

    const auto format = renderer::to_texture_format(render_target.format);
    const renderer::TextureDescriptor descriptor(
        GPU::GetInstance().GetGPUMMU().UnmapAddr(gpu_addr), format,
        NvKind::Pitch, // TODO: correct?
        render_target.width, render_target.height,
        0, // TODO
        get_texture_format_stride(format, render_target.width));

    return RENDERER_INSTANCE->GetTextureCache().GetTextureView(descriptor);
}

renderer::TextureBase* ThreeD::GetDepthStencilTargetTexture() const {
    const auto gpu_addr = MAKE_ADDR(regs.depth_target_addr);
    if (gpu_addr == 0x0) {
        // TODO: is this really an error?
        LOG_ERROR(Engines, "Invalid depth render target");
        return nullptr;
    }

    const auto format = renderer::to_texture_format(regs.depth_target_format);
    const renderer::TextureDescriptor descriptor(
        GPU::GetInstance().GetGPUMMU().UnmapAddr(gpu_addr), format,
        NvKind::Pitch, // TODO: correct?
        regs.depth_target_width, regs.depth_target_height,
        0, // TODO
        get_texture_format_stride(format, regs.depth_target_width));

    return RENDERER_INSTANCE->GetTextureCache().GetTextureView(descriptor);
}

renderer::RenderPassBase* ThreeD::GetRenderPass() const {
    renderer::RenderPassDescriptor descriptor{};

    // Color targets
    for (u32 i = 0; i < regs.color_target_control.count; i++) {
        // TODO: use map0...7 for swizzling?
        descriptor.color_targets[i] = {
            .texture = GetColorTargetTexture(i),
        };
    }

    // Depth stencil target
    descriptor.depth_stencil_target = {
        .texture = (regs.depth_target_enabled ? GetDepthStencilTargetTexture()
                                              : nullptr),
    };

    return RENDERER_INSTANCE->GetRenderPassCache().Find(descriptor);
}

renderer::ShaderBase* ThreeD::GetShaderUnchecked(ShaderStage stage) const {
    return active_shaders[u32(to_renderer_shader_type(stage))];
}

renderer::ShaderBase* ThreeD::GetShader(ShaderStage stage) {
    const auto& program = regs.shader_programs[usize(stage)];
    if (!program.config.enable)
        return nullptr;

    uptr gpu_addr = MAKE_ADDR(regs.shader_program_region) + program.offset;
    uptr ptr = GPU::GetInstance().GetGPUMMU().UnmapAddr(gpu_addr);

    renderer::GuestShaderDescriptor descriptor{
        .stage = stage,
        .code_ptr = ptr,
    };

    // Vertex attribute states
    for (u32 i = 0; i < VERTEX_ATTRIB_COUNT; i++) {
        descriptor.state.vertex_attrib_states[i] = regs.vertex_attrib_states[i];
    }

    // Color target formats
    for (u32 i = 0; i < COLOR_TARGET_COUNT; i++) {
        const auto& render_target = regs.color_targets[i];
        const auto addr = MAKE_ADDR(render_target.addr);
        if (addr == 0x0)
            continue;

        descriptor.state.color_target_formats[i] =
            renderer::to_texture_format(regs.color_targets[i].format);
    }

    auto& active_shader = active_shaders[u32(to_renderer_shader_type(stage))];
    active_shader = RENDERER_INSTANCE->GetShaderCache().Find(descriptor);

    return active_shader;
}

renderer::PipelineBase* ThreeD::GetPipeline() {
    renderer::PipelineDescriptor descriptor{};

    // Shaders
    // TODO: add all shaders
    descriptor.shaders[u32(renderer::ShaderType::Vertex)] =
        GetShader(ShaderStage::VertexB);
    descriptor.shaders[u32(renderer::ShaderType::Fragment)] =
        GetShader(ShaderStage::Fragment);

    // Vertex state

    // Vertex attribute states
    for (u32 i = 0; i < VERTEX_ATTRIB_COUNT; i++) {
        descriptor.vertex_state.vertex_attrib_states[i] =
            regs.vertex_attrib_states[i];
    }

    // Vertex arrays
    for (u32 i = 0; i < VERTEX_ARRAY_COUNT; i++) {
        const auto& vertex_array = regs.vertex_arrays[i];
        descriptor.vertex_state.vertex_arrays[i] = {
            .enable = vertex_array.config.enable,
            .stride = vertex_array.config.stride,
            .is_per_instance =
                static_cast<bool>(regs.is_vertex_array_per_instance[i]),
            .divisor = vertex_array.divisor,
        };
    }

    // Color targets
    ASSERT_DEBUG(!regs.advanced_blend_enabled, Engines,
                 "Advanced blending not implemented");

    for (u32 i = 0; i < COLOR_TARGET_COUNT; i++) {
        auto& color_target = descriptor.color_target_states[i];
        color_target.format =
            renderer::to_texture_format(regs.color_targets[i].format);
        color_target.blend_enabled =
            static_cast<bool>(regs.color_blend_enabled[i]);
        if (color_target.blend_enabled) {
            if (regs.independent_blend_enabled) {
                const auto& blend_state = regs.independent_blend_state[i];
                color_target.rgb_op = get_blend_operation(blend_state.rgb_op);
                color_target.src_rgb_factor =
                    get_blend_factor(blend_state.src_rgb_factor);
                color_target.dst_rgb_factor =
                    get_blend_factor(blend_state.dst_rgb_factor);
                color_target.alpha_op =
                    get_blend_operation(blend_state.alpha_op);
                color_target.src_alpha_factor =
                    get_blend_factor(blend_state.src_alpha_factor);
                color_target.dst_alpha_factor =
                    get_blend_factor(blend_state.dst_alpha_factor);
            } else {
                const auto& blend_state = regs.blend_state;
                color_target.rgb_op = get_blend_operation(blend_state.rgb_op);
                color_target.src_rgb_factor =
                    get_blend_factor(blend_state.src_rgb_factor);
                color_target.dst_rgb_factor =
                    get_blend_factor(blend_state.dst_rgb_factor);
                color_target.alpha_op =
                    get_blend_operation(blend_state.alpha_op);
                color_target.src_alpha_factor =
                    get_blend_factor(blend_state.src_alpha_factor);
                color_target.dst_alpha_factor =
                    get_blend_factor(blend_state.dst_alpha_factor);
            }
        }
    }

    return RENDERER_INSTANCE->GetPipelineCache().Find(descriptor);
}

void ThreeD::ConfigureShaderStage(
    const ShaderStage stage, const TextureImageControl* tex_header_pool,
    const TextureSamplerControl* tex_sampler_pool) {
    const u32 stage_index = static_cast<u32>(stage) -
                            1; // 1 is subtracted, because VertexA is skipped

    const auto shader = GetShaderUnchecked(stage);
    const auto& resource_mapping = shader->GetDescriptor().resource_mapping;

    // TODO: how are uniform buffers handled?
    // TODO: storage buffers

    // Textures
    RENDERER_INSTANCE->UnbindTextures(to_renderer_shader_type(stage));
    auto tex_const_buffer = reinterpret_cast<const u32*>(
        bound_const_buffers[regs.bindless_texture_const_buffer_slot]);
    for (const auto [const_buffer_index, renderer_index] :
         resource_mapping.textures) {
        const auto texture_handle = tex_const_buffer[const_buffer_index];

        // Image
        const auto image_handle = get_image_handle(texture_handle);
        const auto& tic = tex_header_pool[image_handle];
        const auto texture = GetTexture(tic);

        // Sampler
        const auto sampler_handle = get_sampler_handle(texture_handle);
        const auto& tsc = tex_sampler_pool[sampler_handle];
        const auto sampler = GetSampler(tsc);

        if (texture && sampler)
            RENDERER_INSTANCE->BindTexture(texture, sampler,
                                           to_renderer_shader_type(stage),
                                           renderer_index);
        // TODO: else bind null texture
    }

    // TODO: images
}

bool ThreeD::DrawInternal() {
    if (!regs.shader_programs[(u32)ShaderStage::VertexB].config.enable) {
        LOG_WARN(Engines, "Vertex B stage not enabled, skipping draw");
        return false;
    }

    RENDERER_INSTANCE->BindRenderPass(GetRenderPass());

    RENDERER_INSTANCE->BindPipeline(GetPipeline());
    // TODO: viewport and scissor

    for (u32 i = 0; i < VERTEX_ARRAY_COUNT; i++) {
        const auto& vertex_array = regs.vertex_arrays[i];
        if (!vertex_array.config.enable)
            continue;

        const auto buffer = GetVertexBuffer(i);
        if (!buffer)
            continue;

        RENDERER_INSTANCE->BindVertexBuffer(buffer, i);
    }

    // Configure stages
    const auto tex_header_pool_gpu_addr = MAKE_ADDR(regs.tex_header_pool);
    const auto tex_sampler_pool_gpu_addr = MAKE_ADDR(regs.tex_sampler_pool);
    // TODO: remove the condition
    if (tex_header_pool_gpu_addr != 0x0 && tex_sampler_pool_gpu_addr != 0x0) {
        const auto tex_header_pool = reinterpret_cast<TextureImageControl*>(
            GPU::GetInstance().GetGPUMMU().UnmapAddr(tex_header_pool_gpu_addr));
        const auto tex_sampler_pool = reinterpret_cast<TextureSamplerControl*>(
            GPU::GetInstance().GetGPUMMU().UnmapAddr(
                tex_sampler_pool_gpu_addr));

        // TODO: configure all stages
        ConfigureShaderStage(ShaderStage::VertexB, tex_header_pool,
                             tex_sampler_pool);
        ConfigureShaderStage(ShaderStage::Fragment, tex_header_pool,
                             tex_sampler_pool);
    }

    return true;
}

} // namespace hydra::hw::tegra_x1::gpu::engines
