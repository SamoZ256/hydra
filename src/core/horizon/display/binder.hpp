#pragma once

#include "core/horizon/kernel/kernel.hpp"
#include "core/hw/tegra_x1/gpu/const.hpp"

namespace hydra::horizon::display {

struct GraphicBufferHeader {
    u32 magic;
    u32 width;
    u32 height;
    u32 stride;
    u32 format;
    u32 usage;

    u32 pid;
    u32 refcount;

    u32 fd_count;
    u32 int_count;
};

struct GraphicBuffer {
    GraphicBufferHeader header;
    hw::tegra_x1::gpu::NvGraphicsBuffer nv_buffer;
};

struct Buffer {
    bool initialized = false;
    bool queued = false;
    GraphicBuffer buffer;
};

struct NvFence {
    u32 id;
    u32 value;
} PACKED;

struct NvMultiFence {
    u32 num_fences;
    NvFence fences[4];
} PACKED;

enum class TransformFlags : u32 {
    None = 0,
    FlipH = BIT(0),
    FlipV = BIT(1),
    Rot90 = BIT(2),
    InverseDisplay = BIT(3),
    NoVSyncCapability = BIT(4),
    ReturnFrameNumber = BIT(5),
};
ENABLE_ENUM_BITWISE_OPERATORS(TransformFlags)

struct BqBufferInput {
    i64 timestamp;
    bool32 is_auto_timestamp; // TODO: correct?
    struct {
        i32 left;
        i32 top;
        i32 right;
        i32 bottom;
    } rect;
    i32 scaling_mode; // TODO: what is this?
    TransformFlags transform_flags;
    u32 sticky_transform; // TODO: what is this?
    u32 _unknown;
    u32 swap_interval; // TODO: float?
    NvMultiFence fence;
} PACKED;

struct BqBufferOutput {
    u32 width;
    u32 height;
    u32 transform_hint;
    u32 num_pending_buffers;
} PACKED;

constexpr usize MAX_BINDER_BUFFER_COUNT = 8; // TODO: what should this be?

struct AccumulatedTime {
    std::chrono::nanoseconds value{0ns};
    u32 sample_count{0};

    explicit operator bool() const { return sample_count != 0; }

    explicit operator f32() const {
        return f32(std::chrono::duration_cast<std::chrono::duration<f32>>(value)
                       .count()) /
               f32(sample_count);
    }

    AccumulatedTime& operator+=(const std::chrono::nanoseconds other) {
        value += other;
        sample_count += 1;
        return *this;
    }

    AccumulatedTime& operator+=(const AccumulatedTime& other) {
        value += other.value;
        sample_count += other.sample_count;
        return *this;
    }
};

struct Binder {
    using clock_t = std::chrono::steady_clock;

  public:
    // TODO: make private
    u32 weak_ref_count = 0;
    u32 strong_ref_count = 0;

    Binder() : event{new kernel::Event(true, "Display event")} {}

    // Buffers
    void AddBuffer(i32 slot, const GraphicBuffer& buff);
    i32 GetAvailableSlot();
    void QueueBuffer(System& system, i32 slot, const BqBufferInput& input);
    i32 ConsumeBuffer(BqBufferInput& out_input);
    void UnqueueAllBuffers();

    const GraphicBuffer& GetBuffer(i32 slot) {
        std::lock_guard lock(queue_mutex);
        return buffers[slot].buffer;
    }

    AccumulatedTime GetAccumulatedDT() {
        const auto tmp = accumulated_dt;
        accumulated_dt = {};
        return tmp;
    }

  private:
    kernel::Event* event;

    Buffer buffers[MAX_BINDER_BUFFER_COUNT];
    u32 buffer_count = 0;
    std::queue<std::pair<i32, BqBufferInput>> queued_buffers;
    std::mutex queue_mutex;
    std::condition_variable queue_cv;

    // Time
    clock_t::time_point last_queue_time{clock_t::now()};
    AccumulatedTime accumulated_dt;

  public:
    GETTER(event, GetEvent);
};

} // namespace hydra::horizon::display
