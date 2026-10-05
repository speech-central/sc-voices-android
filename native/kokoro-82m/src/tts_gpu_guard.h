// Copyright 2026 Labsii Ltd. SPDX-License-Identifier: Apache-2.0
#pragma once
#include "cancellation.h"
#include <CL/cl.h>
#include <cstdlib>
#include <cstdio>
#include <cstdarg>
#include <ctime>
#include <unistd.h>

inline bool nnopt_tts_stage_diagnostics() {
    static const bool enabled = [] {
        const char* value = std::getenv("NNOPT_DIAGNOSTICS");
        return value && value[0] == '1';
    }();
    return enabled;
}

// Timestamp at the native source, not when the Java stderr reader eventually
// receives it. This distinguishes a paused reader from paused inference.
// No extra GPU synchronization, events, sampling thread or clocks in release.
inline void nnopt_tts_stage(const char* format, ...) {
    if (nnopt_tts_stage_diagnostics()) {
        char stage[256];
        va_list args;
        va_start(args, format);
        std::vsnprintf(stage, sizeof(stage), format, args);
        va_end(args);
        timespec monotonic{};
        clock_gettime(CLOCK_MONOTONIC, &monotonic);
        const long long mono_ms = (long long)monotonic.tv_sec * 1000 + monotonic.tv_nsec / 1000000;
        long long boot_ms = -1;
#ifdef CLOCK_BOOTTIME
        timespec boottime{};
        if (clock_gettime(CLOCK_BOOTTIME, &boottime) == 0)
            boot_ms = (long long)boottime.tv_sec * 1000 + boottime.tv_nsec / 1000000;
#endif
        std::fprintf(stderr, "KOKORO_STAGE command=%llu nativePid=%ld monoMs=%lld bootMs=%lld %s\n",
                     (unsigned long long)nnopt_tts_active_id.load(std::memory_order_relaxed),
                     (long)getpid(), mono_ms, boot_ms, stage);
        std::fflush(stderr);
    }
}

inline bool nnopt_tts_cooperative() {
    static const bool enabled = [] {
        const char* value = std::getenv("NNOPT_COOPERATIVE");
        return value && value[0] == '1';
    }();
    return enabled;
}

// At coarse model-block boundaries only. No polling, timers or per-kernel
// profiling. OpenCL cannot preempt a submitted graph; bounded submission lets
// CANCEL prevent the next block instead of queuing the whole sentence ahead.
inline int nnopt_tts_gpu_checkpoint(cl_command_queue queue) {
    if (nnopt_tts_cooperative() && clFinish(queue) != CL_SUCCESS) return -1;
    return nnopt_tts_cancelled() ? -2 : 0;
}
