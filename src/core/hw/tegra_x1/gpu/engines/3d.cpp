#include "core/hw/tegra_x1/gpu/engines/3d.hpp"

#include "core/hw/tegra_x1/cpu/mmu.hpp"
#include "core/hw/tegra_x1/gpu/gpu.hpp"
#include "core/hw/tegra_x1/gpu/macro/interpreter/driver.hpp"
#include "core/hw/tegra_x1/gpu/renderer/buffer_base.hpp"
#include "core/hw/tegra_x1/gpu/renderer/render_pass_base.hpp"
#include "core/hw/tegra_x1/gpu/renderer/sampler_base.hpp"
#include "core/hw/tegra_x1/gpu/renderer/shader_base.hpp"
#include "core/hw/tegra_x1/gpu/renderer/texture_base.hpp"

namespace hydra::hw::tegra_x1::gpu::engines {

namespace {

u32 get_image_handle(u32 handle) { return extract_bits(handle, 0, 20); }
u32 get_sampler_handle(u32 handle) { return extract_bits(handle, 20, 12); }

renderer::TextureType ToTextureType(TextureType type) {
    switch (type) {
    case TextureType::_1D:
        return renderer::TextureType::_1D;
    case TextureType::_1DArray:
        return renderer::TextureType::_1DArray;
    case TextureType::_1DBuffer:
        return renderer::TextureType::_1DBuffer;
    case TextureType::_2D:
    case TextureType::_2DNoMipmap:
        return renderer::TextureType::_2D;
    case TextureType::_2DArray:
        return renderer::TextureType::_2DArray;
    case TextureType::_3D:
        return renderer::TextureType::_3D;
    case TextureType::Cubemap:
        return renderer::TextureType::Cube;
    case TextureType::CubeArray:
        return renderer::TextureType::CubeArray;
    }
}

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
        ONCE(LOG_ERROR(Engines, "Unknown blend operation 0x{:04x}", blend_op));
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
            ONCE(LOG_ERROR(Engines, "Unknown GL blend factor 0x{:04x}",
                           gl_blend_factor));
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
            ONCE(LOG_ERROR(Engines, "Unknown D3D11 blend factor 0x{:04x}",
                           blend_factor));
            return renderer::BlendFactor::One;
        }
    }
}

// Render target width is aligned to the stride, lets try to figure out the real
// one
u32 GetMinimumWidth(u32 width, renderer::TextureFormat format, u32 width_hint,
                    bool is_linear) {
    if (is_linear || width <= width_hint)
        return width;

    // Get the smallest width that would still align up to the same GOB
    // count
    const auto bpp = renderer::get_texture_format_bpp(format);
    const auto alignment = 64 / bpp;
    const auto width_aligned = align(width, alignment);
    // HACK
    // return std::clamp(width_aligned - alignment + 1, width_hint,
    // width_aligned);
    if (width_aligned - alignment + 1 <= width_hint)
        return width_hint;
    return width;
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
    {
        macro_driver = new macro::interpreter::Driver(this);
    }

    // Initialize default state

    // Viewports
    // TODO: correct?
    for (u32 i = 0; i < VIEWPORT_COUNT; i++) {
        regs.viewport_transforms[i].swizzle = {
            .x = ViewportSwizzle::PositiveX,
            .y = ViewportSwizzle::PositiveY,
            .z = ViewportSwizzle::PositiveZ,
            .w = ViewportSwizzle::PositiveW,
        };
    }

    // Color write masks
    // TODO: correct?
    for (u32 i = 0; i < COLOR_TARGET_COUNT; i++)
        regs.color_write_masks[i] = ColorWriteMask::All;

    // HACK
    regs.shader_programs[(u32)ShaderStage::VertexB].config.enable = true;
}

ThreeD::~ThreeD() {
    delete macro_driver;

    SINGLETON_UNSET_INSTANCE();
}

void ThreeD::FlushMacro(GMmu& gmmu) { macro_driver->Execute(gmmu); }

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

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"

