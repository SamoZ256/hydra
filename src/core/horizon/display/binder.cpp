#include "core/horizon/display/binder.hpp"

namespace hydra::horizon::display {

void Binder::AddBuffer(i32 slot, const GraphicBuffer& buff) {
    std::lock_guard<std::mutex> lock(queue_mutex);
    buffers[slot].initialized = true;
    buffers[slot].buffer = buff;
    buffer_count++;
}

i32 Binder::GetAvailableSlot() {
    // Wait for a slot to become available
    std::unique_lock<std::mutex> lock(queue_mutex);
    queue_cv.wait(lock, [&] { return queued_buffers.size() != buffer_count; });

    // Find an available slot
    i32 slot = -1;
    for (i32 i = 0; i < MAX_BINDER_BUFFER_COUNT; i++) {
        if (buffers[i].initialized && !buffers[i].queued) {
            if (slot == -1) {
                // Consume the first slot
                slot = i;
            } else {
                // We know that there is another available slot, so we can
                // keep the event signalled
                return slot;
            }
        }
    }

    // If we reach here, it means that there won't be a slot available the
    // next time, so clear the event
    event->Clear();

    // TODO: remove this?
    if (slot == -1)
        LOG_ERROR(Horizon, "No available slots");

    return slot;
}

void Binder::QueueBuffer(i32 slot, const BqBufferInput& input) {
    std::lock_guard<std::mutex> lock(queue_mutex);
    queued_buffers.push({slot, input});
    buffers[slot].queued = true;

    // Time
    const auto now = clock_t::now();
    dt_queue.push_back(now - last_queue_time);
    last_queue_time = now;

    queue_cv.notify_all();
}

i32 Binder::ConsumeBuffer(BqBufferInput& out_input,
                          std::vector<std::chrono::nanoseconds>& out_dt_list) {
    // Wait for a buffer to become available
    std::unique_lock<std::mutex> lock(queue_mutex);
    // TODO: should we wait?
    // queue_cv.wait_for(lock, std::chrono::milliseconds(67),
    //                  [&] { return !queued_buffers.empty(); });

    if (queued_buffers.empty())
        return -1;

    // Get the first queued buffer
    const auto [slot, input] = queued_buffers.front();
    queued_buffers.pop();
    buffers[slot].queued = false;

    out_input = input;

    // Time
    out_dt_list = dt_queue;
    dt_queue.clear();

    queue_cv.notify_all();

    // Signal event
    event->Signal();

    return slot;
}

void Binder::UnqueueAllBuffers() {
    // Wait for a buffer to become available
    std::unique_lock<std::mutex> lock(queue_mutex);

    // Unqueue all
    while (!queued_buffers.empty()) {
        const auto [slot, input] = queued_buffers.front();
        queued_buffers.pop();
        buffers[slot].queued = false;
    }

    queue_cv.notify_all();

    // Signal event
    event->Signal();
}

} // namespace hydra::horizon::display
