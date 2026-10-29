// Copyright 2026 Labsii Ltd. SPDX-License-Identifier: Apache-2.0
#pragma once
#include <atomic>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <dlfcn.h>
#include <mutex>
#include <string>
#include <sys/stat.h>
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
        // Match the lookup strategy used by OpenCL capability tools that are
        // known to work on Samsung firmware: try canonical absolute paths
        // before Android's generic sonames. Loading a generic dispatcher first
        // can cause later aliases to resolve to the same unusable handle.
        const char* paths[] = {
            "/system/vendor/lib64/libOpenCL.so",
            "/system/lib64/libOpenCL.so",
            "/system/vendor/lib64/egl/libGLES_mali.so",
            "/system/lib64/egl/libGLES_mali.so",
            "/system/vendor/lib64/libPVROCL.so",
            "/system/lib64/libPVROCL.so",
            "/vendor/lib64/libOpenCL.so",
            "/odm/lib64/libOpenCL.so",
            "/system_ext/lib64/libOpenCL.so",
            "/system_ext/lib64/libOpenCL_system.so",
            "/vendor/lib64/libOpenCL-pixel.so",
            "/odm/lib64/libOpenCL-pixel.so",
            "/vendor/lib64/libOpenCL_adreno.so",
            "/odm/lib64/libOpenCL_adreno.so",
            "/vendor/lib64/egl/libOpenCL.so",
            "/vendor/lib64/egl/libOpenCL_adreno.so",
            "/vendor/lib64/libGLES_mali.so",
            "/vendor/lib64/egl/libGLES_mali.so",
            "/vendor/lib64/libmali.so",
            "/vendor/lib64/libPVROCL.so",
            "libOpenCL.so",
            "libOpenCL_system.so",
            "libOpenCL-pixel.so",
            "libOpenCL_adreno.so",
            "libGLES_mali.so",
            "libmali.so",
            "libPVROCL.so",
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
        struct FileIdentity { dev_t device; ino_t inode; };
        std::vector<FileIdentity> triedFiles;
        // Some vendor drivers leave process-global state behind when unloaded.
        // Keep rejected candidates loaded locally for the lifetime of this
        // short-lived worker instead of unloading and reopening an alias.
        static std::vector<void*> retainedRejectedHandles;
        for (const char* path : paths) {
            bool hasIdentity = false;
            FileIdentity identity {};
            if (path[0] == '/') {
                struct stat info {};
                if (stat(path, &info) == 0) {
                    identity = {info.st_dev, info.st_ino};
                    hasIdentity = true;
                    bool duplicate = false;
                    for (const auto& tried : triedFiles) {
                        if (tried.device == identity.device && tried.inode == identity.inode) {
                            duplicate = true;
                            break;
                        }
                    }
                    if (duplicate) continue;
                }
            }
            Driver candidate;
            struct Reset {
                ~Reset() { initializing = nullptr; }
            } reset;
            initializing = &candidate;
            candidate.handle = dlopen(path, RTLD_LAZY | RTLD_LOCAL);
            if (!candidate.handle) {
                const char* error = dlerror();
                failures.emplace_back(std::string(path) + ": " + (error ? error : "could not load"));
                continue;
            }
            // Only mark an identity after this namespace could actually open
            // it. A denied alias must not suppress an accessible equivalent.
            if (hasIdentity) triedFiles.push_back(identity);
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
            retainedRejectedHandles.push_back(candidate.handle);
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
