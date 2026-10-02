// Generated from Khronos OpenCL-Headers v2024.10.24 by generate_opencl_loader.py.
// Copyright 2026 Labsii Ltd. SPDX-License-Identifier: Apache-2.0
// OpenCL API declarations: Copyright The Khronos Group Inc., Apache-2.0.
#define CL_TARGET_OPENCL_VERSION 300
#define CL_USE_DEPRECATED_OPENCL_1_0_APIS
#define CL_USE_DEPRECATED_OPENCL_1_1_APIS
#define CL_USE_DEPRECATED_OPENCL_1_2_APIS
#define CL_USE_DEPRECATED_OPENCL_2_0_APIS
#define CL_USE_DEPRECATED_OPENCL_2_1_APIS
#define CL_USE_DEPRECATED_OPENCL_2_2_APIS
#include <CL/cl.h>
#include <dlfcn.h>
#include <mutex>

// A neutral SONAME (libkokoro_opencl.so) avoids vendor-specific DT_NEEDED.
// The actual device driver is never shipped in the APK.
static void* driver() {
    static void* handle = nullptr;
    static std::once_flag once;
    std::call_once(once, [] {
        const char* paths[] = {
            "/vendor/lib64/libOpenCL.so", "/vendor/lib64/libOpenCL_adreno.so",
            "/vendor/lib64/egl/libOpenCL.so", "/vendor/lib64/egl/libOpenCL_adreno.so",
            "/odm/lib64/libOpenCL.so", "/odm/lib64/libOpenCL_adreno.so",
            "/system/vendor/lib64/libOpenCL.so", "/system_ext/lib64/libOpenCL.so",
            "/system/lib64/libOpenCL.so", "libOpenCL.so", "libOpenCL_adreno.so",
            "/vendor/lib64/libGLES_mali.so", "/vendor/lib64/egl/libGLES_mali.so"
        };
        for (const char* path : paths) {
            void* candidate = dlopen(path, RTLD_NOW | RTLD_GLOBAL);
            if (!candidate) continue;
            auto platforms = reinterpret_cast<decltype(&clGetPlatformIDs)>(dlsym(candidate, "clGetPlatformIDs"));
            // Do not invoke driver entrypoints inside call_once: some drivers
            // re-enter other exported OpenCL functions during initialization.
            if (platforms && platforms != &clGetPlatformIDs && dlsym(candidate, "clGetDeviceIDs")) {
                handle = candidate;
                break;
            }
            dlclose(candidate);
        }
    });
    return handle;
}

static void* symbol(const char* name) {
    void* handle = driver();
    return handle ? dlsym(handle, name) : nullptr;
}

extern "C" cl_int CL_API_CALL clGetPlatformIDs(cl_uint          num_entries, cl_platform_id * platforms, cl_uint *        num_platforms) {
    static auto fn = reinterpret_cast<decltype(&clGetPlatformIDs)>(symbol("clGetPlatformIDs"));
    if (fn) return fn(num_entries, platforms, num_platforms);
    if (num_platforms) *num_platforms = 0;
    return CL_INVALID_OPERATION;
}

extern "C" cl_int CL_API_CALL clGetPlatformInfo(cl_platform_id   platform, cl_platform_info param_name, size_t           param_value_size, void *           param_value, size_t *         param_value_size_ret) {
    static auto fn = reinterpret_cast<decltype(&clGetPlatformInfo)>(symbol("clGetPlatformInfo"));
    if (fn) return fn(platform, param_name, param_value_size, param_value, param_value_size_ret);
    return CL_INVALID_OPERATION;
}

extern "C" cl_int CL_API_CALL clGetDeviceIDs(cl_platform_id   platform, cl_device_type   device_type, cl_uint          num_entries, cl_device_id *   devices, cl_uint *        num_devices) {
    static auto fn = reinterpret_cast<decltype(&clGetDeviceIDs)>(symbol("clGetDeviceIDs"));
    if (fn) return fn(platform, device_type, num_entries, devices, num_devices);
    if (num_devices) *num_devices = 0;
    return CL_INVALID_OPERATION;
}

extern "C" cl_int CL_API_CALL clGetDeviceInfo(cl_device_id    device, cl_device_info  param_name, size_t          param_value_size, void *          param_value, size_t *        param_value_size_ret) {
    static auto fn = reinterpret_cast<decltype(&clGetDeviceInfo)>(symbol("clGetDeviceInfo"));
    if (fn) return fn(device, param_name, param_value_size, param_value, param_value_size_ret);
    return CL_INVALID_OPERATION;
}

extern "C" cl_int CL_API_CALL clCreateSubDevices(cl_device_id                         in_device, const cl_device_partition_property * properties, cl_uint                              num_devices, cl_device_id *                       out_devices, cl_uint *                            num_devices_ret) {
    static auto fn = reinterpret_cast<decltype(&clCreateSubDevices)>(symbol("clCreateSubDevices"));
    if (fn) return fn(in_device, properties, num_devices, out_devices, num_devices_ret);
    return CL_INVALID_OPERATION;
}

extern "C" cl_int CL_API_CALL clRetainDevice(cl_device_id device) {
    static auto fn = reinterpret_cast<decltype(&clRetainDevice)>(symbol("clRetainDevice"));
    if (fn) return fn(device);
    return CL_INVALID_OPERATION;
}

extern "C" cl_int CL_API_CALL clReleaseDevice(cl_device_id device) {
    static auto fn = reinterpret_cast<decltype(&clReleaseDevice)>(symbol("clReleaseDevice"));
    if (fn) return fn(device);
    return CL_INVALID_OPERATION;
}

extern "C" cl_int CL_API_CALL clSetDefaultDeviceCommandQueue(cl_context           context, cl_device_id         device, cl_command_queue     command_queue) {
    static auto fn = reinterpret_cast<decltype(&clSetDefaultDeviceCommandQueue)>(symbol("clSetDefaultDeviceCommandQueue"));
    if (fn) return fn(context, device, command_queue);
    return CL_INVALID_OPERATION;
}

extern "C" cl_int CL_API_CALL clGetDeviceAndHostTimer(cl_device_id    device, cl_ulong*       device_timestamp, cl_ulong*       host_timestamp) {
    static auto fn = reinterpret_cast<decltype(&clGetDeviceAndHostTimer)>(symbol("clGetDeviceAndHostTimer"));
    if (fn) return fn(device, device_timestamp, host_timestamp);
    return CL_INVALID_OPERATION;
}

extern "C" cl_int CL_API_CALL clGetHostTimer(cl_device_id device, cl_ulong *   host_timestamp) {
    static auto fn = reinterpret_cast<decltype(&clGetHostTimer)>(symbol("clGetHostTimer"));
    if (fn) return fn(device, host_timestamp);
    return CL_INVALID_OPERATION;
}

extern "C" cl_context CL_API_CALL clCreateContext(const cl_context_properties * properties, cl_uint              num_devices, const cl_device_id * devices, void (CL_CALLBACK * pfn_notify)(const char * errinfo,
                                                const void * private_info,
                                                size_t       cb,
                                                void *       user_data), void *               user_data, cl_int *             errcode_ret) {
    static auto fn = reinterpret_cast<decltype(&clCreateContext)>(symbol("clCreateContext"));
    if (fn) return fn(properties, num_devices, devices, pfn_notify, user_data, errcode_ret);
    if (errcode_ret) *errcode_ret = CL_INVALID_OPERATION;
    return nullptr;
}