void ThreeD::LoadMmeInstructionRamPointer(GMmu& gmmu, const u32 index,
                                          const u32 ptr) {
    macro_driver->LoadInstructionRamPointer(ptr);
}

void ThreeD::LoadMmeInstructionRam(GMmu& gmmu, const u32 index,
                                   const u32 data) {
    macro_driver->LoadInstructionRam(data);
}

void ThreeD::LoadMmeStartAddressRamPointer(GMmu& gmmu, const u32 index,
                                           const u32 ptr) {
    macro_driver->LoadStartAddressRamPointer(ptr);
}

void ThreeD::LoadMmeStartAddressRam(GMmu& gmmu, const u32 index,
                                    const u32 data) {
    macro_driver->LoadStartAddressRam(data);
}

void ThreeD::DrawVertexArray(GMmu& gmmu, const u32 index, u32 count) {
    if (!DrawInternal(gmmu))
        return;

    auto index_type = IndexType::None;
    auto primitive_type = regs.begin.primitive_type;
    const auto index_buffer = RENDERER_INSTANCE.GetIndexCache().Decode(
        {.type = index_type, .primitive_type = primitive_type, .count = count},
        index_type, primitive_type, count);

    if (index_buffer.GetBase()) {
        // Bind index buffer
        RENDERER_INSTANCE.BindIndexBuffer(index_buffer, index_type);

        // Draw

        // Vertex start is set as vertex base instead, as start is now index
        // start
        // TODO: instance count
        RENDERER_INSTANCE.DrawIndexed(primitive_type, 0, count,
                                      regs.vertex_array_start,
                                      regs.base_instance, 1);
    } else {
        // Draw
        // TODO: instance count
        RENDERER_INSTANCE.Draw(primitive_type, regs.vertex_array_start, count,
                               regs.base_instance, 1);
    }
}

void ThreeD::DrawVertexElements(GMmu& gmmu, const u32 index, u32 count) {
    if (!DrawInternal(gmmu))
        return;

    // Index buffer
    gpu_vaddr_t index_buffer_ptr = gmmu.UnmapAddr(regs.index_buffer_addr);
    // TODO: uncomment?
    u32 index_buffer_size =
        count * get_index_type_size(
                    regs.index_type); // u64(regs.index_buffer_limit_addr) + 1
                                      // - u64(regs.index_buffer_addr);
    const auto range =
        Range<uptr>::FromSize(index_buffer_ptr, index_buffer_size);

    auto index_type = regs.index_type;
    auto primitive_type = regs.begin.primitive_type;
    const auto index_buffer = RENDERER_INSTANCE.GetIndexCache().Decode(
        {.type = index_type,
         .primitive_type = primitive_type,
         .count = count,
         .mem_range = range},
        index_type, primitive_type, count);

    // Bind index buffer
    ASSERT_DEBUG(index_buffer.GetBase(), Gpu, "Index buffer not found");
    RENDERER_INSTANCE.BindIndexBuffer(index_buffer, index_type);

    // Draw
    // TODO: instance count
    RENDERER_INSTANCE.DrawIndexed(primitive_type, regs.vertex_elements_start,
                                  count, regs.base_vertex, regs.base_instance,
                                  1);
}

void ThreeD::ClearBuffer(GMmu& gmmu, const u32 index,
                         const ClearBufferData data) {
    LOG_DEBUG(Gpu,
              "Depth: {}, stencil: {}, color mask: 0x{:x}, target id: {}, "
              "layer id: {}",
              data.depth, data.stencil, data.color_mask, data.target_id,
              data.layer_id);

    // Deferred clear
    // TODO: implement

    // Regular clear
    RENDERER_INSTANCE.BindRenderPass(GetRenderPass(gmmu));

    if (data.color_mask != 0x0)
        RENDERER_INSTANCE.ClearColor(data.target_id, data.layer_id,
                                     data.color_mask, regs.clear_color);

    if (data.depth)
        RENDERER_INSTANCE.ClearDepth(data.layer_id, regs.clear_depth);

    if (data.stencil)
        RENDERER_INSTANCE.ClearStencil(data.layer_id, regs.clear_stencil);
}

