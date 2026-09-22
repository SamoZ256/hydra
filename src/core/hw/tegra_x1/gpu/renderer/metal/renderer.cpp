#include "core/hw/tegra_x1/gpu/renderer/metal/renderer.hpp"

#include "common/config.hpp"
#include "core/hw/tegra_x1/gpu/engines/3d.hpp"
#include "core/hw/tegra_x1/gpu/renderer/metal/buffer.hpp"
#include "core/hw/tegra_x1/gpu/renderer/metal/command_buffer.hpp"
#include "core/hw/tegra_x1/gpu/renderer/metal/const.hpp"
#include "core/hw/tegra_x1/gpu/renderer/metal/maxwell_to_mtl.hpp"
#include "core/hw/tegra_x1/gpu/renderer/metal/pipeline.hpp"
#include "core/hw/tegra_x1/gpu/renderer/metal/render_pass.hpp"
#include "core/hw/tegra_x1/gpu/renderer/metal/sampler.hpp"
#include "core/hw/tegra_x1/gpu/renderer/metal/shader.hpp"
#include "core/hw/tegra_x1/gpu/renderer/metal/surface_compositor.hpp"
#include "core/hw/tegra_x1/gpu/renderer/metal/texture.hpp"
#include "core/hw/tegra_x1/gpu/renderer/metal/texture_view.hpp"

// TODO: define in a separate file
/*
const std::string utility_shader_source = R"(
#include <metal_stdlib>
using namespace metal;
)";
*/