extern "C" cl_context CL_API_CALL clCreateContextFromType(const cl_context_properties * properties, cl_device_type      device_type, void (CL_CALLBACK * pfn_notify)(const char * errinfo,
                                                        const void * private_info,
                                                        size_t       cb,
                                                        void *       user_data), void *              user_data, cl_int *            errcode_ret) {
    static auto fn = reinterpret_cast<decltype(&clCreateContextFromType)>(symbol("clCreateContextFromType"));
    if (fn) return fn(properties, device_type, pfn_notify, user_data, errcode_ret);
    if (errcode_ret) *errcode_ret = CL_INVALID_OPERATION;
    return nullptr;
}

extern "C" cl_int CL_API_CALL clRetainContext(cl_context context) {
    static auto fn = reinterpret_cast<decltype(&clRetainContext)>(symbol("clRetainContext"));
    if (fn) return fn(context);
    return CL_INVALID_OPERATION;
}

extern "C" cl_int CL_API_CALL clReleaseContext(cl_context context) {
    static auto fn = reinterpret_cast<decltype(&clReleaseContext)>(symbol("clReleaseContext"));
    if (fn) return fn(context);
    return CL_INVALID_OPERATION;
}

extern "C" cl_int CL_API_CALL clGetContextInfo(cl_context         context, cl_context_info    param_name, size_t             param_value_size, void *             param_value, size_t *           param_value_size_ret) {
    static auto fn = reinterpret_cast<decltype(&clGetContextInfo)>(symbol("clGetContextInfo"));
    if (fn) return fn(context, param_name, param_value_size, param_value, param_value_size_ret);
    return CL_INVALID_OPERATION;
}

extern "C" cl_int CL_API_CALL clSetContextDestructorCallback(cl_context         context, void (CL_CALLBACK* pfn_notify)(cl_context context,
                                                              void* user_data), void*              user_data) {
    static auto fn = reinterpret_cast<decltype(&clSetContextDestructorCallback)>(symbol("clSetContextDestructorCallback"));
    if (fn) return fn(context, pfn_notify, user_data);
    return CL_INVALID_OPERATION;
}

extern "C" cl_command_queue CL_API_CALL clCreateCommandQueueWithProperties(cl_context               context, cl_device_id             device, const cl_queue_properties *    properties, cl_int *                 errcode_ret) {
    static auto fn = reinterpret_cast<decltype(&clCreateCommandQueueWithProperties)>(symbol("clCreateCommandQueueWithProperties"));
    if (fn) return fn(context, device, properties, errcode_ret);
    if (errcode_ret) *errcode_ret = CL_INVALID_OPERATION;
    return nullptr;
}

extern "C" cl_int CL_API_CALL clRetainCommandQueue(cl_command_queue command_queue) {
    static auto fn = reinterpret_cast<decltype(&clRetainCommandQueue)>(symbol("clRetainCommandQueue"));
    if (fn) return fn(command_queue);
    return CL_INVALID_OPERATION;
}

extern "C" cl_int CL_API_CALL clReleaseCommandQueue(cl_command_queue command_queue) {
    static auto fn = reinterpret_cast<decltype(&clReleaseCommandQueue)>(symbol("clReleaseCommandQueue"));
    if (fn) return fn(command_queue);
    return CL_INVALID_OPERATION;
}

extern "C" cl_int CL_API_CALL clGetCommandQueueInfo(cl_command_queue      command_queue, cl_command_queue_info param_name, size_t                param_value_size, void *                param_value, size_t *              param_value_size_ret) {
    static auto fn = reinterpret_cast<decltype(&clGetCommandQueueInfo)>(symbol("clGetCommandQueueInfo"));
    if (fn) return fn(command_queue, param_name, param_value_size, param_value, param_value_size_ret);
    return CL_INVALID_OPERATION;
}

extern "C" cl_mem CL_API_CALL clCreateBuffer(cl_context   context, cl_mem_flags flags, size_t       size, void *       host_ptr, cl_int *     errcode_ret) {
    static auto fn = reinterpret_cast<decltype(&clCreateBuffer)>(symbol("clCreateBuffer"));
    if (fn) return fn(context, flags, size, host_ptr, errcode_ret);
    if (errcode_ret) *errcode_ret = CL_INVALID_OPERATION;
    return nullptr;
}

extern "C" cl_mem CL_API_CALL clCreateSubBuffer(cl_mem                   buffer, cl_mem_flags             flags, cl_buffer_create_type    buffer_create_type, const void *             buffer_create_info, cl_int *                 errcode_ret) {
    static auto fn = reinterpret_cast<decltype(&clCreateSubBuffer)>(symbol("clCreateSubBuffer"));
    if (fn) return fn(buffer, flags, buffer_create_type, buffer_create_info, errcode_ret);
    if (errcode_ret) *errcode_ret = CL_INVALID_OPERATION;
    return nullptr;
}

extern "C" cl_mem CL_API_CALL clCreateImage(cl_context              context, cl_mem_flags            flags, const cl_image_format * image_format, const cl_image_desc *   image_desc, void *                  host_ptr, cl_int *                errcode_ret) {
    static auto fn = reinterpret_cast<decltype(&clCreateImage)>(symbol("clCreateImage"));
    if (fn) return fn(context, flags, image_format, image_desc, host_ptr, errcode_ret);
    if (errcode_ret) *errcode_ret = CL_INVALID_OPERATION;
    return nullptr;
}

extern "C" cl_mem CL_API_CALL clCreatePipe(cl_context                 context, cl_mem_flags               flags, cl_uint                    pipe_packet_size, cl_uint                    pipe_max_packets, const cl_pipe_properties * properties, cl_int *                   errcode_ret) {
    static auto fn = reinterpret_cast<decltype(&clCreatePipe)>(symbol("clCreatePipe"));
    if (fn) return fn(context, flags, pipe_packet_size, pipe_max_packets, properties, errcode_ret);
    if (errcode_ret) *errcode_ret = CL_INVALID_OPERATION;
    return nullptr;
}

extern "C" cl_mem CL_API_CALL clCreateBufferWithProperties(cl_context                context, const cl_mem_properties * properties, cl_mem_flags              flags, size_t                    size, void *                    host_ptr, cl_int *                  errcode_ret) {
    static auto fn = reinterpret_cast<decltype(&clCreateBufferWithProperties)>(symbol("clCreateBufferWithProperties"));
    if (fn) return fn(context, properties, flags, size, host_ptr, errcode_ret);
    if (errcode_ret) *errcode_ret = CL_INVALID_OPERATION;
    return nullptr;
}

extern "C" cl_mem CL_API_CALL clCreateImageWithProperties(cl_context                context, const cl_mem_properties * properties, cl_mem_flags              flags, const cl_image_format *   image_format, const cl_image_desc *     image_desc, void *                    host_ptr, cl_int *                  errcode_ret) {
    static auto fn = reinterpret_cast<decltype(&clCreateImageWithProperties)>(symbol("clCreateImageWithProperties"));
    if (fn) return fn(context, properties, flags, image_format, image_desc, host_ptr, errcode_ret);
    if (errcode_ret) *errcode_ret = CL_INVALID_OPERATION;
    return nullptr;
}

extern "C" cl_int CL_API_CALL clRetainMemObject(cl_mem memobj) {
    static auto fn = reinterpret_cast<decltype(&clRetainMemObject)>(symbol("clRetainMemObject"));
    if (fn) return fn(memobj);
    return CL_INVALID_OPERATION;
}