void ThreeD::SetReportSemaphore(GMmu& gmmu, const u32 index, const u32 data) {
    ONCE(LOG_FUNC_STUBBED(Engines));

    const uptr ptr = gmmu.UnmapAddr(regs.report_semaphore_addr);

    // HACK
    *reinterpret_cast<u32*>(ptr) = regs.report_semaphore_payload;
}

void ThreeD::FirmwareCall4(GMmu& gmmu, const u32 index, const u32 data) {
    ONCE(LOG_FUNC_STUBBED(Engines));

    // TODO: find out what this does
    regs.mme_scratch[0] = 0x1;
}

void ThreeD::LoadConstBuffer(GMmu& gmmu, const u32 index, const u32 data) {
    const uptr const_buffer_gpu_addr = u64(regs.const_buffer_selector);
    const uptr gpu_addr = const_buffer_gpu_addr + regs.load_const_buffer_offset;
    const auto ptr = gmmu.UnmapAddr(gpu_addr);

    *reinterpret_cast<u32*>(ptr) = data;
    regs.load_const_buffer_offset += sizeof(u32);

    // Invalidate
    // TODO: invalidate as a whole
    RENDERER_INSTANCE.InvalidateMemory(Range<uptr>::FromSize(ptr, sizeof(u32)));
}

void ThreeD::BindGroup(GMmu& gmmu, const u32 index, const u32 data) {
    const auto shader_stage_index = index / 0x8;
    const auto group = index % 0x8;

    switch (group) {
    case 0x0 ... 0x3:
        LOG_WARN(Engines, "Reserved");
        break;
    case 0x4: {
        const auto index = extract_bits(data, 4, 5);
        bool valid = data & 0x1;
        if (valid) {
            const uptr const_buffer_gpu_ptr =
                gmmu.UnmapAddr(regs.const_buffer_selector);

            const auto range = Range<uptr>::FromSize(
                const_buffer_gpu_ptr, regs.const_buffer_selector_size);
            bound_const_buffers[shader_stage_index][index] = range;
        } else {
            bound_const_buffers[shader_stage_index][index] = Range<uptr>();
        }
        break;
    }
    default:
        LOG_WARN(Engines, "Unknown group {}", group);
        break;
    }
}

#pragma GCC diagnostic pop

renderer::TextureBase*
ThreeD::GetColorTargetTexture(GMmu& gmmu, u32 render_target_index) const {
    const auto& render_target = regs.color_targets[render_target_index];

    const auto gpu_addr = u64(render_target.addr);
    if (gpu_addr == 0x0) {
        // TODO: is this really an error?
        LOG_ERROR(Engines, "Invalid color render target at index {}",
                  render_target_index);
        return nullptr;
    }

    const auto format = renderer::to_texture_format(render_target.format);
    const u32 width_hint =
        regs.screen_scissor.horizontal.x + regs.screen_scissor.horizontal.width;
    const renderer::TextureDescriptor descriptor(
        gmmu.UnmapAddr(gpu_addr), renderer::TextureType::_2D, format,
        NvKind::Pitch, // TODO: correct?
        GetMinimumWidth(render_target.width, format, width_hint,
                        render_target.tile_mode.is_linear),
        render_target.height, 1,
        0, // TODO
        get_texture_format_stride(format, render_target.width));

    return RENDERER_INSTANCE.GetTextureCache().Find(
        descriptor, renderer::TextureUsage::Write);
}

