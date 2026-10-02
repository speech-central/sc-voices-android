#pragma once

#include <atomic>
#include <cstdint>

// The serve-stream input reader owns writes to this flag while the inference
// thread reads it at safe graph boundaries.  OpenCL cannot abort a submitted
// kernel, so this is deliberately cooperative: it prevents the next expensive
// stage from being enqueued and returns control as soon as the current stage
// completes.
// Monotonically increasing IDs within one worker. A cancellation received
// before dequeue stays effective; an old cancellation cannot poison a new SAY.
inline std::atomic<uint64_t> nnopt_tts_active_id{0};
inline std::atomic<uint64_t> nnopt_tts_cancel_through{0};
inline std::atomic_bool nnopt_tts_input_closed{false};

inline bool nnopt_tts_cancelled() {
    const auto id = nnopt_tts_active_id.load(std::memory_order_acquire);
    return nnopt_tts_input_closed.load(std::memory_order_acquire) ||
        (id != 0 && id <= nnopt_tts_cancel_through.load(std::memory_order_acquire));
}

inline void nnopt_tts_begin(uint64_t id) {
    nnopt_tts_active_id.store(id, std::memory_order_release);
}

inline void nnopt_tts_cancel(uint64_t id) {
    auto previous = nnopt_tts_cancel_through.load(std::memory_order_relaxed);
    while (id > previous && !nnopt_tts_cancel_through.compare_exchange_weak(
        previous, id, std::memory_order_release, std::memory_order_relaxed)) {}
}
