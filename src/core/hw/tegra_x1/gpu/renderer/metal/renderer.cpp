#include "core/hw/tegra_x1/gpu/renderer/metal/renderer.hpp"

#include "common/config.hpp"
#include "core/hw/tegra_x1/gpu/engines/3d.hpp"
#include "core/hw/tegra_x1/gpu/renderer/metal/buffer.hpp"
#include "core/hw/tegra_x1/gpu/renderer/metal/const.hpp"
#include "core/hw/tegra_x1/gpu/renderer/metal/maxwell_to_mtl.hpp"
#include "core/hw/tegra_x1/gpu/renderer/metal/pipeline.hpp"
#include "core/hw/tegra_x1/gpu/renderer/metal/render_pass.hpp"
#include "core/hw/tegra_x1/gpu/renderer/metal/sampler.hpp"
#include "core/hw/tegra_x1/gpu/renderer/metal/shader.hpp"
#include "core/hw/tegra_x1/gpu/renderer/metal/surface_compositor.hpp"
#include "core/hw/tegra_x1/gpu/renderer/metal/texture.hpp"

// TODO: define in a separate file
/*
const std::string utility_shader_source = R"(
#include <metal_stdlib>
using namespace metal;
)";
*/

namespace hydra::hw::tegra_x1::gpu::renderer::metal {

SINGLETON_DEFINE_GET_INSTANCE(Renderer, MetalRenderer)

Renderer::Renderer() {
    SINGLETON_SET_INSTANCE(Renderer, MetalRenderer);

    // Device
    device = MTL::CreateSystemDefaultDevice();
    command_queue = device->newCommandQueue();

    /*
    // Library
    MTL::Library* library =
        CreateLibraryFromSource(device, utility_shader_source);

    // Functions
    */

    // Resources

    // Depth stencil states

    // Always + write
    auto depth_stencil_descriptor =
        MTL::DepthStencilDescriptor::alloc()->init();
    depth_stencil_descriptor->setDepthWriteEnabled(true);
    depth_stencil_state_always_and_write =
        device->newDepthStencilState(depth_stencil_descriptor);
    depth_stencil_descriptor->release();

    // Sampler states

    // Nearest
    auto sampler_state_descriptor = MTL::SamplerDescriptor::alloc()->init();
    nearest_sampler = device->newSamplerState(sampler_state_descriptor);

    // Linear
    sampler_state_descriptor->setMinFilter(MTL::SamplerMinMagFilterLinear);
    sampler_state_descriptor->setMagFilter(MTL::SamplerMinMagFilterLinear);
    linear_sampler = device->newSamplerState(sampler_state_descriptor);
    sampler_state_descriptor->release();

    // Caches
    depth_stencil_state_cache = new DepthStencilStateCache();
    blit_pipeline_cache = new BlitPipelineCache();
    clear_color_pipeline_cache = new ClearColorPipelineCache();
    clear_depth_pipeline_cache = new ClearDepthPipelineCache();

    // Null

    // Texture
    constexpr usize NULL_TEXTURE_WIDTH = 128;
    constexpr usize NULL_TEXTURE_HEIGHT = 128;

    MTL::TextureDescriptor* texture_desc =
        MTL::TextureDescriptor::alloc()->init();
    texture_desc->setWidth(NULL_TEXTURE_WIDTH);
    texture_desc->setHeight(NULL_TEXTURE_HEIGHT);
    texture_desc->setPixelFormat(MTL::PixelFormatRGBA8Unorm);
    texture_desc->setStorageMode(MTL::StorageModePrivate);
    texture_desc->setUsage(MTL::TextureUsageShaderRead);
    null_texture = device->newTexture(texture_desc);
    texture_desc->release();

    // HACK: fill the null texture with a gradient
    {
        uchar4 gradient[NULL_TEXTURE_HEIGHT][NULL_TEXTURE_WIDTH];
        for (u32 y = 0; y < NULL_TEXTURE_HEIGHT; y++) {
            for (u32 x = 0; x < NULL_TEXTURE_WIDTH; x++) {
                gradient[y][x] = uchar4(
                    {static_cast<u8>(x * 256 / NULL_TEXTURE_WIDTH),
                     static_cast<u8>(y * 256 / NULL_TEXTURE_HEIGHT), 0, 255});
            }
        }
        null_texture->replaceRegion(
            MTL::Region(0, 0, 0, NULL_TEXTURE_WIDTH, NULL_TEXTURE_HEIGHT, 1), 0,
            gradient, sizeof(gradient) / NULL_TEXTURE_HEIGHT);
    }

    // Info
    info = {
        .supports_quads_primitive = false,
    };
}

Renderer::~Renderer() {
    delete depth_stencil_state_cache;
    delete blit_pipeline_cache;
    delete clear_color_pipeline_cache;
    delete clear_depth_pipeline_cache;

    linear_sampler->release();
    nearest_sampler->release();

    depth_stencil_state_always_and_write->release();

    command_queue->release();
    device->release();

    SINGLETON_UNSET_INSTANCE();
}

void Renderer::SetSurface(void* surface) {
    layer = reinterpret_cast<CA::MetalLayer*>(surface);
    layer->setDevice(device);
    // TODO: set pixel format
}

ISurfaceCompositor* Renderer::AcquireNextSurface() {
    // Drawable
    if (!layer)
        return nullptr;

    drawable = layer->nextDrawable();
    if (!drawable)
        return nullptr;

    return new SurfaceCompositor(drawable, command_queue);
}

BufferBase* Renderer::CreateBuffer(u64 size) { return new Buffer(size); }

BufferBase* Renderer::AllocateTemporaryBuffer(const u64 size) {
    // TODO: use a buffer allocator instead
    auto buffer = device->newBuffer(size, MTL::ResourceStorageModeShared);
    return new Buffer(buffer);
}

void Renderer::FreeTemporaryBuffer(BufferBase* buffer) {
    auto buffer_impl = static_cast<Buffer*>(buffer);

    // TODO: use a buffer allocator instead
    delete buffer_impl;
}

TextureBase* Renderer::CreateTexture(const TextureDescriptor& descriptor) {
    return new Texture(descriptor);
}

SamplerBase* Renderer::CreateSampler(const SamplerDescriptor& descriptor) {
    return new Sampler(descriptor);
}

void Renderer::EndCommandBuffer() { CommitCommandBuffer(); }

RenderPassBase*
Renderer::CreateRenderPass(const RenderPassDescriptor& descriptor) {
    return new RenderPass(descriptor);
}

void Renderer::BindRenderPass(const RenderPassBase* render_pass) {
    state.render_pass = static_cast<const RenderPass*>(render_pass);
}

void Renderer::ClearColor(u32 render_target_id, u32 layer, u8 mask,
                          const uint4 color) {
    auto texture = static_cast<Texture*>(state.render_pass->GetDescriptor()
                                             .color_targets[render_target_id]
                                             .texture);

    // HACK
    if (!texture) {
        ONCE(LOG_WARN(MetalRenderer, "Invalid color target at index {}",
                      render_target_id));
        return;
    }

    // TODO: layer
    ASSERT_DEBUG(layer == 0, MetalRenderer,
                 "Layered clears (layer: {}) not implemented", layer);

    auto encoder = GetRenderCommandEncoder();

    SetRenderPipelineState(clear_color_pipeline_cache->Find(
        {texture->GetPixelFormat(), render_target_id, mask}));
    // TODO: set viewport and scissor
    encoder->setVertexBytes(&render_target_id, sizeof(render_target_id), 0);
    encoder->setFragmentBytes(&color, sizeof(color), 0);
    encoder->drawPrimitives(MTL::PrimitiveTypeTriangle, NS::UInteger(0),
                            NS::UInteger(3));
}

void Renderer::ClearDepth(u32 layer, const float value) {
    auto texture = static_cast<Texture*>(
        state.render_pass->GetDescriptor().depth_stencil_target.texture);

    // HACK
    if (!texture) {
        ONCE(LOG_WARN(MetalRenderer, "Invalid depth target"));
        return;
    }

    const auto format = texture->GetDescriptor().format;
    if (!to_mtl_pixel_format_info(format).has_depth) {
        ONCE(LOG_WARN(MetalRenderer,
                      "Texture format {} does not have a depth component",
                      format));
        return;
    }

    auto encoder = GetRenderCommandEncoder();

    SetRenderPipelineState(
        clear_depth_pipeline_cache->Find(texture->GetPixelFormat()));
    SetDepthStencilState(depth_stencil_state_always_and_write);
    // TODO: set viewport and scissor
    struct {
        u32 layer_id;
        float value;
    } params = {layer, value};
    encoder->setVertexBytes(&params, sizeof(params), 0);
    encoder->drawPrimitives(MTL::PrimitiveTypeTriangle, NS::UInteger(0),
                            NS::UInteger(3));
}

void Renderer::ClearStencil(u32 layer, const u32 value) {
    ONCE(LOG_FUNC_WITH_ARGS_NOT_IMPLEMENTED(
        MetalRenderer, "layer: {}, value: {:#x}", layer, value));
}

void Renderer::SetViewport(u32 index, const Viewport& viewport) {
    state.viewports[index] = viewport;
}

void Renderer::SetScissor(u32 index, const Scissor& scissor) {
    state.scissors[index] = scissor;
}

ShaderBase* Renderer::CreateShader(const ShaderDescriptor& descriptor) {
    return new Shader(descriptor);
}

PipelineBase* Renderer::CreatePipeline(const PipelineDescriptor& descriptor) {
    return new Pipeline(descriptor);
}

void Renderer::BindPipeline(const PipelineBase* pipeline) {
    state.pipeline = static_cast<const Pipeline*>(pipeline);
}

void Renderer::BindVertexBuffer(const BufferView& buffer, u32 index) {
    state.vertex_buffers[index] = buffer;
}

void Renderer::BindIndexBuffer(const BufferView& index_buffer,
                               engines::IndexType index_type) {
    state.index_buffer = index_buffer;
    state.index_type = index_type;
}

void Renderer::BindUniformBuffer(const BufferView& buffer,
                                 ShaderType shader_type, u32 index) {
    // HACK
    if (shader_type == ShaderType::Count)
        return;

    state.uniform_buffers[u32(shader_type)][index] = buffer;
}

void Renderer::BindTexture(TextureBase* texture, SamplerBase* sampler,
                           ShaderType shader_type, u32 index) {
    // HACK
    if (shader_type == ShaderType::Count)
        return;

    state.textures[u32(shader_type)][index] = {static_cast<Texture*>(texture),
                                               static_cast<Sampler*>(sampler)};
}

void Renderer::UnbindUniformBuffers(ShaderType shader_type) {
    // HACK
    if (shader_type == ShaderType::Count)
        return;

    state.uniform_buffers[u32(shader_type)] = {};
}

void Renderer::UnbindTextures(ShaderType shader_type) {
    // HACK
    if (shader_type == ShaderType::Count)
        return;

    state.textures[u32(shader_type)] = {};
}

void Renderer::Draw(const engines::PrimitiveType primitive_type,
                    const u32 start, const u32 count, const u32 base_instance,
                    const u32 instance_count) {
    // Check for errors
    if (!CanDraw())
        return;

    BindDrawState();

    auto encoder = GetRenderCommandEncoderUnchecked();

    // Draw
    encoder->drawPrimitives(to_mtl_primitive_type(primitive_type), start, count,
                            instance_count, base_instance);
}

void Renderer::DrawIndexed(const engines::PrimitiveType primitive_type,
                           const u32 start, const u32 count,
                           const u32 base_vertex, const u32 base_instance,
                           const u32 instance_count) {
    // Check for errors
    if (!CanDraw())
        return;

    BindDrawState();

    auto encoder = GetRenderCommandEncoderUnchecked();

    // Draw
    auto index_buffer_mtl =
        static_cast<Buffer*>(state.index_buffer.GetBase())->GetBuffer();
    // TODO: is start used correctly?
    const auto index_buffer_offset =
        static_cast<u32>(start *
                         engines::get_index_type_size(state.index_type)) +
        state.index_buffer.GetOffset();
    encoder->drawIndexedPrimitives(to_mtl_primitive_type(primitive_type), count,
                                   to_mtl_index_type(state.index_type),
                                   index_buffer_mtl, index_buffer_offset,
                                   instance_count, base_vertex, base_instance);
}

void Renderer::EnsureCommandBuffer() {
    if (!command_buffer) {
        TMP_AUTORELEASE_POOL_BEGIN();
        command_buffer = command_queue->commandBuffer()->retain();
        TMP_AUTORELEASE_POOL_END();
    }
}

MTL::RenderCommandEncoder* Renderer::GetRenderCommandEncoder() {
    auto mtl_render_pass = state.render_pass->GetRenderPassDescriptor();
    if (mtl_render_pass == encoder_state.render_pass)
        return GetRenderCommandEncoderUnchecked();

    encoder_state.render_pass = mtl_render_pass;
    encoder_state.render = {};

    // Reset bindings
    encoder_state.render.buffers = {};
    encoder_state.render.textures = {};
    encoder_state.render.samplers = {};

    return CreateRenderCommandEncoder(mtl_render_pass);
}

MTL::RenderCommandEncoder* Renderer::CreateRenderCommandEncoder(
    MTL::RenderPassDescriptor* render_pass_descriptor) {
    EnsureCommandBuffer();
    EndEncoding();

    TMP_AUTORELEASE_POOL_BEGIN();
    command_encoder =
        command_buffer->renderCommandEncoder(render_pass_descriptor)->retain();
    TMP_AUTORELEASE_POOL_END();

    encoder_type = EncoderType::Render;
    encoder_state.render_pass = render_pass_descriptor;

    // HACK: bind null textures
    for (u32 i = 0; i < TEXTURE_COUNT; i++) {
        GetRenderCommandEncoderUnchecked()->setVertexTexture(null_texture, i);
        GetRenderCommandEncoderUnchecked()->setFragmentTexture(null_texture, i);
    }

    return GetRenderCommandEncoderUnchecked();
}

MTL::BlitCommandEncoder* Renderer::GetBlitCommandEncoder() {
    if (encoder_type == EncoderType::Blit)
        return GetBlitCommandEncoderUnchecked();

    EnsureCommandBuffer();
    EndEncoding();

    TMP_AUTORELEASE_POOL_BEGIN();
    command_encoder = command_buffer->blitCommandEncoder()->retain();
    TMP_AUTORELEASE_POOL_END();

    encoder_type = EncoderType::Blit;

    return GetBlitCommandEncoderUnchecked();
}

void Renderer::EndEncoding() {
    if (encoder_type == EncoderType::None)
        return;

    command_encoder->endEncoding();
    command_encoder->release();
    command_encoder = nullptr;
    encoder_type = EncoderType::None;

    // Reset the render pass
    encoder_state.render_pass = nullptr;
}

void Renderer::SetRenderPipelineState(MTL::RenderPipelineState* pipeline) {
    auto& bound_pipeline = encoder_state.render.pipeline;
    if (pipeline == bound_pipeline)
        return;

    GetRenderCommandEncoderUnchecked()->setRenderPipelineState(pipeline);
    bound_pipeline = pipeline;
}

void Renderer::SetRenderPipelineState() {
    SetRenderPipelineState(state.pipeline->GetPipeline());
}

void Renderer::SetDepthStencilState(
    MTL::DepthStencilState* depth_stencil_state) {
    auto& bound_depth_stencil_state = encoder_state.render.depth_stencil_state;
    if (depth_stencil_state == bound_depth_stencil_state)
        return;

    GetRenderCommandEncoderUnchecked()->setDepthStencilState(
        depth_stencil_state);
    bound_depth_stencil_state = depth_stencil_state;
}

void Renderer::SetDepthStencilState() {
    DepthStencilStateDescriptor descriptor{
        .depth_test_enabled = static_cast<bool>(REGS_3D.depth_test_enabled),
        .depth_write_enabled = static_cast<bool>(REGS_3D.depth_write_enabled),
        .depth_compare_op = REGS_3D.depth_compare_op,
    };

    SetDepthStencilState(depth_stencil_state_cache->Find(descriptor));
}

void Renderer::SetCullMode(MTL::CullMode cull_mode) {
    auto& bound_cull_mode = encoder_state.render.cull_mode;
    if (cull_mode == bound_cull_mode)
        return;

    GetRenderCommandEncoderUnchecked()->setCullMode(cull_mode);
    bound_cull_mode = cull_mode;
}

void Renderer::SetFrontFaceWinding(MTL::Winding front_face_winding) {
    auto& bound_front_face_winding = encoder_state.render.front_face_winding;
    if (front_face_winding == bound_front_face_winding)
        return;

    GetRenderCommandEncoderUnchecked()->setFrontFacingWinding(
        front_face_winding);
    bound_front_face_winding = front_face_winding;
}

void Renderer::SetCullState() {
    /*
    if (REGS_3D.cull_face_enabled) {
        SetCullMode(ToMtlCullMode(REGS_3D.cull_face_mode));
        SetFrontFaceWinding(ToMtlWinding(REGS_3D.front_face_winding));
    } else {
        SetCullMode(MTL::CullModeNone);
    }
    */
}

void Renderer::SetBuffer(MTL::Buffer* buffer, u64 offset,
                         ShaderType shader_type, u32 index) {
    ASSERT_DEBUG(index < BUFFER_COUNT, MetalRenderer, "Invalid buffer index {}",
                 index);

    auto& bound_buffer =
        encoder_state.render.buffers[static_cast<u32>(shader_type)][index];
    if (buffer == bound_buffer.buffer && offset == bound_buffer.offset)
        return;

    // TODO: fast path for offset only change

    switch (shader_type) {
    case ShaderType::Vertex:
        GetRenderCommandEncoderUnchecked()->setVertexBuffer(buffer, offset,
                                                            index);
        break;
    case ShaderType::Fragment:
        GetRenderCommandEncoderUnchecked()->setFragmentBuffer(buffer, offset,
                                                              index);
        break;
    default:
        LOG_ERROR(MetalRenderer, "Invalid shader type {}", shader_type);
        break;
    }
    bound_buffer.buffer = buffer;
    bound_buffer.offset = offset;
}

void Renderer::SetVertexBuffer(u32 index) {
    ASSERT_DEBUG(index < VERTEX_ARRAY_COUNT, MetalRenderer,
                 "Invalid vertex buffer index {}", index);

    const auto buffer = state.vertex_buffers[index];
    if (!buffer.GetBase())
        return;

    SetBuffer(static_cast<Buffer*>(buffer.GetBase())->GetBuffer(),
              buffer.GetOffset(), ShaderType::Vertex,
              GetVertexBufferIndex(index));
}

void Renderer::SetUniformBuffer(ShaderType shader_type, u32 index) {
    // TODO: get the index from resource mapping

    ASSERT_DEBUG(index < CONST_BUFFER_BINDING_COUNT, MetalRenderer,
                 "Invalid uniform buffer index {}", index);

    const auto buffer =
        state.uniform_buffers[static_cast<u32>(shader_type)][index];
    if (!buffer.GetBase())
        return;

    SetBuffer(static_cast<Buffer*>(buffer.GetBase())->GetBuffer(),
              buffer.GetOffset(), shader_type, index);
}

void Renderer::SetTexture(MTL::Texture* texture, ShaderType shader_type,
                          u32 index) {
    ASSERT_DEBUG(index < TEXTURE_COUNT, MetalRenderer,
                 "Invalid texture index {}", index);

    auto& bound_texture =
        encoder_state.render.textures[static_cast<u32>(shader_type)][index];
    if (texture == bound_texture)
        return;

    switch (shader_type) {
    case ShaderType::Vertex:
        GetRenderCommandEncoderUnchecked()->setVertexTexture(texture, index);
        break;
    case ShaderType::Fragment:
        GetRenderCommandEncoderUnchecked()->setFragmentTexture(texture, index);
        break;
    default:
        LOG_ERROR(MetalRenderer, "Invalid shader type {}", shader_type);
        break;
    }
    bound_texture = texture;
}

void Renderer::SetSampler(MTL::SamplerState* sampler, ShaderType shader_type,
                          u32 index) {
    ASSERT_DEBUG(index < TEXTURE_COUNT, MetalRenderer,
                 "Invalid texture index {}", index);

    auto& bound_sampler =
        encoder_state.render.samplers[static_cast<u32>(shader_type)][index];
    if (sampler == bound_sampler)
        return;

    switch (shader_type) {
    case ShaderType::Vertex:
        GetRenderCommandEncoderUnchecked()->setVertexSamplerState(sampler,
                                                                  index);
        break;
    case ShaderType::Fragment:
        GetRenderCommandEncoderUnchecked()->setFragmentSamplerState(sampler,
                                                                    index);
        break;
    default:
        LOG_ERROR(MetalRenderer, "Invalid shader type {}", shader_type);
        break;
    }
    bound_sampler = sampler;
}

void Renderer::SetTexture(ShaderType shader_type, u32 index) {
    const auto texture = state.textures[u32(shader_type)][index];
    if (texture.texture)
        SetTexture(texture.texture->GetTexture(), shader_type, index);
    if (texture.sampler)
        SetSampler(texture.sampler->GetSampler(), shader_type, index);
}

// TODO: what about 3D textures?
void Renderer::BlitTexture(MTL::Texture* src, const float3 src_origin,
                           const usize3 src_size, MTL::Texture* dst,
                           const u32 dst_layer, const float3 dst_origin,
                           const usize3 dst_size) {
    // Render pass
    auto render_pass_descriptor = MTL::RenderPassDescriptor::alloc()->init();
    auto color_attachment =
        render_pass_descriptor->colorAttachments()->object(0);
    color_attachment->setTexture(dst);
    color_attachment->setLoadAction(
        MTL::LoadActionLoad); // TODO: use don't care if blitting to the whole
                              // texture
    color_attachment->setStoreAction(MTL::StoreActionStore);

    auto encoder = CreateRenderCommandEncoder(render_pass_descriptor);
    render_pass_descriptor->release();

    // Draw
    encoder->setRenderPipelineState(
        blit_pipeline_cache->Find({src->pixelFormat(), false}));
    encoder->setViewport(MTL::Viewport(f64(dst_origin.x()), f64(dst_origin.y()),
                                       f64(dst_size.x()), f64(dst_size.y()),
                                       0.0, 1.0));
    encoder->setVertexBytes(&dst_layer, sizeof(dst_layer), 0);
    // TODO: correct?
    float2 scale =
        (float2(src_size) / float2({static_cast<f32>(src->width()),
                                    static_cast<f32>(src->height())}));
    BlitParams params = {
        .src_offset = {f32(src_origin.x()) / f32(src_size.x()),
                       f32(src_origin.y()) / f32(src_size.y())},
        .src_scale = scale,
    };
    encoder->setFragmentBytes(&params, sizeof(params), 0);
    encoder->setFragmentTexture(src, NS::UInteger(0));
    encoder->setFragmentSamplerState(
        linear_sampler, NS::UInteger(0)); // TODO: use the correct sampler

    encoder->drawPrimitives(MTL::PrimitiveTypeTriangle, NS::UInteger(0),
                            NS::UInteger(3));
}

void Renderer::BeginCapture() {
    auto capture_manager = MTL::CaptureManager::sharedCaptureManager();
    auto desc = MTL::CaptureDescriptor::alloc()->init();
    desc->setCaptureObject(device);

    // Check if a debugger with support for Gpu capture is attached
    if (capture_manager->supportsDestination(
            MTL::CaptureDestinationDeveloperTools)) {
        desc->setDestination(MTL::CaptureDestinationDeveloperTools);
    } else {
        // TODO: don't hardcode the directory
        const std::string gpu_capture_dir =
            fmt::format("{}/gpu_captures", CONFIG_INSTANCE.GetAppDataPath());
        if (gpu_capture_dir.empty()) {
            LOG_ERROR(
                MetalRenderer,
                "No GPU capture directory specified, cannot do a Gpu capture");
            return;
        }

        // Check if the Gpu trace document destination is available
        if (!capture_manager->supportsDestination(
                MTL::CaptureDestinationGPUTraceDocument)) {
            LOG_ERROR(MetalRenderer, "GPU trace document destination is not "
                                     "available, cannot do a GPU capture");
            return;
        }

        // Get current date and time as a string
        auto now = std::chrono::system_clock::now();
        std::time_t now_time = std::chrono::system_clock::to_time_t(now);
        std::ostringstream oss;
        oss << std::put_time(std::localtime(&now_time), "%Y.%m.%d_%H:%M:%S");
        std::string now_str = oss.str();

        std::string capture_path =
            fmt::format("{}/hydra_{}.gputrace", gpu_capture_dir, now_str);
        desc->setDestination(MTL::CaptureDestinationGPUTraceDocument);
        desc->setOutputURL(ToNSURL(capture_path));
    }

    NS::Error* error = nullptr;
    capture_manager->startCapture(desc, &error);
    if (error) {
        LOG_ERROR(MetalRenderer, "Failed to start GPU capture: {}",
                  error->localizedDescription()->utf8String());
    }
}

void Renderer::EndCapture() {
    CommitCommandBuffer();

    auto captureManager = MTL::CaptureManager::sharedCaptureManager();
    captureManager->stopCapture();
}

bool Renderer::CanDraw() {
    if (!state.pipeline->GetPipeline()) {
        ONCE(LOG_WARN(MetalRenderer, "Pipeline not present, skipping draw"));
        return false;
    }

    return true;
}

void Renderer::BindDrawState() {
    auto encoder = GetRenderCommandEncoder();

    // States
    SetRenderPipelineState();
    SetDepthStencilState();
    SetCullState();

    // Viewport and scissor
    MTL::Viewport viewports[VIEWPORT_COUNT];
    MTL::ScissorRect scissors[VIEWPORT_COUNT];
    for (u32 i = 0; i < VIEWPORT_COUNT; i++) {
        // Viewport
        const auto& viewport = state.viewports[i];
        viewports[i] =
            MTL::Viewport(viewport.rect.origin.x(), viewport.rect.origin.y(),
                          viewport.rect.size.x(), viewport.rect.size.y(),
                          viewport.depth_near, viewport.depth_far);

        // Scissor
        const auto& scissor = state.scissors[i];
        scissors[i] = MTL::ScissorRect(scissor.origin.x(), scissor.origin.y(),
                                       scissor.size.x(), scissor.size.y());
    }
    encoder->setViewports(viewports, sizeof_array(viewports));
    encoder->setScissorRects(scissors, sizeof_array(scissors));

    // Resources
    for (u32 i = 0; i < VERTEX_ARRAY_COUNT; i++)
        SetVertexBuffer(i);
    for (u32 shader_type = 0; shader_type < usize(ShaderType::Count);
         shader_type++) {
        for (u32 i = 0; i < CONST_BUFFER_BINDING_COUNT; i++)
            SetUniformBuffer(ShaderType(shader_type), i);
    }
    // TODO: storage buffers
    for (u32 shader_type = 0; shader_type < usize(ShaderType::Count);
         shader_type++) {
        for (u32 i = 0; i < TEXTURE_COUNT; i++)
            SetTexture(ShaderType(shader_type), i);
    }
}

} // namespace hydra::hw::tegra_x1::gpu::renderer::metal