extern "C" cl_int CL_API_CALL clReleaseMemObject(cl_mem memobj) {
    static auto fn = reinterpret_cast<decltype(&clReleaseMemObject)>(symbol("clReleaseMemObject"));
    if (fn) return fn(memobj);
    return CL_INVALID_OPERATION;
}

extern "C" cl_int CL_API_CALL clGetSupportedImageFormats(cl_context           context, cl_mem_flags         flags, cl_mem_object_type   image_type, cl_uint              num_entries, cl_image_format *    image_formats, cl_uint *            num_image_formats) {
    static auto fn = reinterpret_cast<decltype(&clGetSupportedImageFormats)>(symbol("clGetSupportedImageFormats"));
    if (fn) return fn(context, flags, image_type, num_entries, image_formats, num_image_formats);
    return CL_INVALID_OPERATION;
}

extern "C" cl_int CL_API_CALL clGetMemObjectInfo(cl_mem           memobj, cl_mem_info      param_name, size_t           param_value_size, void *           param_value, size_t *         param_value_size_ret) {
    static auto fn = reinterpret_cast<decltype(&clGetMemObjectInfo)>(symbol("clGetMemObjectInfo"));
    if (fn) return fn(memobj, param_name, param_value_size, param_value, param_value_size_ret);
    return CL_INVALID_OPERATION;
}

extern "C" cl_int CL_API_CALL clGetImageInfo(cl_mem           image, cl_image_info    param_name, size_t           param_value_size, void *           param_value, size_t *         param_value_size_ret) {
    static auto fn = reinterpret_cast<decltype(&clGetImageInfo)>(symbol("clGetImageInfo"));
    if (fn) return fn(image, param_name, param_value_size, param_value, param_value_size_ret);
    return CL_INVALID_OPERATION;
}

extern "C" cl_int CL_API_CALL clGetPipeInfo(cl_mem           pipe, cl_pipe_info     param_name, size_t           param_value_size, void *           param_value, size_t *         param_value_size_ret) {
    static auto fn = reinterpret_cast<decltype(&clGetPipeInfo)>(symbol("clGetPipeInfo"));
    if (fn) return fn(pipe, param_name, param_value_size, param_value, param_value_size_ret);
    return CL_INVALID_OPERATION;
}

extern "C" cl_int CL_API_CALL clSetMemObjectDestructorCallback(cl_mem memobj, void (CL_CALLBACK * pfn_notify)(cl_mem memobj,
                                                                 void * user_data), void * user_data) {
    static auto fn = reinterpret_cast<decltype(&clSetMemObjectDestructorCallback)>(symbol("clSetMemObjectDestructorCallback"));
    if (fn) return fn(memobj, pfn_notify, user_data);
    return CL_INVALID_OPERATION;
}

extern "C" void * CL_API_CALL clSVMAlloc(cl_context       context, cl_svm_mem_flags flags, size_t           size, cl_uint          alignment) {
    static auto fn = reinterpret_cast<decltype(&clSVMAlloc)>(symbol("clSVMAlloc"));
    if (fn) return fn(context, flags, size, alignment);
    return nullptr;
}

extern "C" void CL_API_CALL clSVMFree(cl_context        context, void *            svm_pointer) {
    static auto fn = reinterpret_cast<decltype(&clSVMFree)>(symbol("clSVMFree"));
    if (fn) fn(context, svm_pointer);
}

extern "C" cl_sampler CL_API_CALL clCreateSamplerWithProperties(cl_context                     context, const cl_sampler_properties *  sampler_properties, cl_int *                       errcode_ret) {
    static auto fn = reinterpret_cast<decltype(&clCreateSamplerWithProperties)>(symbol("clCreateSamplerWithProperties"));
    if (fn) return fn(context, sampler_properties, errcode_ret);
    if (errcode_ret) *errcode_ret = CL_INVALID_OPERATION;
    return nullptr;
}

extern "C" cl_int CL_API_CALL clRetainSampler(cl_sampler sampler) {
    static auto fn = reinterpret_cast<decltype(&clRetainSampler)>(symbol("clRetainSampler"));
    if (fn) return fn(sampler);
    return CL_INVALID_OPERATION;
}

extern "C" cl_int CL_API_CALL clReleaseSampler(cl_sampler sampler) {
    static auto fn = reinterpret_cast<decltype(&clReleaseSampler)>(symbol("clReleaseSampler"));
    if (fn) return fn(sampler);
    return CL_INVALID_OPERATION;
}

extern "C" cl_int CL_API_CALL clGetSamplerInfo(cl_sampler         sampler, cl_sampler_info    param_name, size_t             param_value_size, void *             param_value, size_t *           param_value_size_ret) {
    static auto fn = reinterpret_cast<decltype(&clGetSamplerInfo)>(symbol("clGetSamplerInfo"));
    if (fn) return fn(sampler, param_name, param_value_size, param_value, param_value_size_ret);
    return CL_INVALID_OPERATION;
}

extern "C" cl_program CL_API_CALL clCreateProgramWithSource(cl_context        context, cl_uint           count, const char **     strings, const size_t *    lengths, cl_int *          errcode_ret) {
    static auto fn = reinterpret_cast<decltype(&clCreateProgramWithSource)>(symbol("clCreateProgramWithSource"));
    if (fn) return fn(context, count, strings, lengths, errcode_ret);
    if (errcode_ret) *errcode_ret = CL_INVALID_OPERATION;
    return nullptr;
}

extern "C" cl_program CL_API_CALL clCreateProgramWithBinary(cl_context                     context, cl_uint                        num_devices, const cl_device_id *           device_list, const size_t *                 lengths, const unsigned char **         binaries, cl_int *                       binary_status, cl_int *                       errcode_ret) {
    static auto fn = reinterpret_cast<decltype(&clCreateProgramWithBinary)>(symbol("clCreateProgramWithBinary"));
    if (fn) return fn(context, num_devices, device_list, lengths, binaries, binary_status, errcode_ret);
    if (errcode_ret) *errcode_ret = CL_INVALID_OPERATION;
    return nullptr;
}

extern "C" cl_program CL_API_CALL clCreateProgramWithBuiltInKernels(cl_context            context, cl_uint               num_devices, const cl_device_id *  device_list, const char *          kernel_names, cl_int *              errcode_ret) {
    static auto fn = reinterpret_cast<decltype(&clCreateProgramWithBuiltInKernels)>(symbol("clCreateProgramWithBuiltInKernels"));
    if (fn) return fn(context, num_devices, device_list, kernel_names, errcode_ret);
    if (errcode_ret) *errcode_ret = CL_INVALID_OPERATION;
    return nullptr;
}

extern "C" cl_program CL_API_CALL clCreateProgramWithIL(cl_context    context, const void*    il, size_t         length, cl_int*        errcode_ret) {
    static auto fn = reinterpret_cast<decltype(&clCreateProgramWithIL)>(symbol("clCreateProgramWithIL"));
    if (fn) return fn(context, il, length, errcode_ret);
    if (errcode_ret) *errcode_ret = CL_INVALID_OPERATION;
    return nullptr;
}

extern "C" cl_int CL_API_CALL clRetainProgram(cl_program program) {
    static auto fn = reinterpret_cast<decltype(&clRetainProgram)>(symbol("clRetainProgram"));
    if (fn) return fn(program);
    return CL_INVALID_OPERATION;
}