renderer::TextureBase* ThreeD::GetDepthStencilTargetTexture(GMmu& gmmu) const {
    const auto gpu_addr = u64(regs.depth_target_addr);
    if (gpu_addr == 0x0) {
        // TODO: is this really an error?
        LOG_ERROR(Engines, "Invalid depth render target");
        return nullptr;
    }

    const auto format = renderer::to_texture_format(regs.depth_target_format);
    const u32 width_hint =
        regs.screen_scissor.horizontal.x + regs.screen_scissor.horizontal.width;
    const renderer::TextureDescriptor descriptor(
        gmmu.UnmapAddr(gpu_addr), renderer::TextureType::_2D, format,
        NvKind::Pitch, // TODO: correct?
        GetMinimumWidth(regs.depth_target_width, format, width_hint, false),
        regs.depth_target_height, 1,
        0, // TODO
        get_texture_format_stride(format, regs.depth_target_width));

    return RENDERER_INSTANCE.GetTextureCache().Find(
        descriptor, renderer::TextureUsage::Write);
}

renderer::RenderPassBase* ThreeD::GetRenderPass(GMmu& gmmu) const {
    renderer::RenderPassDescriptor descriptor{};

    // Color targets
    for (u32 i = 0; i < regs.color_target_control.count; i++) {
        descriptor.color_targets[i] = {
            .texture = GetColorTargetTexture(
                gmmu, regs.color_target_control.GetMap(i)),
        };
    }

    // Depth stencil target
    descriptor.depth_stencil_target = {
        .texture =
            (regs.depth_target_enabled ? GetDepthStencilTargetTexture(gmmu)
                                       : nullptr),
    };

    return RENDERER_INSTANCE.GetRenderPassCache().Find(descriptor);
}

renderer::Viewport ThreeD::GetViewport(u32 index) {
    renderer::Viewport res;

    const auto& extent = REGS_3D.viewports[index];
    const auto& transform = REGS_3D.viewport_transforms[index];
    if (/*REGS_3D.viewport_transform_enabled*/ true) { // HACK
        auto scale_x = transform.scale_x;
        auto scale_y = transform.scale_y;
        if (any(REGS_3D.window_origin_flags &
                engines::WindowOriginFlags::LowerLeft))
            scale_y = -scale_y;

        // Swizzle
        // TODO: check for viewport swizzle support
        if (transform.swizzle.x == engines::ViewportSwizzle::NegativeX)
            scale_x = -scale_x;
        else
            ASSERT_DEBUG(transform.swizzle.x ==
                             engines::ViewportSwizzle::PositiveX,
                         Engines, "Unsupported X viewport swizzle {}",
                         transform.swizzle.x);
        if (transform.swizzle.y == engines::ViewportSwizzle::NegativeY)
            scale_y = -scale_y;
        else
            ASSERT_DEBUG(transform.swizzle.y ==
                             engines::ViewportSwizzle::PositiveY,
                         Engines, "Unsupported Y viewport swizzle {}",
                         transform.swizzle.y);
        ASSERT_DEBUG(transform.swizzle.z == engines::ViewportSwizzle::PositiveZ,
                     Engines, "Unsupported Z viewport swizzle {}",
                     transform.swizzle.z);
        ASSERT_DEBUG(transform.swizzle.w == engines::ViewportSwizzle::PositiveW,
                     Engines, "Unsupported W viewport swizzle {}",
                     transform.swizzle.w);

        res.rect.origin.x() = transform.offset_x - scale_x;
        res.rect.origin.y() = transform.offset_y - scale_y;
        res.rect.size.x() = scale_x * 2.0f;
        res.rect.size.y() = scale_y * 2.0f;
        // TODO: Z scale and offset
        res.depth_near = extent.near;
        res.depth_far = extent.far;
    } else {
        const auto& screen_scissor = REGS_3D.screen_scissor;
        res.rect.origin.x() = screen_scissor.horizontal.x;
        res.rect.origin.y() = screen_scissor.vertical.y;
        res.rect.size.x() = screen_scissor.horizontal.width;
        res.rect.size.y() = screen_scissor.vertical.height;
        res.depth_near = extent.near;
        res.depth_far = extent.far;
    }

    // Flip Y
    res.rect.origin.y() += res.rect.size.y();
    res.rect.size.y() = -res.rect.size.y();

    // HACK: if depth range is [0, 0], force it to [0, 1] (many games have
    // it like this, though not on Ryujinx)
    if (res.depth_near == 0.0 && res.depth_far == 0.0) {
        ONCE(LOG_WARN(Engines, "Depth range is [0, 0], forcing to [0, 1]"));
        res.depth_near = 0.0;
        res.depth_far = 1.0;
    }

    return res;
}

