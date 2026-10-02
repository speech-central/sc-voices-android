// Copyright 2026 Labsii Ltd. SPDX-License-Identifier: Apache-2.0
#pragma once
#include <CL/cl.h>
#include <algorithm>
#include <vector>
#include <stdexcept>

// Request-local ownership, including early returns at cancellation boundaries.
// Persistent weight caches must never register their buffers here.
class ClOwnedBuffers {
    std::vector<cl_mem> buffers_;
public:
    ClOwnedBuffers() = default;
    ClOwnedBuffers(const ClOwnedBuffers&) = delete;
    ~ClOwnedBuffers() { for (auto m : buffers_) if (m) clReleaseMemObject(m); }
    cl_mem own(cl_mem m) {
        if (!m) throw std::runtime_error("OpenCL buffer allocation failed");
        buffers_.push_back(m); return m;
    }
    void release(cl_mem m) {
        const auto it = std::find(buffers_.begin(), buffers_.end(), m);
        if (it != buffers_.end()) { if (*it) clReleaseMemObject(*it); buffers_.erase(it); }
    }
};