extern "C" cl_int CL_API_CALL clReleaseProgram(cl_program program) {
    static auto fn = reinterpret_cast<decltype(&clReleaseProgram)>(symbol("clReleaseProgram"));
    if (fn) return fn(program);
    return CL_INVALID_OPERATION;
}

extern "C" cl_int CL_API_CALL clBuildProgram(cl_program           program, cl_uint              num_devices, const cl_device_id * device_list, const char *         options, void (CL_CALLBACK *  pfn_notify)(cl_program program,
                                                void * user_data), void *               user_data) {
    static auto fn = reinterpret_cast<decltype(&clBuildProgram)>(symbol("clBuildProgram"));
    if (fn) return fn(program, num_devices, device_list, options, pfn_notify, user_data);
    return CL_INVALID_OPERATION;
}

extern "C" cl_int CL_API_CALL clCompileProgram(cl_program           program, cl_uint              num_devices, const cl_device_id * device_list, const char *         options, cl_uint              num_input_headers, const cl_program *   input_headers, const char **        header_include_names, void (CL_CALLBACK *  pfn_notify)(cl_program program,
                                                  void * user_data), void *               user_data) {
    static auto fn = reinterpret_cast<decltype(&clCompileProgram)>(symbol("clCompileProgram"));
    if (fn) return fn(program, num_devices, device_list, options, num_input_headers, input_headers, header_include_names, pfn_notify, user_data);
    return CL_INVALID_OPERATION;
}

extern "C" cl_program CL_API_CALL clLinkProgram(cl_context           context, cl_uint              num_devices, const cl_device_id * device_list, const char *         options, cl_uint              num_input_programs, const cl_program *   input_programs, void (CL_CALLBACK *  pfn_notify)(cl_program program,
                                               void * user_data), void *               user_data, cl_int *             errcode_ret) {
    static auto fn = reinterpret_cast<decltype(&clLinkProgram)>(symbol("clLinkProgram"));
    if (fn) return fn(context, num_devices, device_list, options, num_input_programs, input_programs, pfn_notify, user_data, errcode_ret);
    if (errcode_ret) *errcode_ret = CL_INVALID_OPERATION;
    return nullptr;
}

extern "C" cl_int CL_API_CALL clSetProgramReleaseCallback(cl_program          program, void (CL_CALLBACK * pfn_notify)(cl_program program,
                                                            void * user_data), void *              user_data) {
    static auto fn = reinterpret_cast<decltype(&clSetProgramReleaseCallback)>(symbol("clSetProgramReleaseCallback"));
    if (fn) return fn(program, pfn_notify, user_data);
    return CL_INVALID_OPERATION;
}

extern "C" cl_int CL_API_CALL clSetProgramSpecializationConstant(cl_program  program, cl_uint     spec_id, size_t      spec_size, const void* spec_value) {
    static auto fn = reinterpret_cast<decltype(&clSetProgramSpecializationConstant)>(symbol("clSetProgramSpecializationConstant"));
    if (fn) return fn(program, spec_id, spec_size, spec_value);
    return CL_INVALID_OPERATION;
}

extern "C" cl_int CL_API_CALL clUnloadPlatformCompiler(cl_platform_id platform) {
    static auto fn = reinterpret_cast<decltype(&clUnloadPlatformCompiler)>(symbol("clUnloadPlatformCompiler"));
    if (fn) return fn(platform);
    return CL_INVALID_OPERATION;
}

extern "C" cl_int CL_API_CALL clGetProgramInfo(cl_program         program, cl_program_info    param_name, size_t             param_value_size, void *             param_value, size_t *           param_value_size_ret) {
    static auto fn = reinterpret_cast<decltype(&clGetProgramInfo)>(symbol("clGetProgramInfo"));
    if (fn) return fn(program, param_name, param_value_size, param_value, param_value_size_ret);
    return CL_INVALID_OPERATION;
}

extern "C" cl_int CL_API_CALL clGetProgramBuildInfo(cl_program            program, cl_device_id          device, cl_program_build_info param_name, size_t                param_value_size, void *                param_value, size_t *              param_value_size_ret) {
    static auto fn = reinterpret_cast<decltype(&clGetProgramBuildInfo)>(symbol("clGetProgramBuildInfo"));
    if (fn) return fn(program, device, param_name, param_value_size, param_value, param_value_size_ret);
    return CL_INVALID_OPERATION;
}

extern "C" cl_kernel CL_API_CALL clCreateKernel(cl_program      program, const char *    kernel_name, cl_int *        errcode_ret) {
    static auto fn = reinterpret_cast<decltype(&clCreateKernel)>(symbol("clCreateKernel"));
    if (fn) return fn(program, kernel_name, errcode_ret);
    if (errcode_ret) *errcode_ret = CL_INVALID_OPERATION;
    return nullptr;
}

extern "C" cl_int CL_API_CALL clCreateKernelsInProgram(cl_program     program, cl_uint        num_kernels, cl_kernel *    kernels, cl_uint *      num_kernels_ret) {
    static auto fn = reinterpret_cast<decltype(&clCreateKernelsInProgram)>(symbol("clCreateKernelsInProgram"));
    if (fn) return fn(program, num_kernels, kernels, num_kernels_ret);
    return CL_INVALID_OPERATION;
}

extern "C" cl_kernel CL_API_CALL clCloneKernel(cl_kernel     source_kernel, cl_int*       errcode_ret) {
    static auto fn = reinterpret_cast<decltype(&clCloneKernel)>(symbol("clCloneKernel"));
    if (fn) return fn(source_kernel, errcode_ret);
    if (errcode_ret) *errcode_ret = CL_INVALID_OPERATION;
    return nullptr;
}

extern "C" cl_int CL_API_CALL clRetainKernel(cl_kernel    kernel) {
    static auto fn = reinterpret_cast<decltype(&clRetainKernel)>(symbol("clRetainKernel"));
    if (fn) return fn(kernel);
    return CL_INVALID_OPERATION;
}

extern "C" cl_int CL_API_CALL clReleaseKernel(cl_kernel   kernel) {
    static auto fn = reinterpret_cast<decltype(&clReleaseKernel)>(symbol("clReleaseKernel"));
    if (fn) return fn(kernel);
    return CL_INVALID_OPERATION;
}

extern "C" cl_int CL_API_CALL clSetKernelArg(cl_kernel    kernel, cl_uint      arg_index, size_t       arg_size, const void * arg_value) {
    static auto fn = reinterpret_cast<decltype(&clSetKernelArg)>(symbol("clSetKernelArg"));
    if (fn) return fn(kernel, arg_index, arg_size, arg_value);
    return CL_INVALID_OPERATION;
}

extern "C" cl_int CL_API_CALL clSetKernelArgSVMPointer(cl_kernel    kernel, cl_uint      arg_index, const void * arg_value) {
    static auto fn = reinterpret_cast<decltype(&clSetKernelArgSVMPointer)>(symbol("clSetKernelArgSVMPointer"));
    if (fn) return fn(kernel, arg_index, arg_value);
    return CL_INVALID_OPERATION;
}

extern "C" cl_int CL_API_CALL clSetKernelExecInfo(cl_kernel            kernel, cl_kernel_exec_info  param_name, size_t               param_value_size, const void *         param_value) {
    static auto fn = reinterpret_cast<decltype(&clSetKernelExecInfo)>(symbol("clSetKernelExecInfo"));
    if (fn) return fn(kernel, param_name, param_value_size, param_value);
    return CL_INVALID_OPERATION;
}

