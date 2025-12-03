#include "core/hw/tegra_x1/gpu/renderer/index_cache.hpp"

#include "core/hw/tegra_x1/gpu/gpu.hpp"
#include "core/hw/tegra_x1/gpu/renderer/buffer_base.hpp"

namespace hydra::hw::tegra_x1::gpu::renderer {

namespace {

inline void decode_u8_indices(uptr in, uptr out, u32 count) {
    for (u32 i = 0; i < count; i++)
        reinterpret_cast<u16*>(out)[i] = reinterpret_cast<u8*>(in)[i];
}

inline u32 get_index_count_quads_to_triangles(u32 count) {
    return count / 4 * 6;
}

inline engines::PrimitiveType get_primitive_type_quads_to_triangles() {
    return engines::PrimitiveType::Triangles;
}

inline u32 get_index_count_triangle_fan_to_triangle_strip(u32 count) {
    return count;
}

inline engines::PrimitiveType
get_primitive_type_triangle_fan_to_triangle_strip() {
    return engines::PrimitiveType::TriangleStrip;
}

#define GET_INDEX(idx, index)                                                  \
    switch (index_type) {                                                      \
    case engines::IndexType::UInt8:                                            \
        idx = GET_INDEX_IMPL(u8, index);                                       \
        break;                                                                 \
    case engines::IndexType::UInt16:                                           \
        idx = GET_INDEX_IMPL(u16, index);                                      \
        break;                                                                 \
    case engines::IndexType::UInt32:                                           \
        idx = GET_INDEX_IMPL(u32, index);                                      \
        break;                                                                 \
    default:                                                                   \
        break;                                                                 \
    }

#define ADD_INDEX(index)                                                       \
    switch (index_type) {                                                      \
    case engines::IndexType::UInt8:                                            \
        *reinterpret_cast<u8*>(out) = index;                                   \
        out += sizeof(u16);                                                    \
        break;                                                                 \
    case engines::IndexType::UInt16:                                           \
        *reinterpret_cast<u16*>(out) = index;                                  \
        out += sizeof(u16);                                                    \
        break;                                                                 \
    case engines::IndexType::UInt32:                                           \
        *reinterpret_cast<u32*>(out) = index;                                  \
        out += sizeof(u32);                                                    \
        break;                                                                 \
    default:                                                                   \
        break;                                                                 \
    }

#define ADD_INDEX_AUTO(index)                                                  \
    {                                                                          \
        u32 idx;                                                               \
        GET_INDEX(idx, index);                                                 \
        ADD_INDEX(index);                                                      \
    }

#define DEFINE_DECODER_PROTOTYPE(name)                                         \
    void decode_##name##_auto(uptr in_unused, uptr out,                        \
                              engines::IndexType index_type, u32 count)
#define GET_INDEX_IMPL(type, index) (index)

#include "core/hw/tegra_x1/gpu/renderer/index_decoders.inc"

#undef GET_INDEX_IMPL
#undef DEFINE_DECODER_PROTOTYPE

#define DEFINE_DECODER_PROTOTYPE(name)                                         \
    void decode_##name(uptr in, uptr out, engines::IndexType index_type,       \
                       u32 count)
#define GET_INDEX_IMPL(type, index) reinterpret_cast<type*>(in)[index]

#include "core/hw/tegra_x1/gpu/renderer/index_decoders.inc"

#undef GET_INDEX_IMPL
#undef DEFINE_DECODER_PROTOTYPE

} // namespace

IndexCache::~IndexCache() {
    for (auto& [key, index_buffer] : cache)
        RENDERER_INSTANCE.FreeTemporaryBuffer(index_buffer);
}

BufferBase* IndexCache::Decode(const IndexDescriptor& descriptor,
                               engines::IndexType& out_type,
                               engines::PrimitiveType& out_primitive_type,
                               u32& out_count) {
#define PRIMITIVE_TYPE_SWITCH(macro, u8_index_macro)                           \
    switch (descriptor.primitive_type) {                                       \
    case engines::PrimitiveType::Quads:                                        \
        /* TODO: check for quads support */                                    \
        macro(quads_to_triangles);                                             \
        break;                                                                 \
    case engines::PrimitiveType::TriangleFan:                                  \
        /* TODO: check for triangle fan support */                             \
        macro(triangle_fan_to_triangle_strip);                                 \
        break;                                                                 \
    default:                                                                   \
        /* TODO: check for U8 support */                                       \
        if (descriptor.type == engines::IndexType::UInt8) {                    \
            u8_index_macro();                                                  \
            break;                                                             \
        } else {                                                               \
            return descriptor.src_index_buffer;                                \
        }                                                                      \
    }

    out_type = descriptor.type;
    out_primitive_type = descriptor.primitive_type;
    out_count = descriptor.count;

#define GET_PARAMS(name)                                                       \
    out_primitive_type = get_primitive_type_##name();                          \
    out_count = get_index_count_##name(descriptor.count)

#define GET_PARAMS_U8_INDEX() out_type = engines::IndexType::UInt16;

    // Returns src index buffer if no decoding is needed
    PRIMITIVE_TYPE_SWITCH(GET_PARAMS, GET_PARAMS_U8_INDEX)

    switch (out_count) {
    case 0 ... 0xff:
        // TODO: check for u8 support
        out_type = engines::IndexType::UInt16;
        break;
    case 0x100 ... 0xffff:
        out_type = engines::IndexType::UInt16;
        break;
    case 0x10000 ... 0xffffffff:
        out_type = engines::IndexType::UInt32;
        break;
    }

    u64 hash = Hash(descriptor);
    auto& index_buffer = cache[hash];
    if (index_buffer)
        return index_buffer;

    const auto index_size = get_index_type_size(out_type);
    index_buffer =
        RENDERER_INSTANCE.AllocateTemporaryBuffer(out_count * index_size);
    uptr in_ptr = 0x0;
    if (descriptor.src_index_buffer)
        in_ptr = descriptor.src_index_buffer->GetDescriptor().ptr;
    auto out_ptr = index_buffer->GetDescriptor().ptr;

#define DECODE(name) decode_##name(in_ptr, out_ptr, out_type, descriptor.count)

#define DECODE_MACRO_AUTO(name) DECODE(name##_auto);
#define DECODE_MACRO(name) DECODE(name);

#define DECODE_MACRO_AUTO_U8_INDEX() unreachable();
#define DECODE_MACRO_U8_INDEX()                                                \
    decode_u8_indices(in_ptr, out_ptr, descriptor.count);

    if (descriptor.src_index_buffer == nullptr) {
        PRIMITIVE_TYPE_SWITCH(DECODE_MACRO_AUTO, DECODE_MACRO_AUTO_U8_INDEX)
    } else {
        PRIMITIVE_TYPE_SWITCH(DECODE_MACRO, DECODE_MACRO_U8_INDEX)
    }

    return index_buffer;
} // namespace hydra::hw::tegra_x1::gpu::renderer

u64 IndexCache::Hash(const IndexDescriptor& descriptor) {
    u64 hash = 0;
    hash += (u64)descriptor.type;
    hash = std::rotl(hash, 3);
    // TODO: don't always hash primitive type
    hash += (u64)descriptor.primitive_type;
    hash = std::rotl(hash, 5);
    hash += (u64)descriptor.src_index_buffer; // TODO: don't hash it like this
    hash = std::rotl(hash, 47);
    hash += (u64)descriptor.count;
    hash = std::rotl(hash, 17);

    return hash;
}

} // namespace hydra::hw::tegra_x1::gpu::renderer
