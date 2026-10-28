// Copyright 2026 Labsii Ltd. SPDX-License-Identifier: Apache-2.0
#pragma once
#include <atomic>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <dlfcn.h>
#include <mutex>
#include <string>
#include <vector>

namespace kokoro_cl_loader {
using Lookup = void* (*)(const char*);
struct Driver {
    void* handle = nullptr;
    Lookup lookup = nullptr;
};

// Driver initialization can call our forwarding symbols. Such calls must use
// the candidate being initialized, without re-entering call_once or caching it.
static thread_local Driver* initializing = nullptr;

static void* lookup(const Driver& driver, const char* name) {
    if (!driver.handle) return nullptr;
    void* result = dlsym(driver.handle, name);
    return result ? result : driver.lookup ? driver.lookup(name) : nullptr;
}

static Driver& driver() {
    if (initializing) return *initializing;
    static Driver selected;
    static std::once_flag once;
    std::call_once(once, [] {
        const char* paths[] = {
            "libOpenCL.so", "libOpenCL_system.so", "libOpenCL-pixel.so", "libOpenCL_adreno.so",
            "libGLES_mali.so", "libmali.so", "libPVROCL.so",
            "/vendor/lib64/libOpenCL.so", "/vendor/lib64/libOpenCL-pixel.so",
            "/vendor/lib64/libOpenCL_adreno.so",
            "/vendor/lib64/egl/libOpenCL.so", "/vendor/lib64/egl/libOpenCL_adreno.so",
            "/odm/lib64/libOpenCL.so", "/odm/lib64/libOpenCL-pixel.so",
            "/odm/lib64/libOpenCL_adreno.so", "/system/vendor/lib64/libOpenCL.so",
            "/system_ext/lib64/libOpenCL.so", "/system_ext/lib64/libOpenCL_system.so",
            "/system/lib64/libOpenCL.so", "/system/lib64/libOpenCL_system.so",
            "/vendor/lib64/libGLES_mali.so", "/vendor/lib64/egl/libGLES_mali.so",
            "/system/vendor/lib64/egl/libGLES_mali.so", "/system/lib64/egl/libGLES_mali.so",
            "/vendor/lib64/libPVROCL.so", "/system/vendor/lib64/libPVROCL.so",
        };
        const char* required[] = {
            "clGetPlatformIDs", "clGetPlatformInfo", "clGetDeviceIDs", "clGetDeviceInfo",
            "clCreateContext", "clReleaseContext", "clCreateCommandQueue", "clReleaseCommandQueue",
            "clGetCommandQueueInfo", "clCreateBuffer", "clReleaseMemObject",
            "clCreateProgramWithSource", "clBuildProgram", "clGetProgramBuildInfo",
            "clCreateKernel", "clReleaseKernel", "clReleaseProgram", "clSetKernelArg",
            "clEnqueueNDRangeKernel", "clEnqueueReadBuffer", "clEnqueueWriteBuffer",
            "clEnqueueCopyBuffer", "clGetKernelWorkGroupInfo", "clFlush", "clFinish",
        };
        std::vector<std::string> failures;
        for (const char* path : paths) {
            Driver candidate;
            struct Reset {
                ~Reset() { initializing = nullptr; }
            } reset;
            initializing = &candidate;
            candidate.handle = dlopen(path, RTLD_NOW | RTLD_GLOBAL);
            if (!candidate.handle) {
                const char* error = dlerror();
                failures.emplace_back(std::string(path) + ": " + (error ? error : "could not load"));
                continue;
            }
            auto enable = reinterpret_cast<void (*)()>(dlsym(candidate.handle, "enableOpenCL"));
            candidate.lookup = reinterpret_cast<Lookup>(dlsym(candidate.handle, "loadOpenCLPointer"));
            if (enable) enable();
            if (!candidate.lookup) {
                candidate.lookup = reinterpret_cast<Lookup>(dlsym(candidate.handle, "loadOpenCLPointer"));
            }

            const char* missing = nullptr;
            for (const char* name : required) {
                if (!lookup(candidate, name)) { missing = name; break; }
            }
            auto platforms = reinterpret_cast<decltype(&clGetPlatformIDs)>(lookup(candidate, "clGetPlatformIDs"));
            cl_uint count = 0;
            cl_int status = CL_INVALID_OPERATION;
            if (!missing && platforms && platforms != &clGetPlatformIDs) {
                status = platforms(0, nullptr, &count);
            }
            if (!missing && status == CL_SUCCESS && count > 0 && count <= 64) {
                selected = candidate;
                fprintf(stderr, "KOKORO_OPENCL selected=%s wrapper=%s\n", path, candidate.lookup ? "yes" : "no");
                return;
            }
            failures.emplace_back(std::string(path) + (missing ? std::string(": missing ") + missing :
                ": platform enumeration failed code=" + std::to_string(status) + " count=" + std::to_string(count)));
            dlclose(candidate.handle);
        }
        for (const auto& failure : failures) {
            fprintf(stderr, "KOKORO_OPENCL %.*s\n", 768, failure.c_str());
        }
        fprintf(stderr, "KOKORO_OPENCL_FAILURE No usable OpenCL driver could be loaded.\n");
    });
    return selected;
}

template <typename Function>
static Function resolve(const char* name, std::atomic<Function>& cached, Function self) {
    if (!initializing) {
        if (auto function = cached.load(std::memory_order_acquire)) return function;
    }
    auto function = reinterpret_cast<Function>(lookup(driver(), name));
    if (function == self) return nullptr;
    if (function && !initializing) cached.store(function, std::memory_order_release);
    return function;
}
} // namespace kokoro_cl_loader