extern "C" cl_int CL_API_CALL clGetKernelInfo(cl_kernel       kernel, cl_kernel_info  param_name, size_t          param_value_size, void *          param_value, size_t *        param_value_size_ret) {
    static auto fn = reinterpret_cast<decltype(&clGetKernelInfo)>(symbol("clGetKernelInfo"));
    if (fn) return fn(kernel, param_name, param_value_size, param_value, param_value_size_ret);
    return CL_INVALID_OPERATION;
}

extern "C" cl_int CL_API_CALL clGetKernelArgInfo(cl_kernel       kernel, cl_uint         arg_indx, cl_kernel_arg_info  param_name, size_t          param_value_size, void *          param_value, size_t *        param_value_size_ret) {
    static auto fn = reinterpret_cast<decltype(&clGetKernelArgInfo)>(symbol("clGetKernelArgInfo"));
    if (fn) return fn(kernel, arg_indx, param_name, param_value_size, param_value, param_value_size_ret);
    return CL_INVALID_OPERATION;
}

extern "C" cl_int CL_API_CALL clGetKernelWorkGroupInfo(cl_kernel                  kernel, cl_device_id               device, cl_kernel_work_group_info  param_name, size_t                     param_value_size, void *                     param_value, size_t *                   param_value_size_ret) {
    static auto fn = reinterpret_cast<decltype(&clGetKernelWorkGroupInfo)>(symbol("clGetKernelWorkGroupInfo"));
    if (fn) return fn(kernel, device, param_name, param_value_size, param_value, param_value_size_ret);
    return CL_INVALID_OPERATION;
}

extern "C" cl_int CL_API_CALL clGetKernelSubGroupInfo(cl_kernel                   kernel, cl_device_id                device, cl_kernel_sub_group_info    param_name, size_t                      input_value_size, const void*                 input_value, size_t                      param_value_size, void*                       param_value, size_t*                     param_value_size_ret) {
    static auto fn = reinterpret_cast<decltype(&clGetKernelSubGroupInfo)>(symbol("clGetKernelSubGroupInfo"));
    if (fn) return fn(kernel, device, param_name, input_value_size, input_value, param_value_size, param_value, param_value_size_ret);
    return CL_INVALID_OPERATION;
}

extern "C" cl_int CL_API_CALL clWaitForEvents(cl_uint             num_events, const cl_event *    event_list) {
    static auto fn = reinterpret_cast<decltype(&clWaitForEvents)>(symbol("clWaitForEvents"));
    if (fn) return fn(num_events, event_list);
    return CL_INVALID_OPERATION;
}

extern "C" cl_int CL_API_CALL clGetEventInfo(cl_event         event, cl_event_info    param_name, size_t           param_value_size, void *           param_value, size_t *         param_value_size_ret) {
    static auto fn = reinterpret_cast<decltype(&clGetEventInfo)>(symbol("clGetEventInfo"));
    if (fn) return fn(event, param_name, param_value_size, param_value, param_value_size_ret);
    return CL_INVALID_OPERATION;
}

extern "C" cl_event CL_API_CALL clCreateUserEvent(cl_context    context, cl_int *      errcode_ret) {
    static auto fn = reinterpret_cast<decltype(&clCreateUserEvent)>(symbol("clCreateUserEvent"));
    if (fn) return fn(context, errcode_ret);
    if (errcode_ret) *errcode_ret = CL_INVALID_OPERATION;
    return nullptr;
}

extern "C" cl_int CL_API_CALL clRetainEvent(cl_event event) {
    static auto fn = reinterpret_cast<decltype(&clRetainEvent)>(symbol("clRetainEvent"));
    if (fn) return fn(event);
    return CL_INVALID_OPERATION;
}

extern "C" cl_int CL_API_CALL clReleaseEvent(cl_event event) {
    static auto fn = reinterpret_cast<decltype(&clReleaseEvent)>(symbol("clReleaseEvent"));
    if (fn) return fn(event);
    return CL_INVALID_OPERATION;
}

extern "C" cl_int CL_API_CALL clSetUserEventStatus(cl_event   event, cl_int     execution_status) {
    static auto fn = reinterpret_cast<decltype(&clSetUserEventStatus)>(symbol("clSetUserEventStatus"));
    if (fn) return fn(event, execution_status);
    return CL_INVALID_OPERATION;
}

extern "C" cl_int CL_API_CALL clSetEventCallback(cl_event    event, cl_int      command_exec_callback_type, void (CL_CALLBACK * pfn_notify)(cl_event event,
                                                   cl_int   event_command_status,
                                                   void *   user_data), void *      user_data) {
    static auto fn = reinterpret_cast<decltype(&clSetEventCallback)>(symbol("clSetEventCallback"));
    if (fn) return fn(event, command_exec_callback_type, pfn_notify, user_data);
    return CL_INVALID_OPERATION;
}

extern "C" cl_int CL_API_CALL clGetEventProfilingInfo(cl_event            event, cl_profiling_info   param_name, size_t              param_value_size, void *              param_value, size_t *            param_value_size_ret) {
    static auto fn = reinterpret_cast<decltype(&clGetEventProfilingInfo)>(symbol("clGetEventProfilingInfo"));
    if (fn) return fn(event, param_name, param_value_size, param_value, param_value_size_ret);
    return CL_INVALID_OPERATION;
}

extern "C" cl_int CL_API_CALL clFlush(cl_command_queue command_queue) {
    static auto fn = reinterpret_cast<decltype(&clFlush)>(symbol("clFlush"));
    if (fn) return fn(command_queue);
    return CL_INVALID_OPERATION;
}

extern "C" cl_int CL_API_CALL clFinish(cl_command_queue command_queue) {
    static auto fn = reinterpret_cast<decltype(&clFinish)>(symbol("clFinish"));
    if (fn) return fn(command_queue);
    return CL_INVALID_OPERATION;
}

extern "C" cl_int CL_API_CALL clEnqueueReadBuffer(cl_command_queue    command_queue, cl_mem              buffer, cl_bool             blocking_read, size_t              offset, size_t              size, void *              ptr, cl_uint             num_events_in_wait_list, const cl_event *    event_wait_list, cl_event *          event) {
    static auto fn = reinterpret_cast<decltype(&clEnqueueReadBuffer)>(symbol("clEnqueueReadBuffer"));
    if (fn) return fn(command_queue, buffer, blocking_read, offset, size, ptr, num_events_in_wait_list, event_wait_list, event);
    return CL_INVALID_OPERATION;
}

extern "C" cl_int CL_API_CALL clEnqueueReadBufferRect(cl_command_queue    command_queue, cl_mem              buffer, cl_bool             blocking_read, const size_t *      buffer_origin, const size_t *      host_origin, const size_t *      region, size_t              buffer_row_pitch, size_t              buffer_slice_pitch, size_t              host_row_pitch, size_t              host_slice_pitch, void *              ptr, cl_uint             num_events_in_wait_list, const cl_event *    event_wait_list, cl_event *          event) {
    static auto fn = reinterpret_cast<decltype(&clEnqueueReadBufferRect)>(symbol("clEnqueueReadBufferRect"));
    if (fn) return fn(command_queue, buffer, blocking_read, buffer_origin, host_origin, region, buffer_row_pitch, buffer_slice_pitch, host_row_pitch, host_slice_pitch, ptr, num_events_in_wait_list, event_wait_list, event);
    return CL_INVALID_OPERATION;
}

