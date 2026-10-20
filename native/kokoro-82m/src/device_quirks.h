// Device checks for non-Adreno GPUs (PowerVR Rogue GE8320).
#pragma once

#include <CL/cl.h>
#include <cstring>
#include <vector>

// True when the kernel's static __local arrays fit the device. The LDS-cached
// conv kernels declare 8-32 KB caches sized for Adreno; PowerVR GE8320 has 4 KB,
// where the launch fails with CL_OUT_OF_RESOURCES (-5) and the F0/N/noise convs
// silently produced nothing. Callers fall back to the uncached kernel.
inline bool nnopt_kernel_local_fits(cl_kernel k, cl_device_id dev, size_t local_y = 0) {
    if (!k || !dev) return false;
    cl_ulong dev_lm = 0, k_lm = 0;
    if (clGetDeviceInfo(dev, CL_DEVICE_LOCAL_MEM_SIZE, sizeof(dev_lm), &dev_lm, nullptr) != CL_SUCCESS)
        return false;
    if (clGetKernelWorkGroupInfo(k, dev, CL_KERNEL_LOCAL_MEM_SIZE, sizeof(k_lm), &k_lm, nullptr) != CL_SUCCESS)
        return false;
    if (k_lm > dev_lm) return false;
    if (local_y == 0) return true;
    // The cached decoder launches {1, 384}; some GPUs support only 256 lanes.
    size_t kernel_max = 0, device_max = 0;
    cl_uint dimensions = 0;
    if (clGetKernelWorkGroupInfo(k, dev, CL_KERNEL_WORK_GROUP_SIZE, sizeof(kernel_max), &kernel_max, nullptr) != CL_SUCCESS ||
        clGetDeviceInfo(dev, CL_DEVICE_MAX_WORK_GROUP_SIZE, sizeof(device_max), &device_max, nullptr) != CL_SUCCESS ||
        clGetDeviceInfo(dev, CL_DEVICE_MAX_WORK_ITEM_DIMENSIONS, sizeof(dimensions), &dimensions, nullptr) != CL_SUCCESS ||
        dimensions < 2 || dimensions > 16 || local_y > kernel_max || local_y > device_max) return false;
    std::vector<size_t> limits(dimensions);
    return clGetDeviceInfo(dev, CL_DEVICE_MAX_WORK_ITEM_SIZES, limits.size() * sizeof(size_t), limits.data(), nullptr) == CL_SUCCESS &&
        limits[0] >= 1 && local_y <= limits[1];
}

// PowerVR Rogue (Vivo Y21's GE8320). CL_DEVICE_NAME there is "PowerVR GE8320";
// the newer PowerVR DXT (Tensor G5) is a different architecture and is NOT matched.
inline bool nnopt_is_powervr_rogue(cl_device_id dev) {
    char name[128] = {0};
    if (!dev || clGetDeviceInfo(dev, CL_DEVICE_NAME, sizeof(name) - 1, name, nullptr) != CL_SUCCESS)
        return false;
    for (char* p = name; *p; ++p) if (*p >= 'A' && *p <= 'Z') *p = (char)(*p - 'A' + 'a');
    return std::strstr(name, "powervr ge") || std::strstr(name, "rogue");
}
