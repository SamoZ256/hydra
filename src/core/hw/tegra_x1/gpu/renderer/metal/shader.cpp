#include "core/hw/tegra_x1/gpu/renderer/metal/shader.hpp"

#include "core/hw/tegra_x1/gpu/renderer/metal/renderer.hpp"

namespace hydra::hw::tegra_x1::gpu::renderer::metal {

Shader::Shader(const ShaderDescriptor& descriptor) : ShaderBase(descriptor) {
    // Options
    NS_STACK_SCOPED MTL::CompileOptions* options =
        MTL::CompileOptions::alloc()->init();
    if (false) // TODO: make this configurable
        options->setFastMathEnabled(true);
    if (true) // TODO: make this configurable
        options->setPreserveInvariance(true);

    // Library
    MTL::Library* library;
    switch (descriptor.backend) {
    case ShaderBackend::Msl: {
        // TODO: doesn't this copy the contents?
        std::string source;
        source.assign(descriptor.code.begin(), descriptor.code.end());

        NS::Error* error;
        library = METAL_RENDERER_INSTANCE.GetDevice()->newLibrary(
            ToNSString(source), options, &error);
        if (error) {
            LOG_ERROR(MetalRenderer, "Failed to create Metal library: {}",
                      error->localizedDescription()->utf8String());
            error->release(); // TODO: autorelease
            return;
        }
        break;
    }
    case ShaderBackend::Air: {
        auto dispatch_data =
            dispatch_data_create(descriptor.code.data(), descriptor.code.size(),
                                 dispatch_get_global_queue(0, 0),
                                 ^{
                                 });

        NS::Error* error;
        // TODO: options
        library = METAL_RENDERER_INSTANCE.GetDevice()->newLibrary(dispatch_data,
                                                                  &error);
        if (error) {
            LOG_ERROR(MetalRenderer, "Failed to create Metal library: {}",
                      error->localizedDescription()->utf8String());
            error->release(); // TODO: autorelease
            return;
        }
        break;
    }
    default:
        LOG_FATAL(MetalRenderer,
                  "Unsupported shader backend {} for the Metal renderer",
                  descriptor.backend);
        break;
    }

    // Function
    function = library->newFunction(ToNSString("main_"));
    library->release();
}

Shader::~Shader() { function->release(); }

} // namespace hydra::hw::tegra_x1::gpu::renderer::metal