extern "C" cl_int CL_API_CALL clEnqueueWriteBuffer(cl_command_queue   command_queue, cl_mem             buffer, cl_bool            blocking_write, size_t             offset, size_t             size, const void *       ptr, cl_uint            num_events_in_wait_list, const cl_event *   event_wait_list, cl_event *         event) {
    static auto fn = reinterpret_cast<decltype(&clEnqueueWriteBuffer)>(symbol("clEnqueueWriteBuffer"));
    if (fn) return fn(command_queue, buffer, blocking_write, offset, size, ptr, num_events_in_wait_list, event_wait_list, event);
    return CL_INVALID_OPERATION;
}

extern "C" cl_int CL_API_CALL clEnqueueWriteBufferRect(cl_command_queue    command_queue, cl_mem              buffer, cl_bool             blocking_write, const size_t *      buffer_origin, const size_t *      host_origin, const size_t *      region, size_t              buffer_row_pitch, size_t              buffer_slice_pitch, size_t              host_row_pitch, size_t              host_slice_pitch, const void *        ptr, cl_uint             num_events_in_wait_list, const cl_event *    event_wait_list, cl_event *          event) {
    static auto fn = reinterpret_cast<decltype(&clEnqueueWriteBufferRect)>(symbol("clEnqueueWriteBufferRect"));
    if (fn) return fn(command_queue, buffer, blocking_write, buffer_origin, host_origin, region, buffer_row_pitch, buffer_slice_pitch, host_row_pitch, host_slice_pitch, ptr, num_events_in_wait_list, event_wait_list, event);
    return CL_INVALID_OPERATION;
}

extern "C" cl_int CL_API_CALL clEnqueueFillBuffer(cl_command_queue   command_queue, cl_mem             buffer, const void *       pattern, size_t             pattern_size, size_t             offset, size_t             size, cl_uint            num_events_in_wait_list, const cl_event *   event_wait_list, cl_event *         event) {
    static auto fn = reinterpret_cast<decltype(&clEnqueueFillBuffer)>(symbol("clEnqueueFillBuffer"));
    if (fn) return fn(command_queue, buffer, pattern, pattern_size, offset, size, num_events_in_wait_list, event_wait_list, event);
    return CL_INVALID_OPERATION;
}

extern "C" cl_int CL_API_CALL clEnqueueCopyBuffer(cl_command_queue    command_queue, cl_mem              src_buffer, cl_mem              dst_buffer, size_t              src_offset, size_t              dst_offset, size_t              size, cl_uint             num_events_in_wait_list, const cl_event *    event_wait_list, cl_event *          event) {
    static auto fn = reinterpret_cast<decltype(&clEnqueueCopyBuffer)>(symbol("clEnqueueCopyBuffer"));
    if (fn) return fn(command_queue, src_buffer, dst_buffer, src_offset, dst_offset, size, num_events_in_wait_list, event_wait_list, event);
    return CL_INVALID_OPERATION;
}

extern "C" cl_int CL_API_CALL clEnqueueCopyBufferRect(cl_command_queue    command_queue, cl_mem              src_buffer, cl_mem              dst_buffer, const size_t *      src_origin, const size_t *      dst_origin, const size_t *      region, size_t              src_row_pitch, size_t              src_slice_pitch, size_t              dst_row_pitch, size_t              dst_slice_pitch, cl_uint             num_events_in_wait_list, const cl_event *    event_wait_list, cl_event *          event) {
    static auto fn = reinterpret_cast<decltype(&clEnqueueCopyBufferRect)>(symbol("clEnqueueCopyBufferRect"));
    if (fn) return fn(command_queue, src_buffer, dst_buffer, src_origin, dst_origin, region, src_row_pitch, src_slice_pitch, dst_row_pitch, dst_slice_pitch, num_events_in_wait_list, event_wait_list, event);
    return CL_INVALID_OPERATION;
}

extern "C" cl_int CL_API_CALL clEnqueueReadImage(cl_command_queue     command_queue, cl_mem               image, cl_bool              blocking_read, const size_t *       origin, const size_t *       region, size_t               row_pitch, size_t               slice_pitch, void *               ptr, cl_uint              num_events_in_wait_list, const cl_event *     event_wait_list, cl_event *           event) {
    static auto fn = reinterpret_cast<decltype(&clEnqueueReadImage)>(symbol("clEnqueueReadImage"));
    if (fn) return fn(command_queue, image, blocking_read, origin, region, row_pitch, slice_pitch, ptr, num_events_in_wait_list, event_wait_list, event);
    return CL_INVALID_OPERATION;
}

extern "C" cl_int CL_API_CALL clEnqueueWriteImage(cl_command_queue    command_queue, cl_mem              image, cl_bool             blocking_write, const size_t *      origin, const size_t *      region, size_t              input_row_pitch, size_t              input_slice_pitch, const void *        ptr, cl_uint             num_events_in_wait_list, const cl_event *    event_wait_list, cl_event *          event) {
    static auto fn = reinterpret_cast<decltype(&clEnqueueWriteImage)>(symbol("clEnqueueWriteImage"));
    if (fn) return fn(command_queue, image, blocking_write, origin, region, input_row_pitch, input_slice_pitch, ptr, num_events_in_wait_list, event_wait_list, event);
    return CL_INVALID_OPERATION;
}

extern "C" cl_int CL_API_CALL clEnqueueFillImage(cl_command_queue   command_queue, cl_mem             image, const void *       fill_color, const size_t *     origin, const size_t *     region, cl_uint            num_events_in_wait_list, const cl_event *   event_wait_list, cl_event *         event) {
    static auto fn = reinterpret_cast<decltype(&clEnqueueFillImage)>(symbol("clEnqueueFillImage"));
    if (fn) return fn(command_queue, image, fill_color, origin, region, num_events_in_wait_list, event_wait_list, event);
    return CL_INVALID_OPERATION;
}

extern "C" cl_int CL_API_CALL clEnqueueCopyImage(cl_command_queue     command_queue, cl_mem               src_image, cl_mem               dst_image, const size_t *       src_origin, const size_t *       dst_origin, const size_t *       region, cl_uint              num_events_in_wait_list, const cl_event *     event_wait_list, cl_event *           event) {
    static auto fn = reinterpret_cast<decltype(&clEnqueueCopyImage)>(symbol("clEnqueueCopyImage"));
    if (fn) return fn(command_queue, src_image, dst_image, src_origin, dst_origin, region, num_events_in_wait_list, event_wait_list, event);
    return CL_INVALID_OPERATION;
}

extern "C" cl_int CL_API_CALL clEnqueueCopyImageToBuffer(cl_command_queue command_queue, cl_mem           src_image, cl_mem           dst_buffer, const size_t *   src_origin, const size_t *   region, size_t           dst_offset, cl_uint          num_events_in_wait_list, const cl_event * event_wait_list, cl_event *       event) {
    static auto fn = reinterpret_cast<decltype(&clEnqueueCopyImageToBuffer)>(symbol("clEnqueueCopyImageToBuffer"));
    if (fn) return fn(command_queue, src_image, dst_buffer, src_origin, region, dst_offset, num_events_in_wait_list, event_wait_list, event);
    return CL_INVALID_OPERATION;
}

extern "C" cl_int CL_API_CALL clEnqueueCopyBufferToImage(cl_command_queue command_queue, cl_mem           src_buffer, cl_mem           dst_image, size_t           src_offset, const size_t *   dst_origin, const size_t *   region, cl_uint          num_events_in_wait_list, const cl_event * event_wait_list, cl_event *       event) {
    static auto fn = reinterpret_cast<decltype(&clEnqueueCopyBufferToImage)>(symbol("clEnqueueCopyBufferToImage"));
    if (fn) return fn(command_queue, src_buffer, dst_image, src_offset, dst_origin, region, num_events_in_wait_list, event_wait_list, event);
    return CL_INVALID_OPERATION;
}

