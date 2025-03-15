#pragma once

#include "hw/tegra_x1/gpu/engines/const.hpp"

namespace Hydra::HW::TegraX1::GPU::Renderer {

class TextureBase;
class ShaderBase;

struct BufferDescriptor {
    uptr ptr;
    usize size;
};

struct TextureDescriptor {
    uptr ptr;
    SurfaceFormat surface_format;
    NvKind kind;
    usize width;
    usize height;
    usize block_height_log2;
    usize stride;
    // TODO: more
};

struct RenderTargetDescriptor {
    TextureBase* texture;
    bool load_action_clear = false;
    union {
        float color[4];
        struct {
            bool clear_depth;
            bool clear_stencil;
            float depth;
            u32 stencil;
        };
    } clear_data;
};

struct RenderPassDescriptor {
    RenderTargetDescriptor color_targets[COLOR_TARGET_COUNT];
    RenderTargetDescriptor depth_stencil_target;
};

struct VertexArray {
    bool enable;
    u32 stride;
    bool is_per_instance;
    u32 divisor;
};

struct VertexState {
    Engines::VertexAttribState vertex_attrib_states[VERTEX_ATTRIB_COUNT];
    VertexArray vertex_arrays[VERTEX_ARRAY_COUNT];
};

enum class ShaderType {
    Vertex,
    Fragment,

    Count,
};

struct ShaderDescriptor {
    ShaderType type;
    std::vector<u8> code;
};

struct PipelineDescriptor {
    ShaderBase* shaders[usize(ShaderType::Count)];
    VertexState vertex_state;
    // TODO: other stuff
};

} // namespace Hydra::HW::TegraX1::GPU::Renderer

ENABLE_ENUM_FORMATTING(Hydra::HW::TegraX1::GPU::Renderer::ShaderType, Vertex,
                       "vertex", Fragment, "fragment", Count, "invalid")
