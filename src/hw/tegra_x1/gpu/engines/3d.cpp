#include "hw/tegra_x1/gpu/engines/3d.hpp"

#include "hw/tegra_x1/gpu/gpu.hpp"
#include "hw/tegra_x1/gpu/macro/interpreter/driver.hpp"
#include "hw/tegra_x1/gpu/renderer/buffer_base.hpp"
#include "hw/tegra_x1/gpu/renderer/render_pass_base.hpp"
#include "hw/tegra_x1/gpu/renderer/shader_base.hpp"
#include "hw/tegra_x1/gpu/renderer/texture_base.hpp"

namespace Hydra::HW::TegraX1::GPU::Engines {

namespace {

u32 get_image_handle(u32 handle) { return extract_bits<u32, 0, 20>(handle); }
u32 get_sampler_handle(u32 handle) { return extract_bits<u32, 20, 12>(handle); }

} // namespace

DEFINE_METHOD_TABLE(ThreeD, 0x45, 1, LoadMmeInstructionRamPointer, u32, 0x46, 1,
                    LoadMmeInstructionRam, u32, 0x47, 1,
                    LoadMmeStartAddressRamPointer, u32, 0x48, 1,
                    LoadMmeStartAddressRam, u32, 0x6c, 1, LaunchDMA, u32, 0x6d,
                    1, LoadInlineData, u32, 0x35e, 1, DrawVertexArray, u32,
                    0x5f8, 1, DrawVertexElements, u32, 0x674, 1, ClearBuffer,
                    ClearBufferData, 0x6c3, 1, SetReportSemaphore, u32, 0x8c4,
                    1, FirmwareCall4, u32, 0x8e4, 16, LoadConstBuffer, u32,
                    0x900, 5 * 8, BindGroup, u32)

SINGLETON_DEFINE_GET_INSTANCE(ThreeD, Engines, "3D engine")

ThreeD::ThreeD() {
    SINGLETON_SET_INSTANCE(Engines, "3D engine");

    // TODO: choose based on Macro backend
    {
        macro_driver = new Macro::Interpreter::Driver(this);
    }
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

void ThreeD::LaunchDMA(const u32 index, const u32 data) {
    // TODO: args
    LOG_FUNC_STUBBED(Engines);
}

void ThreeD::LoadInlineData(const u32 index, const u32 data) {
    inline_data.push_back(data);
    // TODO: correct?
    if (inline_data.size() * sizeof(u32) ==
        regs.inline_regs.line_length_in * regs.inline_regs.line_count) {
        // Flush
        // TODO: determine what type of copy this is based on launch DMA args

        // Buffer to buffer
        gpu_vaddr dst_ptr = UNMAP_ADDR(regs.inline_regs.offset_out);
        auto dst = RENDERER->GetBufferCache().Find(
            {dst_ptr, inline_data.size() * sizeof(u32)});

        dst->CopyFrom(inline_data.data());
    }
}

void ThreeD::DrawVertexArray(const u32 index, u32 count) {
    DrawInternal();

    auto primitive_type = regs.begin.primitive_type;
    Renderer::BufferBase* index_buffer{nullptr};
    if (!RENDERER->GetInfo().IsPrimitiveSupported(primitive_type)) {
        // TODO: move this into an index cache
        PrimitiveType target_primitive_type;
        usize index_count;
        switch (primitive_type) {
        case PrimitiveType::Quads:
            target_primitive_type = PrimitiveType::Triangles;
            index_count = count * 6 / 4;
            break;
        default:
            throw;
            break;
        }

        IndexType index_type;
        // TODO: also support UInt8 index type
        if (count <= 0x10000) {
            index_type = IndexType::UInt16;
        } else {
            index_type = IndexType::UInt32;
        }
        usize index_size = get_index_type_size(index_type);

        index_buffer =
            RENDERER->AllocateTemporaryBuffer(index_count * index_size);
        auto index_buffer_ptr = index_buffer->GetDescriptor().ptr;

#define ADD_INDEX(index)                                                       \
    switch (index_type) {                                                      \
    case IndexType::UInt8:                                                     \
        *reinterpret_cast<u8*>(index_buffer_ptr) = index;                      \
    case IndexType::UInt16:                                                    \
        *reinterpret_cast<u16*>(index_buffer_ptr) = index;                     \
    case IndexType::UInt32:                                                    \
        *reinterpret_cast<u32*>(index_buffer_ptr) = index;                     \
    }                                                                          \
    index_buffer_ptr += index_size;

        // Generate indices
        switch (primitive_type) {
        case PrimitiveType::Quads:
            for (u32 i = 0; i < count / 4; i++) {
                const u32 base = i * 4;
                const u32 base_index = i * 6;
                ADD_INDEX(base + 0);
                ADD_INDEX(base + 1);
                ADD_INDEX(base + 2);
                ADD_INDEX(base + 0);
                ADD_INDEX(base + 2);
                ADD_INDEX(base + 3);
            }
            break;
        default:
            throw;
            break;
        }

#undef ADD_INDEX

        RENDERER->BindIndexBuffer(index_buffer, index_type);

        primitive_type = target_primitive_type;
        count = index_count;
    }

    RENDERER->Draw(primitive_type, regs.vertex_array_start, count,
                   index_buffer != nullptr);

    if (index_buffer)
        RENDERER->FreeTemporaryBuffer(index_buffer);
}

void ThreeD::DrawVertexElements(const u32 index, u32 count) {
    DrawInternal();

    // Index buffer
    gpu_vaddr index_buffer_ptr = UNMAP_ADDR(regs.index_buffer_addr);
    // TODO: uncomment?
    usize index_buffer_size =
        count * get_index_type_size(
                    regs.index_type); // MAKE_ADDR(regs.index_buffer_limit_addr)
                                      // - MAKE_ADDR(regs.index_buffer_addr);

    auto index_buffer =
        RENDERER->GetBufferCache().Find({index_buffer_ptr, index_buffer_size});
    RENDERER->BindIndexBuffer(index_buffer, regs.index_type);

    // Draw
    RENDERER->Draw(regs.begin.primitive_type, regs.vertex_elements_start, count,
                   true);
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
    RENDERER->BindRenderPass(GetRenderPass());

    if (data.color_mask != 0x0)
        RENDERER->ClearColor(data.target_id, data.layer_id, data.color_mask,
                             regs.clear_color);

    if (data.depth)
        RENDERER->ClearDepth(data.layer_id, regs.clear_depth);

    if (data.stencil)
        RENDERER->ClearStencil(data.layer_id, regs.clear_stencil);
}

void ThreeD::SetReportSemaphore(const u32 index, const u32 data) {
    LOG_FUNC_STUBBED(Engines);

    const uptr ptr = UNMAP_ADDR(regs.report_semaphore_addr);

    // HACK
    *reinterpret_cast<u32*>(ptr) = regs.report_semaphore_payload;
}

void ThreeD::FirmwareCall4(const u32 index, const u32 data) {
    LOG_NOT_IMPLEMENTED(Engines, "Firmware call 4");

    // TODO: find out what this does
    regs.mme_firmware_args[0] = 0x1;
}

void ThreeD::LoadConstBuffer(const u32 index, const u32 data) {
    const uptr const_buffer_gpu_addr = MAKE_ADDR(regs.const_buffer_selector);
    const uptr gpu_addr = const_buffer_gpu_addr + regs.load_const_buffer_offset;
    // TODO: should the offset get
    // incremented?
    regs.load_const_buffer_offset += sizeof(u32);
    uptr ptr = GPU::GetInstance().GetGPUMMU().UnmapAddr(gpu_addr);

    *reinterpret_cast<u32*>(ptr) = data;
}

void ThreeD::BindGroup(const u32 index, const u32 data) {
    const auto shader_stage = static_cast<ShaderStage>(index / 0x8 + 1);
    const auto group = index % 0x8;

    switch (group) {
    case 0x0 ... 0x3:
        LOG_WARNING(Engines, "Reserved");
        break;
    case 0x4: {
        const auto index = extract_bits<u32, 4, 5>(data);
        bool valid = data & 0x1;
        if (valid) {
            const uptr const_buffer_gpu_ptr =
                UNMAP_ADDR(regs.const_buffer_selector);

            const auto buffer = RENDERER->GetBufferCache().Find(
                {const_buffer_gpu_ptr, regs.const_buffer_selector_size});

            RENDERER->BindUniformBuffer(
                buffer, to_renderer_shader_type(shader_stage), index);
        } else {
            RENDERER->BindUniformBuffer(
                nullptr, to_renderer_shader_type(shader_stage), index);
        }
        break;
    }
    default:
        LOG_WARNING(Engines, "Unknown group {}", group);
        break;
    }
}

Renderer::BufferBase* ThreeD::GetVertexBuffer(u32 vertex_array_index) const {
    const auto& vertex_array = regs.vertex_arrays[vertex_array_index];

    const Renderer::BufferDescriptor descriptor{
        .ptr = UNMAP_ADDR(vertex_array.addr),
        .size = MAKE_ADDR(regs.vertex_array_limits[vertex_array_index]) -
                MAKE_ADDR(vertex_array.addr),
    };

    return RENDERER->GetBufferCache().Find(descriptor);
}

Renderer::TextureBase*
ThreeD::GetTexture(const TextureImageControl& tic) const {
    const uptr gpu_addr = make_addr(tic.addr_lo, tic.addr_hi);
    if (gpu_addr == 0x0)
        return nullptr;

    const Renderer::TextureDescriptor descriptor{
        .ptr = GPU::GetInstance().GetGPUMMU().UnmapAddr(gpu_addr),
        .format = Renderer::to_texture_format(tic.format_word),
        .kind = NvKind::Pitch, // TODO: correct?
        .width = static_cast<usize>(tic.width_minus_one + 1),
        .height = static_cast<usize>(tic.height_minus_one + 1),
        .block_height_log2 = tic.tile_height_gobs_log2, // TODO: correct?
        .stride = static_cast<usize>((tic.width_minus_one + 1) * 4), // HACK
    };

    return RENDERER->GetTextureCache().GetTextureView(descriptor);
}

Renderer::TextureBase*
ThreeD::GetColorTargetTexture(u32 render_target_index) const {
    const auto& render_target = regs.color_targets[render_target_index];

    const auto gpu_addr = MAKE_ADDR(render_target.addr);
    if (gpu_addr == 0x0) {
        LOG_ERROR(Engines, "Invalid color render target at index {}",
                  render_target_index)
        return nullptr;
    }

    const Renderer::TextureDescriptor descriptor{
        .ptr = GPU::GetInstance().GetGPUMMU().UnmapAddr(gpu_addr),
        .format = Renderer::to_texture_format(render_target.format),
        .kind = NvKind::Pitch, // TODO: correct?
        .width = render_target.width,
        .height = render_target.height,
        .block_height_log2 = 0,            // TODO
        .stride = render_target.width * 4, // HACK
    };

    return RENDERER->GetTextureCache().GetTextureView(descriptor);
}

Renderer::TextureBase* ThreeD::GetDepthStencilTargetTexture() const {
    const auto gpu_addr = MAKE_ADDR(regs.depth_target_addr);
    if (gpu_addr == 0x0) {
        LOG_ERROR(Engines, "Invalid depth render target")
        return nullptr;
    }

    const Renderer::TextureDescriptor descriptor{
        .ptr = GPU::GetInstance().GetGPUMMU().UnmapAddr(gpu_addr),
        .format = Renderer::to_texture_format(regs.depth_target_format),
        .kind = NvKind::Pitch, // TODO: correct?
        .width = regs.depth_target_width,
        .height = regs.depth_target_height,
        .block_height_log2 = 0,                // TODO
        .stride = regs.depth_target_width * 4, // HACK
    };

    return RENDERER->GetTextureCache().GetTextureView(descriptor);
}

Renderer::RenderPassBase* ThreeD::GetRenderPass() const {
    Renderer::RenderPassDescriptor descriptor{};

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

    return RENDERER->GetRenderPassCache().Find(descriptor);
}

Renderer::ShaderBase* ThreeD::GetShaderUnchecked(ShaderStage stage) const {
    return active_shaders[u32(to_renderer_shader_type(stage))];
}

Renderer::ShaderBase* ThreeD::GetShader(ShaderStage stage) {
    const auto& program = regs.shader_programs[usize(stage)];
    if (!program.config.enable)
        return nullptr;

    uptr gpu_addr = MAKE_ADDR(regs.shader_program_region) + program.offset;
    uptr ptr = GPU::GetInstance().GetGPUMMU().UnmapAddr(gpu_addr);

    Renderer::GuestShaderDescriptor descriptor{
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
            Renderer::to_texture_format(regs.color_targets[i].format);
    }

    auto& active_shader = active_shaders[u32(to_renderer_shader_type(stage))];
    active_shader = RENDERER->GetShaderCache().Find(descriptor);

    return active_shader;
}

Renderer::PipelineBase* ThreeD::GetPipeline() {
    Renderer::PipelineDescriptor descriptor{};

    // Shaders
    // TODO: add all shaders
    descriptor.shaders[u32(Renderer::ShaderType::Vertex)] =
        GetShader(ShaderStage::VertexB);
    descriptor.shaders[u32(Renderer::ShaderType::Fragment)] =
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
            .is_per_instance = regs.is_vertex_array_per_instance[i].enable,
            .divisor = vertex_array.divisor,
        };
    }