extern "C" void * CL_API_CALL clEnqueueMapBuffer(cl_command_queue command_queue, cl_mem           buffer, cl_bool          blocking_map, cl_map_flags     map_flags, size_t           offset, size_t           size, cl_uint          num_events_in_wait_list, const cl_event * event_wait_list, cl_event *       event, cl_int *         errcode_ret) {
    static auto fn = reinterpret_cast<decltype(&clEnqueueMapBuffer)>(symbol("clEnqueueMapBuffer"));
    if (fn) return fn(command_queue, buffer, blocking_map, map_flags, offset, size, num_events_in_wait_list, event_wait_list, event, errcode_ret);
    if (errcode_ret) *errcode_ret = CL_INVALID_OPERATION;
    return nullptr;
}

extern "C" void * CL_API_CALL clEnqueueMapImage(cl_command_queue  command_queue, cl_mem            image, cl_bool           blocking_map, cl_map_flags      map_flags, const size_t *    origin, const size_t *    region, size_t *          image_row_pitch, size_t *          image_slice_pitch, cl_uint           num_events_in_wait_list, const cl_event *  event_wait_list, cl_event *        event, cl_int *          errcode_ret) {
    static auto fn = reinterpret_cast<decltype(&clEnqueueMapImage)>(symbol("clEnqueueMapImage"));
    if (fn) return fn(command_queue, image, blocking_map, map_flags, origin, region, image_row_pitch, image_slice_pitch, num_events_in_wait_list, event_wait_list, event, errcode_ret);
    if (errcode_ret) *errcode_ret = CL_INVALID_OPERATION;
    return nullptr;
}

extern "C" cl_int CL_API_CALL clEnqueueUnmapMemObject(cl_command_queue command_queue, cl_mem           memobj, void *           mapped_ptr, cl_uint          num_events_in_wait_list, const cl_event * event_wait_list, cl_event *       event) {
    static auto fn = reinterpret_cast<decltype(&clEnqueueUnmapMemObject)>(symbol("clEnqueueUnmapMemObject"));
    if (fn) return fn(command_queue, memobj, mapped_ptr, num_events_in_wait_list, event_wait_list, event);
    return CL_INVALID_OPERATION;
}

extern "C" cl_int CL_API_CALL clEnqueueMigrateMemObjects(cl_command_queue       command_queue, cl_uint                num_mem_objects, const cl_mem *         mem_objects, cl_mem_migration_flags flags, cl_uint                num_events_in_wait_list, const cl_event *       event_wait_list, cl_event *             event) {
    static auto fn = reinterpret_cast<decltype(&clEnqueueMigrateMemObjects)>(symbol("clEnqueueMigrateMemObjects"));
    if (fn) return fn(command_queue, num_mem_objects, mem_objects, flags, num_events_in_wait_list, event_wait_list, event);
    return CL_INVALID_OPERATION;
}

extern "C" cl_int CL_API_CALL clEnqueueNDRangeKernel(cl_command_queue command_queue, cl_kernel        kernel, cl_uint          work_dim, const size_t *   global_work_offset, const size_t *   global_work_size, const size_t *   local_work_size, cl_uint          num_events_in_wait_list, const cl_event * event_wait_list, cl_event *       event) {
    static auto fn = reinterpret_cast<decltype(&clEnqueueNDRangeKernel)>(symbol("clEnqueueNDRangeKernel"));
    if (fn) return fn(command_queue, kernel, work_dim, global_work_offset, global_work_size, local_work_size, num_events_in_wait_list, event_wait_list, event);
    return CL_INVALID_OPERATION;
}

extern "C" cl_int CL_API_CALL clEnqueueNativeKernel(cl_command_queue  command_queue, void (CL_CALLBACK * user_func)(void *), void *            args, size_t            cb_args, cl_uint           num_mem_objects, const cl_mem *    mem_list, const void **     args_mem_loc, cl_uint           num_events_in_wait_list, const cl_event *  event_wait_list, cl_event *        event) {
    static auto fn = reinterpret_cast<decltype(&clEnqueueNativeKernel)>(symbol("clEnqueueNativeKernel"));
    if (fn) return fn(command_queue, user_func, args, cb_args, num_mem_objects, mem_list, args_mem_loc, num_events_in_wait_list, event_wait_list, event);
    return CL_INVALID_OPERATION;
}

extern "C" cl_int CL_API_CALL clEnqueueMarkerWithWaitList(cl_command_queue  command_queue, cl_uint           num_events_in_wait_list, const cl_event *  event_wait_list, cl_event *        event) {
    static auto fn = reinterpret_cast<decltype(&clEnqueueMarkerWithWaitList)>(symbol("clEnqueueMarkerWithWaitList"));
    if (fn) return fn(command_queue, num_events_in_wait_list, event_wait_list, event);
    return CL_INVALID_OPERATION;
}

extern "C" cl_int CL_API_CALL clEnqueueBarrierWithWaitList(cl_command_queue  command_queue, cl_uint           num_events_in_wait_list, const cl_event *  event_wait_list, cl_event *        event) {
    static auto fn = reinterpret_cast<decltype(&clEnqueueBarrierWithWaitList)>(symbol("clEnqueueBarrierWithWaitList"));
    if (fn) return fn(command_queue, num_events_in_wait_list, event_wait_list, event);
    return CL_INVALID_OPERATION;
}

extern "C" cl_int CL_API_CALL clEnqueueSVMFree(cl_command_queue  command_queue, cl_uint           num_svm_pointers, void *            svm_pointers[], void (CL_CALLBACK * pfn_free_func)(cl_command_queue queue,
                                                    cl_uint          num_svm_pointers,
                                                    void *           svm_pointers[],
                                                    void *           user_data), void *            user_data, cl_uint           num_events_in_wait_list, const cl_event *  event_wait_list, cl_event *        event) {
    static auto fn = reinterpret_cast<decltype(&clEnqueueSVMFree)>(symbol("clEnqueueSVMFree"));
    if (fn) return fn(command_queue, num_svm_pointers, svm_pointers, pfn_free_func, user_data, num_events_in_wait_list, event_wait_list, event);
    return CL_INVALID_OPERATION;
}

extern "C" cl_int CL_API_CALL clEnqueueSVMMemcpy(cl_command_queue  command_queue, cl_bool           blocking_copy, void *            dst_ptr, const void *      src_ptr, size_t            size, cl_uint           num_events_in_wait_list, const cl_event *  event_wait_list, cl_event *        event) {
    static auto fn = reinterpret_cast<decltype(&clEnqueueSVMMemcpy)>(symbol("clEnqueueSVMMemcpy"));
    if (fn) return fn(command_queue, blocking_copy, dst_ptr, src_ptr, size, num_events_in_wait_list, event_wait_list, event);
    return CL_INVALID_OPERATION;
}

extern "C" cl_int CL_API_CALL clEnqueueSVMMemFill(cl_command_queue  command_queue, void *            svm_ptr, const void *      pattern, size_t            pattern_size, size_t            size, cl_uint           num_events_in_wait_list, const cl_event *  event_wait_list, cl_event *        event) {
    static auto fn = reinterpret_cast<decltype(&clEnqueueSVMMemFill)>(symbol("clEnqueueSVMMemFill"));
    if (fn) return fn(command_queue, svm_ptr, pattern, pattern_size, size, num_events_in_wait_list, event_wait_list, event);
    return CL_INVALID_OPERATION;
}