namespace hydra::hw::tegra_x1::gpu::renderer::metal {

Renderer::Renderer()
    : device{MTL::CreateSystemDefaultDevice()},
      command_queue{device->newCommandQueue()},
      depth_stencil_state_cache(device), blit_pipeline_cache(device),
      clear_color_pipeline_cache(device), clear_depth_pipeline_cache(device) {
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

    // Info
    info = {
        .supports_quads_primitive = false,
    };
}

Renderer::~Renderer() {
    linear_sampler->release();
    nearest_sampler->release();

    depth_stencil_state_always_and_write->release();

    // TODO: destroy caches here?

    command_queue->release();
    device->release();
}

void Renderer::setSurface(void* surface) {
    ca_layer = reinterpret_cast<CA::MetalLayer*>(surface);
    ca_layer->setDevice(device);
    // TODO: set pixel format
}

ISurfaceCompositor* Renderer::acquireNextSurface() {
    // Drawable
    if (ca_layer == nullptr)
        return nullptr;

    ca_drawable = ca_layer->nextDrawable();
    if (ca_drawable == nullptr)
        return nullptr;

    return new SurfaceCompositor(*this, ca_drawable);
}

BufferBase* Renderer::createBuffer(u64 size) {
    return new Buffer(device, size);
}

BufferBase* Renderer::allocateTemporaryBuffer(const u64 size) {
    // TODO: use a buffer allocator instead
    auto buffer = device->newBuffer(size, MTL::ResourceStorageModeShared);
    return new Buffer(buffer);
}

void Renderer::freeTemporaryBuffer(BufferBase* buffer) {
    auto buffer_impl = static_cast<Buffer*>(buffer);

    // TODO: use a buffer allocator instead
    delete buffer_impl;
}

ITexture* Renderer::createTexture(const TextureDescriptor& descriptor) {
    return new Texture(device, descriptor);
}

void Renderer::blitTexture(ICommandBuffer* command_buffer, ITextureView* src,
                           float3 src_origin, uint3 src_size, u32 src_level,
                           u32 src_layer, ITextureView* dst, float3 dst_origin,
                           uint3 dst_size, u32 dst_level, u32 dst_layer,
                           u32 level_count, u32 layer_count) {
    // TODO: what about 3D textures?
    (void)src_level;
    (void)src_layer;
    (void)dst_level;
    (void)level_count;
    (void)layer_count;

    const auto command_buffer_impl =
        static_cast<CommandBuffer*>(command_buffer);
    const auto src_impl = static_cast<TextureView*>(src);
    const auto dst_impl = static_cast<TextureView*>(dst);

    // Render pass
    NS_STACK_SCOPED auto render_pass_descriptor =
        MTL::RenderPassDescriptor::alloc()->init();
    auto color_attachment =
        render_pass_descriptor->colorAttachments()->object(0);
    color_attachment->setTexture(dst_impl->getTexture());
    color_attachment->setLoadAction(
        MTL::LoadActionLoad); // TODO: use don't care if blitting to the whole
                              // texture
    color_attachment->setStoreAction(MTL::StoreActionStore);

    auto encoder =
        command_buffer_impl->createRenderCommandEncoder(render_pass_descriptor);

    // Draw
    encoder->setRenderPipelineState(blit_pipeline_cache.find(
        {.pixel_format = src_impl->getTexture()->pixelFormat(),
         .transparent = false}));
    encoder->setViewport(MTL::Viewport(
        static_cast<f64>(dst_origin.x()), static_cast<f64>(dst_origin.y()),
        static_cast<f64>(dst_size.x()), static_cast<f64>(dst_size.y()), 0.0,
        1.0));
    encoder->setVertexBytes(&dst_layer, sizeof(dst_layer), 0);
    BlitParams params = {
        .src_offset = {static_cast<f32>(src_origin.x()) /
                           static_cast<f32>(src_size.x()),
                       static_cast<f32>(src_origin.y()) /
                           static_cast<f32>(src_size.y())},
        .src_scale =
            float2(src_size) /
            float2({static_cast<f32>(src_impl->getTexture()->width()),
                    static_cast<f32>(src_impl->getTexture()->height())}),
    };
    encoder->setFragmentBytes(&params, sizeof(params), 0);
    encoder->setFragmentTexture(src_impl->getTexture(),
                                static_cast<NS::UInteger>(0));
    encoder->setFragmentSamplerState(
        linear_sampler,
        static_cast<NS::UInteger>(0)); // TODO: use the correct sampler

    encoder->drawPrimitives(MTL::PrimitiveTypeTriangle,
                            static_cast<NS::UInteger>(0),
                            static_cast<NS::UInteger>(3));
}

SamplerBase* Renderer::createSampler(const SamplerDescriptor& descriptor) {
    return new Sampler(device, descriptor);
}

ICommandBuffer* Renderer::createCommandBuffer() {
    return new CommandBuffer(command_queue);
}

RenderPassBase*
Renderer::createRenderPass(const RenderPassDescriptor& descriptor) {
    return new RenderPass(descriptor);
}

void Renderer::bindRenderPass(const RenderPassBase* render_pass) {
    state.render_pass = static_cast<const RenderPass*>(render_pass);
}

void Renderer::clearColor(ICommandBuffer* command_buffer, u32 render_target_id,
                          u32 layer, u8 mask, const uint4 color) {
    const auto command_buffer_impl =
        static_cast<CommandBuffer*>(command_buffer);
    auto texture =
        static_cast<TextureView*>(state.render_pass->getDescriptor()
                                      .color_targets[render_target_id]
                                      .texture);

    // HACK
    if (texture == nullptr) {
        ONCE(LOG_WARN(MetalRenderer, "Invalid color target at index {}",
                      render_target_id));
        return;
    }

    // TODO: layer
    ASSERT_DEBUG(layer == 0, MetalRenderer,
                 "Layered clears (layer: {}) not implemented", layer);

    auto encoder = getRenderCommandEncoder(command_buffer_impl);

    command_buffer_impl->setRenderPipelineState(clear_color_pipeline_cache.find(
        {.pixel_format = toMtlPixelFormat(texture->getDescriptor().format),
         .render_target_id = render_target_id,
         .mask = mask}));
    // TODO: set viewport and scissor
    encoder->setVertexBytes(&render_target_id, sizeof(render_target_id), 0);
    encoder->setFragmentBytes(&color, sizeof(color), 0);
    encoder->drawPrimitives(MTL::PrimitiveTypeTriangle,
                            static_cast<NS::UInteger>(0),
                            static_cast<NS::UInteger>(3));
}

void Renderer::clearDepth(ICommandBuffer* command_buffer, u32 layer,
                          const float value) {
    const auto command_buffer_impl =
        static_cast<CommandBuffer*>(command_buffer);
    auto texture = static_cast<TextureView*>(
        state.render_pass->getDescriptor().depth_stencil_target.texture);

    // HACK
    if (texture == nullptr) {
        ONCE(LOG_WARN(MetalRenderer, "Invalid depth target"));
        return;
    }

    const auto format = texture->getDescriptor().format;
    if (!toMtlPixelFormatInfo(format).has_depth) {
        ONCE(LOG_WARN(MetalRenderer,
                      "Texture format {} does not have a depth component",
                      format));
        return;
    }

    auto encoder = getRenderCommandEncoder(command_buffer_impl);

    command_buffer_impl->setRenderPipelineState(clear_depth_pipeline_cache.find(
        toMtlPixelFormat(texture->getDescriptor().format)));
    command_buffer_impl->setDepthStencilState(
        depth_stencil_state_always_and_write);
    // TODO: set viewport and scissor
    struct {
        u32 layer_id;
        float value;
    } params = {.layer_id = layer, .value = value};
    encoder->setVertexBytes(&params, sizeof(params), 0);
    encoder->drawPrimitives(MTL::PrimitiveTypeTriangle,
                            static_cast<NS::UInteger>(0),
                            static_cast<NS::UInteger>(3));
}

void Renderer::clearStencil(ICommandBuffer* command_buffer, u32 layer,
                            const u32 value) {
    (void)command_buffer;
    ONCE(LOG_FUNC_WITH_ARGS_NOT_IMPLEMENTED(
        MetalRenderer, "layer: {}, value: {:#x}", layer, value));
}

ShaderBase* Renderer::createShader(const ShaderDescriptor& descriptor) {
    return new Shader(device, descriptor);
}

PipelineBase* Renderer::createPipeline(const PipelineDescriptor& descriptor) {
    return new Pipeline(device, descriptor);
}

void Renderer::bindPipeline(const PipelineBase* pipeline) {
    state.pipeline = static_cast<const Pipeline*>(pipeline);
}

void Renderer::setDepthTestEnabled(bool enabled) {
    state.depth_test_enabled = enabled;
}

void Renderer::setDepthWriteEnabled(bool enabled) {
    state.depth_write_enabled = enabled;
}

void Renderer::setDepthCompareOp(engines::CompareOp op) {
    state.depth_compare_op = op;
}

void Renderer::setViewport(u32 index, const Viewport& viewport) {
    state.viewports[index] = viewport;
}

void Renderer::setScissor(u32 index, const Scissor& scissor) {
    state.scissors[index] = scissor;
}

void Renderer::bindVertexBuffer(const BufferView& buffer, u32 index) {
    state.vertex_buffers[index] = buffer;
}

void Renderer::bindIndexBuffer(const BufferView& index_buffer,
                               engines::IndexType index_type) {
    state.index_buffer = index_buffer;
    state.index_type = index_type;
}

void Renderer::bindUniformBuffer(const BufferView& buffer,
                                 ShaderType shader_type, u32 index) {
    // HACK
    if (shader_type == ShaderType::Count)
        return;

    state.uniform_buffers[static_cast<u32>(shader_type)][index] = buffer;
}

void Renderer::bindTexture(ITextureView* texture, SamplerBase* sampler,
                           ShaderType shader_type, u32 index) {
    // HACK
    if (shader_type == ShaderType::Count)
        return;

    state.textures[static_cast<u32>(shader_type)][index] = {
        .texture_view = static_cast<TextureView*>(texture),
        .sampler = static_cast<Sampler*>(sampler)};
}

void Renderer::unbindUniformBuffers(ShaderType shader_type) {
    // HACK
    if (shader_type == ShaderType::Count)
        return;

    state.uniform_buffers[static_cast<u32>(shader_type)] = {};
}

void Renderer::unbindTextures(ShaderType shader_type) {
    // HACK
    if (shader_type == ShaderType::Count)
        return;

    state.textures[static_cast<u32>(shader_type)] = {};
}

void Renderer::draw(ICommandBuffer* command_buffer,
                    const engines::PrimitiveType primitive_type,
                    const u32 start, const u32 count, const u32 base_instance,
                    const u32 instance_count) {
    const auto command_buffer_impl =
        static_cast<CommandBuffer*>(command_buffer);

    // Check for errors
    if (!canDraw())
        return;

    bindDrawState(command_buffer_impl);

    auto encoder = command_buffer_impl->getRenderCommandEncoderUnchecked();

    // Draw
    encoder->drawPrimitives(toMtlPrimitiveType(primitive_type), start, count,
                            instance_count, base_instance);
}

void Renderer::drawIndexed(ICommandBuffer* command_buffer,
                           const engines::PrimitiveType primitive_type,
                           const u32 start, const u32 count,
                           const u32 base_vertex, const u32 base_instance,
                           const u32 instance_count) {
    const auto command_buffer_impl =
        static_cast<CommandBuffer*>(command_buffer);

    // Check for errors
    if (!canDraw())
        return;

    bindDrawState(command_buffer_impl);

    auto encoder = command_buffer_impl->getRenderCommandEncoderUnchecked();

    // Draw
    auto index_buffer_mtl =
        static_cast<Buffer*>(state.index_buffer.getBase())->getBuffer();
    // TODO: is start used correctly?
    const auto index_buffer_offset =
        static_cast<u64>(start) * engines::getIndexTypeSize(state.index_type) +
        state.index_buffer.getOffset();
    encoder->drawIndexedPrimitives(toMtlPrimitiveType(primitive_type), count,
                                   toMtlIndexType(state.index_type),
                                   index_buffer_mtl, index_buffer_offset,
                                   instance_count, base_vertex, base_instance);
}

MTL::RenderCommandEncoder*
Renderer::getRenderCommandEncoder(CommandBuffer* command_buffer) const {
    return command_buffer->getRenderCommandEncoder(
        state.render_pass->getRenderPassDescriptor());
}

void Renderer::setRenderPipelineState(CommandBuffer* command_buffer) const {
    command_buffer->setRenderPipelineState(state.pipeline->getPipeline());
}

void Renderer::setDepthStencilState(CommandBuffer* command_buffer) {
    DepthStencilStateDescriptor descriptor{
        .depth_test_enabled = state.depth_test_enabled,
        .depth_write_enabled = state.depth_write_enabled,
        .depth_compare_op = state.depth_compare_op,
    };

    command_buffer->setDepthStencilState(
        depth_stencil_state_cache.find(descriptor));
}

void Renderer::setVertexBuffer(CommandBuffer* command_buffer, u32 index) {
    ASSERT_DEBUG(index < VERTEX_ARRAY_COUNT, MetalRenderer,
                 "Invalid vertex buffer index {}", index);

    const auto buffer = state.vertex_buffers[index];
    if (buffer.getBase() == nullptr)
        return;

    command_buffer->setBuffer(
        static_cast<Buffer*>(buffer.getBase())->getBuffer(), buffer.getOffset(),
        ShaderType::Vertex, getVertexBufferIndex(index));
}

void Renderer::setUniformBuffer(CommandBuffer* command_buffer,
                                ShaderType shader_type, u32 index) {
    // TODO: get the index from resource mapping

    ASSERT_DEBUG(index < CONST_BUFFER_BINDING_COUNT, MetalRenderer,
                 "Invalid uniform buffer index {}", index);

    const auto buffer =
        state.uniform_buffers[static_cast<u32>(shader_type)][index];
    if (buffer.getBase() == nullptr)
        return;

    command_buffer->setBuffer(
        static_cast<Buffer*>(buffer.getBase())->getBuffer(), buffer.getOffset(),
        shader_type, index);
}

void Renderer::setTexture(CommandBuffer* command_buffer, ShaderType shader_type,
                          u32 index) {
    const auto texture = state.textures[static_cast<u32>(shader_type)][index];
    if (texture.texture_view != nullptr)
        command_buffer->setTexture(texture.texture_view->getTexture(),
                                   shader_type, index);
    if (texture.sampler != nullptr)
        command_buffer->setSampler(texture.sampler->getSampler(), shader_type,
                                   index);
}

void Renderer::beginCapture() {
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
            fmt::format("{}/gpu_captures", CONFIG_INSTANCE.getAppDataPath());
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
        desc->setOutputURL(toNsurl(capture_path));
    }

