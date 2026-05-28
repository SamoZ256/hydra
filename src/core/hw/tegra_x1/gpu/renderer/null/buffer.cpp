#include "core/hw/tegra_x1/gpu/renderer/null/buffer.hpp"

#include "core/hw/tegra_x1/gpu/renderer/null/renderer.hpp"

namespace hydra::hw::tegra_x1::gpu::renderer::null {

Buffer::Buffer(u64 size) : BufferBase(size) { buffer = new u8[size]; }
Buffer::~Buffer() { delete[] buffer; }

void Buffer::CopyFrom([[maybe_unused]] ICommandBuffer* command_buffer,
                      [[maybe_unused]] ITextureView* src,
                      [[maybe_unused]] const uint3 src_origin,
                      [[maybe_unused]] const uint3 src_size,
                      [[maybe_unused]] const Range<u32> src_levels,
                      [[maybe_unused]] const Range<u32> src_layers,
                      [[maybe_unused]] u64 dst_offset) {}

void Buffer::CopyFromImpl([[maybe_unused]] const uptr data,
                          [[maybe_unused]] u64 dst_offset,
                          [[maybe_unused]] u64 size_) {}
void Buffer::CopyFromImpl([[maybe_unused]] ICommandBuffer* command_buffer,
                          [[maybe_unused]] BufferBase* src,
                          [[maybe_unused]] u64 dst_offset,
                          [[maybe_unused]] u64 src_offset,
                          [[maybe_unused]] u64 size_) {}

} // namespace hydra::hw::tegra_x1::gpu::renderer::null
