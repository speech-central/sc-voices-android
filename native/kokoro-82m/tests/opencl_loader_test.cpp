// Copyright 2026 Labsii Ltd. SPDX-License-Identifier: Apache-2.0
#define CL_TARGET_OPENCL_VERSION 120
#include <CL/cl.h>
#include <atomic>
#include <cassert>
#include <cstring>
#include <iostream>
#include <string>
#include <thread>
#include <vector>

static std::string scenario;
static int good_driver, bad_driver, wrapper_driver;
static bool enabled = false;
static bool fallback_tried = false;
static int reentrant_calls = 0;
static cl_int good_devices(cl_platform_id, cl_device_type, cl_uint, cl_device_id*, cl_uint* count) {
    if (count) *count = 7;
    return CL_SUCCESS;
}
static cl_int bad_devices(cl_platform_id, cl_device_type, cl_uint, cl_device_id*, cl_uint* count) {
    if (count) *count = 55;
    return CL_SUCCESS;
}
static cl_int good_platforms(cl_uint n, cl_platform_id* ids, cl_uint* count) {
    if (count) *count = 1;
    if (n && ids) ids[0] = reinterpret_cast<cl_platform_id>(&good_driver);
    return CL_SUCCESS;
}
static cl_int bad_platforms(cl_uint, cl_platform_id*, cl_uint* count) {
    // A driver can re-enter the exported forwarding API while initializing.
    cl_uint devices = 0;
    assert(clGetDeviceIDs(nullptr, CL_DEVICE_TYPE_ALL, 0, nullptr, &devices) == CL_SUCCESS);
    assert(devices == 55);
    ++reentrant_calls;
    if (count) *count = 0;
    return -1001;
}
static void noop() {}
static void* function(const char* name) {
    if (!std::strcmp(name, "clGetPlatformIDs")) return reinterpret_cast<void*>(&good_platforms);
    if (!std::strcmp(name, "clGetDeviceIDs")) return reinterpret_cast<void*>(&good_devices);
    return reinterpret_cast<void*>(&noop);
}
static void* load_pointer(const char* name) { return enabled ? function(name) : nullptr; }
static void enable_opencl() {
    enabled = true;
    cl_uint devices = 0;
    assert(clGetDeviceIDs(nullptr, CL_DEVICE_TYPE_ALL, 0, nullptr, &devices) == CL_SUCCESS);
    assert(devices == 7);
    ++reentrant_calls;
}
extern "C" void* __wrap_dlopen(const char* path, int) {
    const std::string p(path);
    if (scenario == "generic" && p == "libOpenCL.so") return &good_driver;
    if (scenario == "android16_system") {
        if (p == "libOpenCL.so") return &bad_driver;
        if (p == "libOpenCL_system.so") {
            fallback_tried = true;
            return &good_driver;
        }
    }
    if (scenario == "pixel_direct" && p == "libOpenCL-pixel.so") return &good_driver;
    if (scenario == "pixel_wrapper" && p == "libOpenCL-pixel.so") return &wrapper_driver;
    if (scenario == "mali_soname" && p == "libGLES_mali.so") return &good_driver;
    if (scenario == "powervr" && p == "libPVROCL.so") return &good_driver;
    if (scenario == "fallback" || scenario == "missing_symbol") {
        if (p == "libOpenCL.so") return &bad_driver;
        if (p == "/vendor/lib64/egl/libGLES_mali.so") {
            fallback_tried = true;
            return &good_driver;
        }
    }
    return nullptr;
}
extern "C" void* __wrap_dlsym(void* handle, const char* name) {
    if (handle == &wrapper_driver) {
        if (!std::strcmp(name, "enableOpenCL")) return reinterpret_cast<void*>(&enable_opencl);
        if (!std::strcmp(name, "loadOpenCLPointer")) return reinterpret_cast<void*>(&load_pointer);
        return nullptr;
    }
    if (!std::strcmp(name, "enableOpenCL") || !std::strcmp(name, "loadOpenCLPointer")) return nullptr;
    if (handle == &bad_driver) {
        if (scenario == "missing_symbol" && !std::strcmp(name, "clCreateContext")) return nullptr;
        if (!std::strcmp(name, "clGetPlatformIDs")) return reinterpret_cast<void*>(&bad_platforms);
        if (!std::strcmp(name, "clGetDeviceIDs")) return reinterpret_cast<void*>(&bad_devices);
    }
    return function(name);
}
extern "C" int __wrap_dlclose(void*) { return 0; }
extern "C" char* __wrap_dlerror() { return const_cast<char*>("test library unavailable"); }

int main(int argc, char** argv) {
    assert(argc == 2);
    scenario = argv[1];
    cl_uint count = 999;
    const cl_int result = clGetPlatformIDs(0, nullptr, &count);
    if (scenario == "unavailable") {
        assert(result == CL_INVALID_OPERATION && count == 0);
    } else {
        assert(result == CL_SUCCESS && count == 1);
        std::vector<std::thread> threads;
        for (int i = 0; i < 8; ++i) threads.emplace_back([] {
            for (int j = 0; j < 100; ++j) {
                cl_uint devices = 0;
                assert(clGetDeviceIDs(nullptr, CL_DEVICE_TYPE_ALL, 0, nullptr, &devices) == CL_SUCCESS);
                assert(devices == 7); // Never use a pointer cached from the rejected candidate.
            }
        });
        for (auto& t : threads) t.join();
        if (scenario == "pixel_wrapper") assert(enabled && reentrant_calls == 1);
        if (scenario == "android16_system") assert(fallback_tried && reentrant_calls == 1);
        if (scenario == "fallback") assert(fallback_tried && reentrant_calls == 1);
        if (scenario == "missing_symbol") assert(fallback_tried);
    }
    std::cout << "PASS " << scenario << '\n';
}