    return RENDERER->GetPipelineCache().Find(descriptor);
}

void ThreeD::ConfigureShaderStage(const ShaderStage stage,
                                  const GraphicsDriverCbuf& const_buffer,
                                  const TextureImageControl* tex_header_pool) {
    const u32 stage_index = static_cast<u32>(stage) -
                            1; // 1 is subtracted, because VertexA is skipped

    const auto shader = GetShaderUnchecked(stage);
    const auto& resource_mapping = shader->GetDescriptor().resource_mapping;

    // TODO: how are uniform buffers handled?
    // TODO: storage buffers

    // Textures
    for (u32 i = 0; i < TEXTURE_BINDING_COUNT; i++) {
        if (resource_mapping.textures[i] == invalid<u32>()) {
            RENDERER->BindTexture(nullptr, to_renderer_shader_type(stage), i);
            continue;
        }

        const auto texture_handle = const_buffer.data[stage_index].textures[i];

        // Image
        const auto image_handle = get_image_handle(texture_handle);
        const auto& tic = tex_header_pool[image_handle];
        const auto texture = GetTexture(tic);
        if (texture)
            RENDERER->BindTexture(texture, to_renderer_shader_type(stage), i);
    }

    // TODO: images
}

void ThreeD::DrawInternal() {
    RENDERER->BindRenderPass(GetRenderPass());

    RENDERER->BindPipeline(GetPipeline());
    // TODO: viewport and scissor

    for (u32 i = 0; i < VERTEX_ARRAY_COUNT; i++) {
        const auto& vertex_array = regs.vertex_arrays[i];
        if (!vertex_array.config.enable)
            continue;

        const auto buffer = GetVertexBuffer(i);
        if (!buffer)
            continue;

        RENDERER->BindVertexBuffer(buffer, i);
    }

    // Constant buffer
    const uptr const_buffer_gpu_addr = MAKE_ADDR(regs.const_buffer_selector);
    if (const_buffer_gpu_addr != 0x0) {
        const auto const_buffer =
            GPU::GetInstance().GetGPUMMU().Load<GraphicsDriverCbuf>(
                const_buffer_gpu_addr);

        const auto tex_header_pool_gpu_addr = MAKE_ADDR(regs.tex_header_pool);
        if (tex_header_pool_gpu_addr != 0x0) {
            const auto tex_header_pool = reinterpret_cast<TextureImageControl*>(
                GPU::GetInstance().GetGPUMMU().UnmapAddr(
                    tex_header_pool_gpu_addr));

            // TODO: configure all stages
            ConfigureShaderStage(ShaderStage::VertexB, const_buffer,
                                 tex_header_pool);
            ConfigureShaderStage(ShaderStage::Fragment, const_buffer,
                                 tex_header_pool);
        }
    }
}

} // namespace Hydra::HW::TegraX1::GPU::Engines