    NS::Error* error = nullptr;
    capture_manager->startCapture(desc, &error);
    if (error != nullptr) {
        LOG_ERROR(MetalRenderer, "Failed to start GPU capture: {}",
                  error->localizedDescription()->utf8String());
    }
}

void Renderer::endCapture() {
    auto captureManager = MTL::CaptureManager::sharedCaptureManager();
    captureManager->stopCapture();
}

bool Renderer::canDraw() const {
    if (state.pipeline->getPipeline() == nullptr) {
        ONCE(LOG_WARN(MetalRenderer, "Pipeline not present, skipping draw"));
        return false;
    }

    return true;
}

void Renderer::bindDrawState(CommandBuffer* command_buffer) {
    auto encoder = getRenderCommandEncoder(command_buffer);

    // States
    setRenderPipelineState(command_buffer);
    setDepthStencilState(command_buffer);

    // TODO
    /*
    if (REGS_3D.cull_face_enabled) {
        SetCullMode(ToMtlCullMode(REGS_3D.cull_face_mode));
        SetFrontFaceWinding(ToMtlWinding(REGS_3D.front_face_winding));
    } else {
        SetCullMode(MTL::CullModeNone);
    }
    */

    // Viewport and scissor
    MTL::Viewport viewports[VIEWPORT_COUNT];
    MTL::ScissorRect scissors[VIEWPORT_COUNT];
    for (u32 i = 0; i < VIEWPORT_COUNT; i++) {
        // Viewport
        const auto& viewport = state.viewports[i];
        viewports[i] = MTL::Viewport(static_cast<f64>(viewport.rect.origin.x()),
                                     static_cast<f64>(viewport.rect.origin.y()),
                                     static_cast<f64>(viewport.rect.size.x()),
                                     static_cast<f64>(viewport.rect.size.y()),
                                     static_cast<f64>(viewport.depth_near),
                                     static_cast<f64>(viewport.depth_far));

        // Scissor
        const auto& scissor = state.scissors[i];
        scissors[i] = MTL::ScissorRect(scissor.origin.x(), scissor.origin.y(),
                                       scissor.size.x(), scissor.size.y());
    }
    encoder->setViewports(viewports, SIZEOF_ARRAY(viewports));
    encoder->setScissorRects(scissors, SIZEOF_ARRAY(scissors));

    // Resources
    for (u32 i = 0; i < VERTEX_ARRAY_COUNT; i++)
        setVertexBuffer(command_buffer, i);
    for (u32 shader_type = 0;
         shader_type < static_cast<usize>(ShaderType::Count); shader_type++) {
        for (u32 i = 0; i < CONST_BUFFER_BINDING_COUNT; i++)
            setUniformBuffer(command_buffer, ShaderType(shader_type), i);
    }
    // TODO: storage buffers
    for (u32 shader_type = 0;
         shader_type < static_cast<usize>(ShaderType::Count); shader_type++) {
        for (u32 i = 0; i < TEXTURE_COUNT; i++)
            setTexture(command_buffer, ShaderType(shader_type), i);
    }
}

} // namespace hydra::hw::tegra_x1::gpu::renderer::metal