extern "C" cl_int CL_API_CALL clEnqueueSVMMap(cl_command_queue  command_queue, cl_bool           blocking_map, cl_map_flags      flags, void *            svm_ptr, size_t            size, cl_uint           num_events_in_wait_list, const cl_event *  event_wait_list, cl_event *        event) {
    static auto fn = reinterpret_cast<decltype(&clEnqueueSVMMap)>(symbol("clEnqueueSVMMap"));
    if (fn) return fn(command_queue, blocking_map, flags, svm_ptr, size, num_events_in_wait_list, event_wait_list, event);
    return CL_INVALID_OPERATION;
}

extern "C" cl_int CL_API_CALL clEnqueueSVMUnmap(cl_command_queue  command_queue, void *            svm_ptr, cl_uint           num_events_in_wait_list, const cl_event *  event_wait_list, cl_event *        event) {
    static auto fn = reinterpret_cast<decltype(&clEnqueueSVMUnmap)>(symbol("clEnqueueSVMUnmap"));
    if (fn) return fn(command_queue, svm_ptr, num_events_in_wait_list, event_wait_list, event);
    return CL_INVALID_OPERATION;
}

extern "C" cl_int CL_API_CALL clEnqueueSVMMigrateMem(cl_command_queue         command_queue, cl_uint                  num_svm_pointers, const void **            svm_pointers, const size_t *           sizes, cl_mem_migration_flags   flags, cl_uint                  num_events_in_wait_list, const cl_event *         event_wait_list, cl_event *               event) {
    static auto fn = reinterpret_cast<decltype(&clEnqueueSVMMigrateMem)>(symbol("clEnqueueSVMMigrateMem"));
    if (fn) return fn(command_queue, num_svm_pointers, svm_pointers, sizes, flags, num_events_in_wait_list, event_wait_list, event);
    return CL_INVALID_OPERATION;
}

extern "C" void * CL_API_CALL clGetExtensionFunctionAddressForPlatform(cl_platform_id platform, const char *   func_name) {
    static auto fn = reinterpret_cast<decltype(&clGetExtensionFunctionAddressForPlatform)>(symbol("clGetExtensionFunctionAddressForPlatform"));
    if (fn) return fn(platform, func_name);
    return nullptr;
}

extern "C" cl_int CL_API_CALL clSetCommandQueueProperty(cl_command_queue              command_queue, cl_command_queue_properties   properties, cl_bool                       enable, cl_command_queue_properties * old_properties) {
    static auto fn = reinterpret_cast<decltype(&clSetCommandQueueProperty)>(symbol("clSetCommandQueueProperty"));
    if (fn) return fn(command_queue, properties, enable, old_properties);
    return CL_INVALID_OPERATION;
}

extern "C" cl_mem CL_API_CALL clCreateImage2D(cl_context              context, cl_mem_flags            flags, const cl_image_format * image_format, size_t                  image_width, size_t                  image_height, size_t                  image_row_pitch, void *                  host_ptr, cl_int *                errcode_ret) {
    static auto fn = reinterpret_cast<decltype(&clCreateImage2D)>(symbol("clCreateImage2D"));
    if (fn) return fn(context, flags, image_format, image_width, image_height, image_row_pitch, host_ptr, errcode_ret);
    if (errcode_ret) *errcode_ret = CL_INVALID_OPERATION;
    return nullptr;
}

extern "C" cl_mem CL_API_CALL clCreateImage3D(cl_context              context, cl_mem_flags            flags, const cl_image_format * image_format, size_t                  image_width, size_t                  image_height, size_t                  image_depth, size_t                  image_row_pitch, size_t                  image_slice_pitch, void *                  host_ptr, cl_int *                errcode_ret) {
    static auto fn = reinterpret_cast<decltype(&clCreateImage3D)>(symbol("clCreateImage3D"));
    if (fn) return fn(context, flags, image_format, image_width, image_height, image_depth, image_row_pitch, image_slice_pitch, host_ptr, errcode_ret);
    if (errcode_ret) *errcode_ret = CL_INVALID_OPERATION;
    return nullptr;
}

extern "C" cl_int CL_API_CALL clEnqueueMarker(cl_command_queue    command_queue, cl_event *          event) {
    static auto fn = reinterpret_cast<decltype(&clEnqueueMarker)>(symbol("clEnqueueMarker"));
    if (fn) return fn(command_queue, event);
    return CL_INVALID_OPERATION;
}

extern "C" cl_int CL_API_CALL clEnqueueWaitForEvents(cl_command_queue  command_queue, cl_uint          num_events, const cl_event * event_list) {
    static auto fn = reinterpret_cast<decltype(&clEnqueueWaitForEvents)>(symbol("clEnqueueWaitForEvents"));
    if (fn) return fn(command_queue, num_events, event_list);
    return CL_INVALID_OPERATION;
}

extern "C" cl_int CL_API_CALL clEnqueueBarrier(cl_command_queue command_queue) {
    static auto fn = reinterpret_cast<decltype(&clEnqueueBarrier)>(symbol("clEnqueueBarrier"));
    if (fn) return fn(command_queue);
    return CL_INVALID_OPERATION;
}

extern "C" cl_int CL_API_CALL clUnloadCompiler() {
    static auto fn = reinterpret_cast<decltype(&clUnloadCompiler)>(symbol("clUnloadCompiler"));
    if (fn) return fn();
    return CL_INVALID_OPERATION;
}

extern "C" void * CL_API_CALL clGetExtensionFunctionAddress(const char * func_name) {
    static auto fn = reinterpret_cast<decltype(&clGetExtensionFunctionAddress)>(symbol("clGetExtensionFunctionAddress"));
    if (fn) return fn(func_name);
    return nullptr;
}

extern "C" cl_command_queue CL_API_CALL clCreateCommandQueue(cl_context                     context, cl_device_id                   device, cl_command_queue_properties    properties, cl_int *                       errcode_ret) {
    static auto fn = reinterpret_cast<decltype(&clCreateCommandQueue)>(symbol("clCreateCommandQueue"));
    if (fn) return fn(context, device, properties, errcode_ret);
    if (errcode_ret) *errcode_ret = CL_INVALID_OPERATION;
    return nullptr;
}

extern "C" cl_sampler CL_API_CALL clCreateSampler(cl_context          context, cl_bool             normalized_coords, cl_addressing_mode  addressing_mode, cl_filter_mode      filter_mode, cl_int *            errcode_ret) {
    static auto fn = reinterpret_cast<decltype(&clCreateSampler)>(symbol("clCreateSampler"));
    if (fn) return fn(context, normalized_coords, addressing_mode, filter_mode, errcode_ret);
    if (errcode_ret) *errcode_ret = CL_INVALID_OPERATION;
    return nullptr;
}

extern "C" cl_int CL_API_CALL clEnqueueTask(cl_command_queue  command_queue, cl_kernel         kernel, cl_uint           num_events_in_wait_list, const cl_event *  event_wait_list, cl_event *        event) {
    static auto fn = reinterpret_cast<decltype(&clEnqueueTask)>(symbol("clEnqueueTask"));
    if (fn) return fn(command_queue, kernel, num_events_in_wait_list, event_wait_list, event);
    return CL_INVALID_OPERATION;
}