renderer::Scissor ThreeD::GetScissor(u32 index) {
    const auto& scissor = REGS_3D.scissors[index];
    if (scissor.enabled) {
        return renderer::Scissor(
            uint2({scissor.horizontal.min, scissor.vertical.min}),
            uint2({static_cast<u32>(scissor.horizontal.max -
                                    scissor.horizontal.min),
                   static_cast<u32>(scissor.vertical.max -
                                    scissor.vertical.min)}));
    } else {
        return renderer::Scissor(uint2({0, 0}), uint2({0xffff, 0xffff}));
    }
}

renderer::ShaderBase* ThreeD::GetShaderUnchecked(ShaderStage stage) const {
    return active_shaders[u32(to_renderer_shader_type(stage))];
}

renderer::ShaderBase* ThreeD::GetShader(GMmu& gmmu, ShaderStage stage) {
    const auto& program = regs.shader_programs[usize(stage)];
    if (!program.config.enable)
        return nullptr;

    uptr gpu_addr = u64(regs.shader_program_region) + program.offset;
    uptr ptr = gmmu.UnmapAddr(gpu_addr);

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
        const auto addr = u64(render_target.addr);
        if (addr == 0x0)
            continue;

        descriptor.state.color_target_data_types[i] =
            renderer::to_color_data_type(regs.color_targets[i].format);
    }

    auto& active_shader = active_shaders[u32(to_renderer_shader_type(stage))];
    active_shader = RENDERER_INSTANCE.GetShaderCache().Find(descriptor);

    return active_shader;
}

