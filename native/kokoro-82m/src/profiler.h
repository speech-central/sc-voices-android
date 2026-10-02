// Lightweight OpenCL event-based kernel profiler.
//
// Activation: set env `NNOPT_PROFILE=1` at runtime. When unset (the
// default benchmark/release path) all helpers compile to nullptr / no-op.
//
// Usage at an enqueue site:
//   cl_event* evt = KernelProfiler::event_for("op_attention_scores");
//   clEnqueueNDRangeKernel(queue, k, 1, nullptr, &gws, &lws, 0, nullptr, evt);
//
// The profiler stashes the event with its label. After generate() finishes,
// call KernelProfiler::process_pending() to read CL_PROFILING_COMMAND_START
// and CL_PROFILING_COMMAND_END from each captured event and accumulate into
// per-label totals. Then dump_summary() prints a sorted breakdown to stderr.
//
// The queue must have CL_QUEUE_PROFILING_ENABLE set (OpenCLContext enables it
// only when profiling is requested — see opencl_context.cpp). Per-event overhead is
// ~10-30 µs of host work plus ~0 GPU work — fine for one-off profile
// runs, never on the benchmark path.

#pragma once

#include <CL/cl.h>
#include <cstdlib>
#include <string>
#include <stdexcept>

// Failure propagation is independent of profiling; ignored launch errors must
// not become a successful readback of stale audio from a previous request.
inline cl_int nnopt_require_enqueue(cl_int error) {
    if (error != CL_SUCCESS) throw std::runtime_error("OpenCL operation failed: " + std::to_string(error));
    return error;
}

namespace KernelProfiler {

inline bool enabled() {
    static int e = -1;
    if (e == -1) {
        const char* env = std::getenv("NNOPT_PROFILE");
        e = (env && env[0] != '0') ? 1 : 0;
    }
    return e == 1;
}

// Returns a cl_event* to pass to clEnqueueNDRangeKernel (or CLBlast). When
// profiling is off returns nullptr and the enqueue call sees no event arg.
cl_event* event_for(const char* label);

// Drain all captured events: read profiling info, accumulate, release.
// Safe to call multiple times — drains whatever is pending. The next
// process_pending() picks up new events captured since.
void process_pending();

// Print the per-label summary to stderr (sorted by total time desc).
// Implicitly calls process_pending() first so any tail events are counted.
void dump_summary();

// Reset all accumulated state. Useful between prefill and decode if you want
// per-phase numbers; we don't do that today (single combined dump).
void reset();

}  // namespace KernelProfiler

// Drop-in replacement for clEnqueueNDRangeKernel that, when NNOPT_PROFILE=1,
// attaches a profiling event auto-labeled with the kernel's function name.
// Zero overhead when profiling is off (direct passthrough).
cl_int nnopt_enqueue_profiled(cl_command_queue queue, cl_kernel kernel,
                              cl_uint work_dim, const size_t* gwo,
                              const size_t* gws, const size_t* lws,
                              cl_uint num_wait, const cl_event* wait_list,
                              cl_event* evt);

// These operations feed mandatory model data. Do not continue after a failed
// argument/upload/readback and synthesize from old or zero-filled buffers.
// No profiling or synchronization is introduced on the successful path.
#define clSetKernelArg(...) nnopt_require_enqueue(clSetKernelArg(__VA_ARGS__))
#define clEnqueueReadBuffer(...) nnopt_require_enqueue(clEnqueueReadBuffer(__VA_ARGS__))
#define clEnqueueWriteBuffer(...) nnopt_require_enqueue(clEnqueueWriteBuffer(__VA_ARGS__))
#define clEnqueueCopyBuffer(...) nnopt_require_enqueue(clEnqueueCopyBuffer(__VA_ARGS__))
#define clEnqueueFillBuffer(...) nnopt_require_enqueue(clEnqueueFillBuffer(__VA_ARGS__))
