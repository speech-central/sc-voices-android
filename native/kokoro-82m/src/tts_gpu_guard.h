// Copyright 2026 Labsii Ltd. SPDX-License-Identifier: Apache-2.0
#pragma once
#include "cancellation.h"
#include <CL/cl.h>
#include <cstdlib>

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
