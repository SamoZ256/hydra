#include "hw/tegra_x1/gpu/renderer/metal/renderer.hpp"

#include "hw/tegra_x1/gpu/engines/3d.hpp"
#include "hw/tegra_x1/gpu/renderer/metal/buffer.hpp"
#include "hw/tegra_x1/gpu/renderer/metal/const.hpp"
#include "hw/tegra_x1/gpu/renderer/metal/maxwell_to_mtl.hpp"
#include "hw/tegra_x1/gpu/renderer/metal/pipeline.hpp"
#include "hw/tegra_x1/gpu/renderer/metal/render_pass.hpp"
#include "hw/tegra_x1/gpu/renderer/metal/shader.hpp"
#include "hw/tegra_x1/gpu/renderer/metal/texture.hpp"

// TODO: define in a separate file
/*
const std::string utility_shader_source = R"(
#include <metal_stdlib>
using namespace metal;
)";
*/

namespace Hydra::HW::TegraX1::GPU::Renderer::Metal {

namespace {

struct BlitParams {
    float2 src_offset;
    float2 src_scale;
};

} // namespace

SINGLETON_DEFINE_GET_INSTANCE(Renderer, MetalRenderer, "Metal renderer")

Renderer::Renderer() {
    SINGLETON_SET_INSTANCE(MetalRenderer, "Metal renderer");

    // Device
    device = MTL::CreateSystemDefaultDevice();
    command_queue = device->newCommandQueue();

    /*
    // Library
    MTL::Library* library =
        CreateLibraryFromSource(device, utility_shader_source);

    // Functions
    */

    // Objects

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

    // Clear state
    for (u32 shader_type = 0; shader_type < usize(ShaderType::Count);
         shader_type++) {
        for (u32 i = 0; i < UNIFORM_BUFFER_BINDING_COUNT; i++)
            state.uniform_buffers[shader_type][i] = nullptr;
        for (u32 i = 0; i < TEXTURE_BINDING_COUNT; i++)
            state.textures[shader_type][i] = nullptr;
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

void Renderer::Present(TextureBase* texture) {
    auto texture_impl = static_cast<Texture*>(texture);

    // TODO: acquire drawable earlier?
    auto drawable = layer->nextDrawable();
    auto dst = drawable->texture();

    // Command buffer
    MTL::CommandBuffer* command_buffer = command_queue->commandBuffer();

    // Render pass
    auto render_pass_descriptor = MTL::RenderPassDescriptor::alloc()->init();
    auto color_attachment =
        render_pass_descriptor->colorAttachments()->object(0);
    color_attachment->setTexture(dst);
    color_attachment->setLoadAction(
        MTL::LoadActionDontCare); // TODO: use load if not blitting to the whole
                                  // texture
    color_attachment->setStoreAction(MTL::StoreActionStore);

    auto encoder = command_buffer->renderCommandEncoder(render_pass_descriptor);
    render_pass_descriptor->release();

    // Draw
    encoder->setRenderPipelineState(
        blit_pipeline_cache->Find({dst->pixelFormat()}));
    u32 zero = 0;
    encoder->setVertexBytes(&zero, sizeof(zero), 0);
    BlitParams params = {
        .src_offset = {0.0f, 0.0f},
        .src_scale = {1.0f, 1.0f},
    };
    encoder->setFragmentBytes(&params, sizeof(params), 0);
    encoder->setFragmentTexture(texture_impl->GetTexture(), NS::UInteger(0));
    encoder->setFragmentSamplerState(linear_sampler, NS::UInteger(0));
    encoder->drawPrimitives(MTL::PrimitiveTypeTriangle, NS::UInteger(0),
                            NS::UInteger(3));

    encoder->endEncoding();

    // Present
    command_buffer->presentDrawable(drawable);
    command_buffer->commit();
}

BufferBase* Renderer::CreateBuffer(const BufferDescriptor& descriptor) {
    return new Buffer(descriptor);
}

BufferBase* Renderer::AllocateTemporaryBuffer(const usize size) {
    // TODO: use a buffer allocator instead
    auto buffer = device->newBuffer(size, MTL::ResourceStorageModeShared);
    return new Buffer(buffer, 0);
}

void Renderer::FreeTemporaryBuffer(BufferBase* buffer) {
    auto buffer_impl = static_cast<Buffer*>(buffer);

    // TODO: use a buffer allocator instead
    buffer_impl->GetBuffer()->release();
    delete buffer_impl;
}

TextureBase* Renderer::CreateTexture(const TextureDescriptor& descriptor) {
    return new Texture(descriptor);
}

void Renderer::BeginCommandBuffer() {
    ASSERT_DEBUG(!command_buffer, MetalRenderer,
                 "Command buffer already started");
    command_buffer = command_queue->commandBuffer();
}

void Renderer::EndCommandBuffer() {
    ASSERT_DEBUG(command_buffer, MetalRenderer, "Command buffer not started");

    EndEncoding();

    command_buffer->commit();
    command_buffer->release(); // TODO: release?
    command_buffer = nullptr;
}

RenderPassBase*
Renderer::CreateRenderPass(const RenderPassDescriptor& descriptor) {
    return new RenderPass(descriptor);
}

void Renderer::BindRenderPass(const RenderPassBase* render_pass) {
    state.render_pass = static_cast<const RenderPass*>(render_pass);
}

void Renderer::ClearColor(u32 render_target_id, u32 layer, u8 mask,
                          const uint4 color) {
    auto encoder = GetRenderCommandEncoder();

    auto texture = static_cast<Texture*>(state.render_pass->GetDescriptor()
                                             .color_targets[render_target_id]
                                             .texture);

    SetRenderPipelineState(clear_color_pipeline_cache->Find(
        {texture->GetPixelFormat(), render_target_id, mask}));
    // TODO: set viewport and scissor
    encoder->setVertexBytes(&render_target_id, sizeof(render_target_id), 0);
    encoder->setFragmentBytes(&color, sizeof(color), 0);
    encoder->drawPrimitives(MTL::PrimitiveTypeTriangle, NS::UInteger(0),
                            NS::UInteger(3));
}

void Renderer::ClearDepth(u32 layer, const float value) {
    auto encoder = GetRenderCommandEncoder();

    auto texture = static_cast<Texture*>(
        state.render_pass->GetDescriptor().depth_stencil_target.texture);

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
    LOG_NOT_IMPLEMENTED(MetalRenderer, "Stencil clears");
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

void Renderer::BindVertexBuffer(BufferBase* buffer, u32 index) {
    state.vertex_buffers[index] = static_cast<Buffer*>(buffer);
}

void Renderer::BindIndexBuffer(BufferBase* index_buffer,
                               Engines::IndexType index_type) {
    state.index_buffer = static_cast<Buffer*>(index_buffer);
    state.index_type = index_type;
}

void Renderer::BindUniformBuffer(BufferBase* buffer, ShaderType shader_type,
                                 u32 index) {
    // HACK
    if (shader_type == ShaderType::Count)
        return;

    state.uniform_buffers[u32(shader_type)][index] =
        static_cast<Buffer*>(buffer);
}

void Renderer::BindTexture(TextureBase* texture, ShaderType shader_type,
                           u32 index) {
    // HACK
    if (shader_type == ShaderType::Count)
        return;

    state.textures[u32(shader_type)][index] = static_cast<Texture*>(texture);
}

void Renderer::Draw(const Engines::PrimitiveType primitive_type,
                    const u32 start, const u32 count, bool indexed) {
    auto encoder = GetRenderCommandEncoder();

    // State
    SetRenderPipelineState();
    SetDepthStencilState();
    // TODO: viewport and scissor
    for (u32 i = 0; i < VERTEX_ARRAY_COUNT; i++)
        SetVertexBuffer(i);
    for (u32 shader_type = 0; shader_type < usize(ShaderType::Count);
         shader_type++) {
        for (u32 i = 0; i < UNIFORM_BUFFER_BINDING_COUNT; i++)
            SetUniformBuffer(ShaderType(shader_type), i);
    }
    // TODO: storage buffers
    for (u32 shader_type = 0; shader_type < usize(ShaderType::Count);
         shader_type++) {
        for (u32 i = 0; i < TEXTURE_COUNT; i++)
            SetTexture(ShaderType(shader_type), i);
    }

    // Draw
    if (indexed) {
        auto index_buffer_mtl = state.index_buffer->GetBuffer();

        // TODO: is start used correctly?
        encoder->drawIndexedPrimitives(
            to_mtl_primitive_type(primitive_type), NS::UInteger(count),
            to_mtl_index_type(state.index_type), index_buffer_mtl,
            NS::UInteger(start), 1);
    } else {
        encoder->drawPrimitives(to_mtl_primitive_type(primitive_type),
                                NS::UInteger(start), NS::UInteger(count));
    }

    // Debug
#if 0
    static u32 frames = 0;
    if (capturing) {
        if (frames >= 3)
            EndCapture();
        frames++;
    }

    static bool did_capture = false;
    if (!did_capture) {
        BeginCapture();
        did_capture = true;
    }
#endif
}

MTL::RenderCommandEncoder* Renderer::GetRenderCommandEncoder() {
    auto mtl_render_pass = state.render_pass->GetRenderPassDescriptor();
    if (mtl_render_pass == encoder_state.render_pass)
        return GetRenderCommandEncoderUnchecked();

    encoder_state.render_pass = mtl_render_pass;
    encoder_state.render = {};
    for (u32 shader_type = 0; shader_type < usize(ShaderType::Count);
         shader_type++) {
        for (u32 i = 0; i < BUFFER_COUNT; i++)
            encoder_state.render.buffers[shader_type][i] = nullptr;
        for (u32 i = 0; i < TEXTURE_COUNT; i++)
            encoder_state.render.textures[shader_type][i] = nullptr;
    }

    return CreateRenderCommandEncoder(mtl_render_pass);
}

MTL::RenderCommandEncoder* Renderer::CreateRenderCommandEncoder(
    MTL::RenderPassDescriptor* render_pass_descriptor) {
    ASSERT_DEBUG(command_buffer, MetalRenderer, "Command buffer not started");

    EndEncoding();

    command_encoder =
        command_buffer->renderCommandEncoder(render_pass_descriptor);
    encoder_type = EncoderType::Render;
    encoder_state.render_pass = render_pass_descriptor;

    return GetRenderCommandEncoderUnchecked();
}

MTL::BlitCommandEncoder* Renderer::GetBlitCommandEncoder() {
    if (encoder_type == EncoderType::Blit)
        return GetBlitCommandEncoderUnchecked();

    ASSERT_DEBUG(command_buffer, MetalRenderer, "Command buffer not started");

    EndEncoding();

    command_encoder = command_buffer->blitCommandEncoder();
    encoder_type = EncoderType::Blit;

    return GetBlitCommandEncoderUnchecked();
}

void Renderer::EndEncoding() {
    if (encoder_type == EncoderType::None)
        return;

    command_encoder->endEncoding();
    command_encoder->release(); // TODO: release?
    command_encoder = nullptr;
    encoder_type = EncoderType::None;

    // Reset the render pass
    encoder_state.render_pass = nullptr;
}

void Renderer::SetRenderPipelineState(MTL::RenderPipelineState* mtl_pipeline) {
    auto& bound_pipeline = encoder_state.render.pipeline;
    if (mtl_pipeline == bound_pipeline)
        return;

    GetRenderCommandEncoderUnchecked()->setRenderPipelineState(mtl_pipeline);
    bound_pipeline = mtl_pipeline;
}

void Renderer::SetRenderPipelineState() {
    SetRenderPipelineState(state.pipeline->GetPipeline());
}

void Renderer::SetDepthStencilState(
    MTL::DepthStencilState* mtl_depth_stencil_state) {
    auto& bound_depth_stencil_state = encoder_state.render.depth_stencil_state;
    if (mtl_depth_stencil_state == bound_depth_stencil_state)
        return;

    GetRenderCommandEncoderUnchecked()->setDepthStencilState(
        mtl_depth_stencil_state);
    bound_depth_stencil_state = mtl_depth_stencil_state;
}

void Renderer::SetDepthStencilState() {
    DepthStencilStateDescriptor descriptor{
        .depth_test_enabled = REGS_3D.depth_test_enabled,
        .depth_write_enabled = REGS_3D.depth_write_enabled,
        .depth_test_func = REGS_3D.depth_test_func,
    };

    SetDepthStencilState(depth_stencil_state_cache->Find(descriptor));
}

void Renderer::SetBuffer(MTL::Buffer* buffer, ShaderType shader_type,
                         u32 index) {
    ASSERT_DEBUG(index < BUFFER_COUNT, MetalRenderer, "Invalid buffer index {}",
                 index);

    auto& bound_buffer =
        encoder_state.render.buffers[static_cast<u32>(shader_type)][index];
    if (buffer == bound_buffer)
        return;

    switch (shader_type) {
    case ShaderType::Vertex:
        GetRenderCommandEncoderUnchecked()->setVertexBuffer(buffer, 0, index);
        break;
    case ShaderType::Fragment:
        GetRenderCommandEncoderUnchecked()->setFragmentBuffer(buffer, 0, index);
        break;
    default:
        LOG_ERROR(MetalRenderer, "Invalid shader type {}", shader_type);
        break;
    }
    bound_buffer = buffer;
}

void Renderer::SetVertexBuffer(u32 index) {
    ASSERT_DEBUG(index < VERTEX_ARRAY_COUNT, MetalRenderer,
                 "Invalid vertex buffer index {}", index);

    const auto buffer = state.vertex_buffers[index];
    if (!buffer)
        return;

    SetBuffer(buffer->GetBuffer(), ShaderType::Vertex,
              GetVertexBufferIndex(index));
}

void Renderer::SetUniformBuffer(ShaderType shader_type, u32 index) {
    // TODO: get the index from resource mapping

    ASSERT_DEBUG(index < UNIFORM_BUFFER_BINDING_COUNT, MetalRenderer,
                 "Invalid uniform buffer index {}", index);

    const auto buffer =
        state.uniform_buffers[static_cast<u32>(shader_type)][index];
    if (!buffer)
        return;

    SetBuffer(buffer->GetBuffer(), shader_type, index);
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

void Renderer::SetTexture(ShaderType shader_type, u32 index) {
    // TODO: get the index from resource mapping

    const auto texture = state.textures[u32(shader_type)][index];
    if (!texture)
        return;

    SetTexture(texture->GetTexture(), shader_type, index);
}

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
        blit_pipeline_cache->Find({src->pixelFormat()}));
    // TODO: viewport
    encoder->setVertexBytes(&dst_layer, sizeof(dst_layer), 0);
    // TODO: correct?
    float2 scale = (float2(src_size) / float2(dst_size)) *
                   (float2({static_cast<f32>(dst->width()),
                            static_cast<f32>(dst->height())}) /
                    float2({static_cast<f32>(src->width()),
                            static_cast<f32>(src->height())}));
    BlitParams params = {
        .src_offset = {static_cast<f32>(src_origin.x()) /
                           static_cast<f32>(src_size.x()),
                       static_cast<f32>(src_origin.y()) /
                           static_cast<f32>(src_size.y())},
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

    // Check if a debugger with support for GPU capture is attached
    if (capture_manager->supportsDestination(
            MTL::CaptureDestinationDeveloperTools)) {
        desc->setDestination(MTL::CaptureDestinationDeveloperTools);
    } else {
        // TODO: don't hardcode the directory
        const std::string gpu_capture_dir = "/Users/samuliak/Downloads";
        if (gpu_capture_dir.empty()) {
            LOG_ERROR(
                MetalRenderer,
                "No GPU capture directory specified, cannot do a GPU capture");
            return;
        }

        // Check if the GPU trace document destination is available
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
        oss << std::put_time(std::localtime(&now_time), "%d.%m.%Y_%H:%M:%S");
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

    capturing = true;
}

void Renderer::EndCapture() {
    auto captureManager = MTL::CaptureManager::sharedCaptureManager();
    captureManager->stopCapture();

    capturing = false;
}

} // namespace Hydra::HW::TegraX1::GPU::Renderer::Metal