renderer::PipelineBase* ThreeD::GetPipeline(GMmu& gmmu) {
    renderer::PipelineDescriptor descriptor{};

    // Shaders
    // TODO: add all shaders
    descriptor.shaders[u32(renderer::ShaderType::Vertex)] =
        GetShader(gmmu, ShaderStage::VertexB);
    descriptor.shaders[u32(renderer::ShaderType::Fragment)] =
        GetShader(gmmu, ShaderStage::Fragment);

    // Vertex state

    // Vertex attribute states
    bool vertex_arrays_used[VERTEX_ARRAY_COUNT] = {false};
    for (u32 i = 0; i < VERTEX_ATTRIB_COUNT; i++) {
        const auto& state = regs.vertex_attrib_states[i];
        descriptor.vertex_state.vertex_attrib_states[i] = state;

        // Set used vertex array
        if (state.type == engines::VertexAttribType::None)
            continue;

        // HACK: how are attributes disabled?
        if (state.is_fixed)
            continue;

        vertex_arrays_used[state.buffer_id] = true;
    }

    // Vertex arrays
    for (u32 i = 0; i < VERTEX_ARRAY_COUNT; i++) {
        if (!vertex_arrays_used[i])
            continue;

        const auto& vertex_array = regs.vertex_arrays[i];
        descriptor.vertex_state.vertex_arrays[i] = {
            .enable = static_cast<bool>(vertex_array.config.enable),
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
        color_target.write_mask = regs.color_write_masks[i];
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

    return RENDERER_INSTANCE.GetPipelineCache().Find(descriptor);
}

renderer::BufferView ThreeD::GetVertexBuffer(GMmu& gmmu,
                                             u32 vertex_array_index) const {
    const auto& vertex_array = regs.vertex_arrays[vertex_array_index];

    // HACK
    if (u64(vertex_array.addr) == 0x0) {
        ONCE(LOG_ERROR(Engines, "Invalid vertex buffer"));
        return renderer::BufferView();
    }

    const auto ptr = gmmu.UnmapAddr(vertex_array.addr);
    const auto size = u64(regs.vertex_array_limits[vertex_array_index]) + 1 -
                      u64(vertex_array.addr);
    return RENDERER_INSTANCE.GetBufferCache().Get(
        Range<uptr>::FromSize(ptr, size));
}

renderer::TextureBase*
ThreeD::GetTexture(GMmu& gmmu, const TextureImageControl& tic) const {
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

    const auto format =
        renderer::to_texture_format(tic.format_word, tic.is_srgb);

    NvKind kind;
    u32 stride;
    switch (tic.hdr_version) {
    case TicHdrVersion::Pitch:
        kind = NvKind::Pitch;
        stride = static_cast<u32>(tic.pitch_5_20) << 5u;
        break;
    case TicHdrVersion::BlockLinear:
        kind = NvKind::Generic_16BX2;
        // TODO: is the alignment correct?
        stride = align(
            get_texture_format_stride(format, tic.width_minus_one + 1), 64u);
        break;
    default:
        LOG_NOT_IMPLEMENTED(Engines, "TIC HDR version {}", tic.hdr_version);
        kind = NvKind::Pitch;
        stride = get_texture_format_stride(format, tic.width_minus_one + 1);
        break;
    }

    const renderer::TextureDescriptor descriptor(
        gmmu.UnmapAddr(gpu_addr), ToTextureType(tic.texture_type), format, kind,
        static_cast<u32>(tic.width_minus_one + 1),
        static_cast<u32>(tic.height_minus_one + 1),
        static_cast<u32>(tic.depth_minus_one + 1),
        tic.tile_height_gobs_log2, // TODO: correct?
        stride,
        renderer::SwizzleChannels(
            format, tic.format_word.swizzle_x, tic.format_word.swizzle_y,
            tic.format_word.swizzle_z, tic.format_word.swizzle_w));

    return RENDERER_INSTANCE.GetTextureCache().Find(
        descriptor, renderer::TextureUsage::Read);
}

renderer::SamplerBase*
ThreeD::GetSampler(const TextureSamplerControl& tsc) const {
    const renderer::SamplerDescriptor descriptor{
        .min_filter = static_cast<renderer::SamplerFilter>(tsc.min_filter),
        .mag_filter = static_cast<renderer::SamplerFilter>(tsc.mag_filter),
        .mip_filter = static_cast<renderer::SamplerMipFilter>(tsc.mip_filter),
        .address_mode_s =
            static_cast<renderer::SamplerAddressMode>(tsc.address_u),
        .address_mode_t =
            static_cast<renderer::SamplerAddressMode>(tsc.address_v),
        .address_mode_r =
            static_cast<renderer::SamplerAddressMode>(tsc.address_p),
        .depth_compare_op = (tsc.depth_compare ? static_cast<CompareOp>(
                                                     tsc.depth_compare_op + 1)
                                               : CompareOp::Invalid),
        .border_color_u = uint4({tsc.border_color_r, tsc.border_color_g,
                                 tsc.border_color_b, tsc.border_color_a}),
    };

    return RENDERER_INSTANCE.GetSamplerCache().Find(descriptor);
}

void ThreeD::ConfigureShaderStage(
    GMmu& gmmu, const ShaderStage stage,
    const TextureImageControl* tex_header_pool,
    const TextureSamplerControl* tex_sampler_pool) {
    const auto shader_type = to_renderer_shader_type(stage);
    const u32 stage_index = static_cast<u32>(stage) -
                            1; // 1 is subtracted, because VertexA is skipped

    const auto shader = GetShaderUnchecked(stage);
    const auto& resource_mapping = shader->GetDescriptor().resource_mapping;

    // Uniform buffers
    RENDERER_INSTANCE.UnbindUniformBuffers(shader_type);
    for (u32 i = 0; i < CONST_BUFFER_BINDING_COUNT; i++) {
        const auto index = resource_mapping.uniform_buffers[i];
        if (index == invalid<u32>())
            continue;

        // TODO: analyze the shader to get the max possible size
        const auto range = bound_const_buffers[stage_index][i];
        if (range.GetBegin() == 0x0) {
            LOG_WARN(Engines, "Uniform buffer at index {} is not bound", index);
            continue;
        }

        const auto buffer = RENDERER_INSTANCE.GetBufferCache().Get(range);
        RENDERER_INSTANCE.BindUniformBuffer(buffer, shader_type, index);
    }

    // TODO: storage buffers

    // Textures
    RENDERER_INSTANCE.UnbindTextures(shader_type);
    auto tex_const_buffer = reinterpret_cast<const u32*>(
        bound_const_buffers[stage_index]
                           [regs.bindless_texture_const_buffer_slot]
                               .GetBegin());
    for (const auto [const_buffer_index, renderer_index] :
         resource_mapping.textures) {
        const auto texture_handle = tex_const_buffer[const_buffer_index];

        // Image
        const auto image_handle = get_image_handle(texture_handle);
        const auto& tic = tex_header_pool[image_handle];
        const auto texture = GetTexture(gmmu, tic);

        // Sampler
        const auto sampler_handle = get_sampler_handle(texture_handle);
        const auto& tsc = tex_sampler_pool[sampler_handle];
        const auto sampler = GetSampler(tsc);

        if (texture && sampler)
            RENDERER_INSTANCE.BindTexture(texture, sampler, shader_type,
                                          renderer_index);
        // TODO: else bind null texture
    }
}

bool ThreeD::DrawInternal(GMmu& gmmu) {
    // Flush tracked pages
    gmmu.GetMmu()->FlushTrackedPages();

    // State
    if (!regs.shader_programs[(u32)ShaderStage::VertexB].config.enable) {
        LOG_WARN(Engines, "Vertex B stage not enabled, skipping draw");
        return false;
    }

    RENDERER_INSTANCE.BindRenderPass(GetRenderPass(gmmu));

    for (u32 i = 0; i < VIEWPORT_COUNT; i++) {
        RENDERER_INSTANCE.SetViewport(i, GetViewport(i));
        RENDERER_INSTANCE.SetScissor(i, GetScissor(i));
    }

    RENDERER_INSTANCE.BindPipeline(GetPipeline(gmmu));

    for (u32 i = 0; i < VERTEX_ARRAY_COUNT; i++) {
        const auto& vertex_array = regs.vertex_arrays[i];
        // HACK: Super Meat Boy contains invalid vertex arrays with address 4096
        if (!vertex_array.config.enable ||
            (vertex_array.addr.hi == 0 && vertex_array.addr.lo == 4096)) {
            RENDERER_INSTANCE.BindVertexBuffer(renderer::BufferView(), i);
            continue;
        }

        const auto buffer = GetVertexBuffer(gmmu, i);
        RENDERER_INSTANCE.BindVertexBuffer(buffer, i);
    }

    // Configure stages
    const auto tex_header_pool_gpu_addr = u64(regs.tex_header_pool);
    const auto tex_sampler_pool_gpu_addr = u64(regs.tex_sampler_pool);
    // TODO: remove the condition
    if (tex_header_pool_gpu_addr != 0x0 && tex_sampler_pool_gpu_addr != 0x0) {
        const auto tex_header_pool = reinterpret_cast<TextureImageControl*>(
            gmmu.UnmapAddr(tex_header_pool_gpu_addr));
        const auto tex_sampler_pool = reinterpret_cast<TextureSamplerControl*>(
            gmmu.UnmapAddr(tex_sampler_pool_gpu_addr));

        // TODO: configure all stages
        ConfigureShaderStage(gmmu, ShaderStage::VertexB, tex_header_pool,
                             tex_sampler_pool);
        ConfigureShaderStage(gmmu, ShaderStage::Fragment, tex_header_pool,
                             tex_sampler_pool);
    }

    return true;
}

} // namespace hydra::hw::tegra_x1::gpu::engines
