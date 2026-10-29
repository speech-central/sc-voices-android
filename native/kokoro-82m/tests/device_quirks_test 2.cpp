#include "device_quirks.h"
#include <cassert>
#include <cstdio>

static cl_ulong device_memory = 32768, kernel_memory = 16384;
static size_t device_group = 1024, kernel_group = 1024, y_limit = 1024;
static bool query_fails = false;
static const char* device_name = "PowerVR GE8320";

extern "C" cl_int clGetDeviceInfo(cl_device_id, cl_device_info key, size_t, void* result, size_t*) {
    if (query_fails) return CL_INVALID_DEVICE;
    switch (key) {
        case CL_DEVICE_LOCAL_MEM_SIZE: *static_cast<cl_ulong*>(result) = device_memory; break;
        case CL_DEVICE_MAX_WORK_GROUP_SIZE: *static_cast<size_t*>(result) = device_group; break;
        case CL_DEVICE_MAX_WORK_ITEM_DIMENSIONS: *static_cast<cl_uint*>(result) = 3; break;
        case CL_DEVICE_MAX_WORK_ITEM_SIZES: {
            auto sizes = static_cast<size_t*>(result);
            sizes[0] = 1024; sizes[1] = y_limit; sizes[2] = 64;
            break;
        }
        case CL_DEVICE_NAME: std::strcpy(static_cast<char*>(result), device_name); break;
        default: return CL_INVALID_VALUE;
    }
    return CL_SUCCESS;
}

extern "C" cl_int clGetKernelWorkGroupInfo(cl_kernel, cl_device_id, cl_kernel_work_group_info key,
                                            size_t, void* result, size_t*) {
    if (query_fails) return CL_INVALID_KERNEL;
    if (key == CL_KERNEL_LOCAL_MEM_SIZE) *static_cast<cl_ulong*>(result) = kernel_memory;
    else if (key == CL_KERNEL_WORK_GROUP_SIZE) *static_cast<size_t*>(result) = kernel_group;
    else return CL_INVALID_VALUE;
    return CL_SUCCESS;
}

int main() {
    auto dev = reinterpret_cast<cl_device_id>(1);
    auto kernel = reinterpret_cast<cl_kernel>(2);
    assert(nnopt_kernel_local_fits(kernel, dev, 384));
    device_memory = 4096;
    assert(!nnopt_kernel_local_fits(kernel, dev, 384));
    device_memory = 32768;
    device_group = 256;
    assert(!nnopt_kernel_local_fits(kernel, dev, 384));
    device_group = 1024; kernel_group = 256;
    assert(!nnopt_kernel_local_fits(kernel, dev, 384));
    kernel_group = 1024; y_limit = 256;
    assert(!nnopt_kernel_local_fits(kernel, dev, 384));
    y_limit = 1024; query_fails = true;
    assert(!nnopt_kernel_local_fits(kernel, dev, 384));
    query_fails = false;
    assert(nnopt_is_powervr_rogue(dev));
    device_name = "PowerVR DXT";
    assert(!nnopt_is_powervr_rogue(dev));
    device_name = "Mali-G715";
    assert(!nnopt_is_powervr_rogue(dev));
    puts("PASS device local-memory and work-group limits");
}
