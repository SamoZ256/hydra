#pragma once

#include <Metal/Metal.hpp>
#include <QuartzCore/QuartzCore.hpp>

#include "core/hw/tegra_x1/gpu/renderer/const.hpp"

namespace hydra::hw::tegra_x1::gpu::renderer::metal {

// Cast from const char* to NS::String*
inline NS::String* ToNSString(const char* str) {
    return NS::String::string(str, NS::ASCIIStringEncoding);
}

// Cast from std::string to NS::String*
inline NS::String* ToNSString(const std::string& str) {
    return ToNSString(str.c_str());
}

// Cast from const char* to NS::URL*
inline NS::URL* ToNSURL(const char* str) {
    return NS::URL::fileURLWithPath(ToNSString(str));
}

// Cast from std::string to NS::URL*
inline NS::URL* ToNSURL(const std::string& str) { return ToNSURL(str.c_str()); }

inline NS::String* GetLabel(const std::string_view label,
                            const void* identifier) {
    return ToNSString(
        fmt::format("{} ({})", label,
                    std::to_string(reinterpret_cast<uintptr_t>(identifier))));
}

constexpr usize BUFFER_COUNT = 31;
constexpr usize TEXTURE_COUNT = 31;
constexpr usize SAMPLER_COUNT = 16;

inline u32 GetVertexBufferIndex(u32 index) { return BUFFER_COUNT - index - 1; }

inline MTL::Library* CreateLibraryFromSource(MTL::Device* device,
                                             const std::string_view source) {
    NS::Error* error;
    // TODO: don't construct a new string?
    MTL::Library* library =
        device->newLibrary(ToNSString(std::string(source)), nullptr, &error);
    if (error) {
        LOG_ERROR(GPU, "Failed to create library: {}",
                  error->localizedDescription()->utf8String());
        error->release(); // TODO: release?
        return nullptr;
    }

    return library;
}

inline MTL::Function* CreateFunctionFromSource(MTL::Device* device,
                                               const std::string_view source,
                                               const std::string_view name) {
    auto library = CreateLibraryFromSource(device, source);
    // TODO: don't construct a new string?
    auto function = library->newFunction(ToNSString(std::string(name)));
    library->release();

    return function;
}

} // namespace hydra::hw::tegra_x1::gpu::renderer::metal
