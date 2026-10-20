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
#include "opencl_loader_runtime.h"

extern "C" cl_int CL_API_CALL clGetPlatformIDs(cl_uint          num_entries, cl_platform_id * platforms, cl_uint *        num_platforms) {
    static std::atomic<decltype(&clGetPlatformIDs)> cached{nullptr};
    auto fn = kokoro_cl_loader::resolve("clGetPlatformIDs", cached, &clGetPlatformIDs);
    if (fn) return fn(num_entries, platforms, num_platforms);
    if (num_platforms) *num_platforms = 0;
    return CL_INVALID_OPERATION;
}

extern "C" cl_int CL_API_CALL clGetPlatformInfo(cl_platform_id   platform, cl_platform_info param_name, size_t           param_value_size, void *           param_value, size_t *         param_value_size_ret) {
    static std::atomic<decltype(&clGetPlatformInfo)> cached{nullptr};
    auto fn = kokoro_cl_loader::resolve("clGetPlatformInfo", cached, &clGetPlatformInfo);
    if (fn) return fn(platform, param_name, param_value_size, param_value, param_value_size_ret);
    return CL_INVALID_OPERATION;
}

extern "C" cl_int CL_API_CALL clGetDeviceIDs(cl_platform_id   platform, cl_device_type   device_type, cl_uint          num_entries, cl_device_id *   devices, cl_uint *        num_devices) {
    static std::atomic<decltype(&clGetDeviceIDs)> cached{nullptr};
    auto fn = kokoro_cl_loader::resolve("clGetDeviceIDs", cached, &clGetDeviceIDs);
    if (fn) return fn(platform, device_type, num_entries, devices, num_devices);
    if (num_devices) *num_devices = 0;
    return CL_INVALID_OPERATION;
}

extern "C" cl_int CL_API_CALL clGetDeviceInfo(cl_device_id    device, cl_device_info  param_name, size_t          param_value_size, void *          param_value, size_t *        param_value_size_ret) {
    static std::atomic<decltype(&clGetDeviceInfo)> cached{nullptr};
    auto fn = kokoro_cl_loader::resolve("clGetDeviceInfo", cached, &clGetDeviceInfo);
    if (fn) return fn(device, param_name, param_value_size, param_value, param_value_size_ret);
    return CL_INVALID_OPERATION;
}

extern "C" cl_int CL_API_CALL clCreateSubDevices(cl_device_id                         in_device, const cl_device_partition_property * properties, cl_uint                              num_devices, cl_device_id *                       out_devices, cl_uint *                            num_devices_ret) {
    static std::atomic<decltype(&clCreateSubDevices)> cached{nullptr};
    auto fn = kokoro_cl_loader::resolve("clCreateSubDevices", cached, &clCreateSubDevices);
    if (fn) return fn(in_device, properties, num_devices, out_devices, num_devices_ret);
    return CL_INVALID_OPERATION;
}

extern "C" cl_int CL_API_CALL clRetainDevice(cl_device_id device) {
    static std::atomic<decltype(&clRetainDevice)> cached{nullptr};
    auto fn = kokoro_cl_loader::resolve("clRetainDevice", cached, &clRetainDevice);
    if (fn) return fn(device);
    return CL_INVALID_OPERATION;
}

extern "C" cl_int CL_API_CALL clReleaseDevice(cl_device_id device) {
    static std::atomic<decltype(&clReleaseDevice)> cached{nullptr};
    auto fn = kokoro_cl_loader::resolve("clReleaseDevice", cached, &clReleaseDevice);
    if (fn) return fn(device);
    return CL_INVALID_OPERATION;
}

extern "C" cl_int CL_API_CALL clSetDefaultDeviceCommandQueue(cl_context           context, cl_device_id         device, cl_command_queue     command_queue) {
    static std::atomic<decltype(&clSetDefaultDeviceCommandQueue)> cached{nullptr};
    auto fn = kokoro_cl_loader::resolve("clSetDefaultDeviceCommandQueue", cached, &clSetDefaultDeviceCommandQueue);
    if (fn) return fn(context, device, command_queue);
    return CL_INVALID_OPERATION;
}

extern "C" cl_int CL_API_CALL clGetDeviceAndHostTimer(cl_device_id    device, cl_ulong*       device_timestamp, cl_ulong*       host_timestamp) {
    static std::atomic<decltype(&clGetDeviceAndHostTimer)> cached{nullptr};
    auto fn = kokoro_cl_loader::resolve("clGetDeviceAndHostTimer", cached, &clGetDeviceAndHostTimer);
    if (fn) return fn(device, device_timestamp, host_timestamp);
    return CL_INVALID_OPERATION;
}

extern "C" cl_int CL_API_CALL clGetHostTimer(cl_device_id device, cl_ulong *   host_timestamp) {
    static std::atomic<decltype(&clGetHostTimer)> cached{nullptr};
    auto fn = kokoro_cl_loader::resolve("clGetHostTimer", cached, &clGetHostTimer);
    if (fn) return fn(device, host_timestamp);
    return CL_INVALID_OPERATION;
}

extern "C" cl_context CL_API_CALL clCreateContext(const cl_context_properties * properties, cl_uint              num_devices, const cl_device_id * devices, void (CL_CALLBACK * pfn_notify)(const char * errinfo,
                                                const void * private_info,
                                                size_t       cb,
                                                void *       user_data), void *               user_data, cl_int *             errcode_ret) {
    static std::atomic<decltype(&clCreateContext)> cached{nullptr};
    auto fn = kokoro_cl_loader::resolve("clCreateContext", cached, &clCreateContext);
    if (fn) return fn(properties, num_devices, devices, pfn_notify, user_data, errcode_ret);
    if (errcode_ret) *errcode_ret = CL_INVALID_OPERATION;
    return nullptr;
}

extern "C" cl_context CL_API_CALL clCreateContextFromType(const cl_context_properties * properties, cl_device_type      device_type, void (CL_CALLBACK * pfn_notify)(const char * errinfo,
                                                        const void * private_info,
                                                        size_t       cb,
                                                        void *       user_data), void *              user_data, cl_int *            errcode_ret) {
    static std::atomic<decltype(&clCreateContextFromType)> cached{nullptr};
    auto fn = kokoro_cl_loader::resolve("clCreateContextFromType", cached, &clCreateContextFromType);
    if (fn) return fn(properties, device_type, pfn_notify, user_data, errcode_ret);
    if (errcode_ret) *errcode_ret = CL_INVALID_OPERATION;
    return nullptr;
}

extern "C" cl_int CL_API_CALL clRetainContext(cl_context context) {
    static std::atomic<decltype(&clRetainContext)> cached{nullptr};
    auto fn = kokoro_cl_loader::resolve("clRetainContext", cached, &clRetainContext);
    if (fn) return fn(context);
    return CL_INVALID_OPERATION;
}

extern "C" cl_int CL_API_CALL clReleaseContext(cl_context context) {
    static std::atomic<decltype(&clReleaseContext)> cached{nullptr};
    auto fn = kokoro_cl_loader::resolve("clReleaseContext", cached, &clReleaseContext);
    if (fn) return fn(context);
    return CL_INVALID_OPERATION;
}

extern "C" cl_int CL_API_CALL clGetContextInfo(cl_context         context, cl_context_info    param_name, size_t             param_value_size, void *             param_value, size_t *           param_value_size_ret) {
    static std::atomic<decltype(&clGetContextInfo)> cached{nullptr};
    auto fn = kokoro_cl_loader::resolve("clGetContextInfo", cached, &clGetContextInfo);
    if (fn) return fn(context, param_name, param_value_size, param_value, param_value_size_ret);
    return CL_INVALID_OPERATION;
}

extern "C" cl_int CL_API_CALL clSetContextDestructorCallback(cl_context         context, void (CL_CALLBACK* pfn_notify)(cl_context context,
                                                              void* user_data), void*              user_data) {
    static std::atomic<decltype(&clSetContextDestructorCallback)> cached{nullptr};
    auto fn = kokoro_cl_loader::resolve("clSetContextDestructorCallback", cached, &clSetContextDestructorCallback);
    if (fn) return fn(context, pfn_notify, user_data);
    return CL_INVALID_OPERATION;
}

extern "C" cl_command_queue CL_API_CALL clCreateCommandQueueWithProperties(cl_context               context, cl_device_id             device, const cl_queue_properties *    properties, cl_int *                 errcode_ret) {
    static std::atomic<decltype(&clCreateCommandQueueWithProperties)> cached{nullptr};
    auto fn = kokoro_cl_loader::resolve("clCreateCommandQueueWithProperties", cached, &clCreateCommandQueueWithProperties);
    if (fn) return fn(context, device, properties, errcode_ret);
    if (errcode_ret) *errcode_ret = CL_INVALID_OPERATION;
    return nullptr;
}

extern "C" cl_int CL_API_CALL clRetainCommandQueue(cl_command_queue command_queue) {
    static std::atomic<decltype(&clRetainCommandQueue)> cached{nullptr};
    auto fn = kokoro_cl_loader::resolve("clRetainCommandQueue", cached, &clRetainCommandQueue);
    if (fn) return fn(command_queue);
    return CL_INVALID_OPERATION;
}

extern "C" cl_int CL_API_CALL clReleaseCommandQueue(cl_command_queue command_queue) {
    static std::atomic<decltype(&clReleaseCommandQueue)> cached{nullptr};
    auto fn = kokoro_cl_loader::resolve("clReleaseCommandQueue", cached, &clReleaseCommandQueue);
    if (fn) return fn(command_queue);
    return CL_INVALID_OPERATION;
}

extern "C" cl_int CL_API_CALL clGetCommandQueueInfo(cl_command_queue      command_queue, cl_command_queue_info param_name, size_t                param_value_size, void *                param_value, size_t *              param_value_size_ret) {
    static std::atomic<decltype(&clGetCommandQueueInfo)> cached{nullptr};
    auto fn = kokoro_cl_loader::resolve("clGetCommandQueueInfo", cached, &clGetCommandQueueInfo);
    if (fn) return fn(command_queue, param_name, param_value_size, param_value, param_value_size_ret);
    return CL_INVALID_OPERATION;
}

extern "C" cl_mem CL_API_CALL clCreateBuffer(cl_context   context, cl_mem_flags flags, size_t       size, void *       host_ptr, cl_int *     errcode_ret) {
    static std::atomic<decltype(&clCreateBuffer)> cached{nullptr};
    auto fn = kokoro_cl_loader::resolve("clCreateBuffer", cached, &clCreateBuffer);
    if (fn) return fn(context, flags, size, host_ptr, errcode_ret);
    if (errcode_ret) *errcode_ret = CL_INVALID_OPERATION;
    return nullptr;
}

extern "C" cl_mem CL_API_CALL clCreateSubBuffer(cl_mem                   buffer, cl_mem_flags             flags, cl_buffer_create_type    buffer_create_type, const void *             buffer_create_info, cl_int *                 errcode_ret) {
    static std::atomic<decltype(&clCreateSubBuffer)> cached{nullptr};
    auto fn = kokoro_cl_loader::resolve("clCreateSubBuffer", cached, &clCreateSubBuffer);
    if (fn) return fn(buffer, flags, buffer_create_type, buffer_create_info, errcode_ret);
    if (errcode_ret) *errcode_ret = CL_INVALID_OPERATION;
    return nullptr;
}

extern "C" cl_mem CL_API_CALL clCreateImage(cl_context              context, cl_mem_flags            flags, const cl_image_format * image_format, const cl_image_desc *   image_desc, void *                  host_ptr, cl_int *                errcode_ret) {
    static std::atomic<decltype(&clCreateImage)> cached{nullptr};
    auto fn = kokoro_cl_loader::resolve("clCreateImage", cached, &clCreateImage);
    if (fn) return fn(context, flags, image_format, image_desc, host_ptr, errcode_ret);
    if (errcode_ret) *errcode_ret = CL_INVALID_OPERATION;
    return nullptr;
}

extern "C" cl_mem CL_API_CALL clCreatePipe(cl_context                 context, cl_mem_flags               flags, cl_uint                    pipe_packet_size, cl_uint                    pipe_max_packets, const cl_pipe_properties * properties, cl_int *                   errcode_ret) {
    static std::atomic<decltype(&clCreatePipe)> cached{nullptr};
    auto fn = kokoro_cl_loader::resolve("clCreatePipe", cached, &clCreatePipe);
    if (fn) return fn(context, flags, pipe_packet_size, pipe_max_packets, properties, errcode_ret);
    if (errcode_ret) *errcode_ret = CL_INVALID_OPERATION;
    return nullptr;
}

extern "C" cl_mem CL_API_CALL clCreateBufferWithProperties(cl_context                context, const cl_mem_properties * properties, cl_mem_flags              flags, size_t                    size, void *                    host_ptr, cl_int *                  errcode_ret) {
    static std::atomic<decltype(&clCreateBufferWithProperties)> cached{nullptr};
    auto fn = kokoro_cl_loader::resolve("clCreateBufferWithProperties", cached, &clCreateBufferWithProperties);
    if (fn) return fn(context, properties, flags, size, host_ptr, errcode_ret);
    if (errcode_ret) *errcode_ret = CL_INVALID_OPERATION;
    return nullptr;
}

extern "C" cl_mem CL_API_CALL clCreateImageWithProperties(cl_context                context, const cl_mem_properties * properties, cl_mem_flags              flags, const cl_image_format *   image_format, const cl_image_desc *     image_desc, void *                    host_ptr, cl_int *                  errcode_ret) {
    static std::atomic<decltype(&clCreateImageWithProperties)> cached{nullptr};
    auto fn = kokoro_cl_loader::resolve("clCreateImageWithProperties", cached, &clCreateImageWithProperties);
    if (fn) return fn(context, properties, flags, image_format, image_desc, host_ptr, errcode_ret);
    if (errcode_ret) *errcode_ret = CL_INVALID_OPERATION;
    return nullptr;
}

extern "C" cl_int CL_API_CALL clRetainMemObject(cl_mem memobj) {
    static std::atomic<decltype(&clRetainMemObject)> cached{nullptr};
    auto fn = kokoro_cl_loader::resolve("clRetainMemObject", cached, &clRetainMemObject);
    if (fn) return fn(memobj);
    return CL_INVALID_OPERATION;
}

extern "C" cl_int CL_API_CALL clReleaseMemObject(cl_mem memobj) {
    static std::atomic<decltype(&clReleaseMemObject)> cached{nullptr};
    auto fn = kokoro_cl_loader::resolve("clReleaseMemObject", cached, &clReleaseMemObject);
    if (fn) return fn(memobj);
    return CL_INVALID_OPERATION;
}

extern "C" cl_int CL_API_CALL clGetSupportedImageFormats(cl_context           context, cl_mem_flags         flags, cl_mem_object_type   image_type, cl_uint              num_entries, cl_image_format *    image_formats, cl_uint *            num_image_formats) {
    static std::atomic<decltype(&clGetSupportedImageFormats)> cached{nullptr};
    auto fn = kokoro_cl_loader::resolve("clGetSupportedImageFormats", cached, &clGetSupportedImageFormats);
    if (fn) return fn(context, flags, image_type, num_entries, image_formats, num_image_formats);
    return CL_INVALID_OPERATION;
}

extern "C" cl_int CL_API_CALL clGetMemObjectInfo(cl_mem           memobj, cl_mem_info      param_name, size_t           param_value_size, void *           param_value, size_t *         param_value_size_ret) {
    static std::atomic<decltype(&clGetMemObjectInfo)> cached{nullptr};
    auto fn = kokoro_cl_loader::resolve("clGetMemObjectInfo", cached, &clGetMemObjectInfo);
    if (fn) return fn(memobj, param_name, param_value_size, param_value, param_value_size_ret);
    return CL_INVALID_OPERATION;
}

extern "C" cl_int CL_API_CALL clGetImageInfo(cl_mem           image, cl_image_info    param_name, size_t           param_value_size, void *           param_value, size_t *         param_value_size_ret) {
    static std::atomic<decltype(&clGetImageInfo)> cached{nullptr};
    auto fn = kokoro_cl_loader::resolve("clGetImageInfo", cached, &clGetImageInfo);
    if (fn) return fn(image, param_name, param_value_size, param_value, param_value_size_ret);
    return CL_INVALID_OPERATION;
}

extern "C" cl_int CL_API_CALL clGetPipeInfo(cl_mem           pipe, cl_pipe_info     param_name, size_t           param_value_size, void *           param_value, size_t *         param_value_size_ret) {
    static std::atomic<decltype(&clGetPipeInfo)> cached{nullptr};
    auto fn = kokoro_cl_loader::resolve("clGetPipeInfo", cached, &clGetPipeInfo);
    if (fn) return fn(pipe, param_name, param_value_size, param_value, param_value_size_ret);
    return CL_INVALID_OPERATION;
}

extern "C" cl_int CL_API_CALL clSetMemObjectDestructorCallback(cl_mem memobj, void (CL_CALLBACK * pfn_notify)(cl_mem memobj,
                                                                 void * user_data), void * user_data) {
    static std::atomic<decltype(&clSetMemObjectDestructorCallback)> cached{nullptr};
    auto fn = kokoro_cl_loader::resolve("clSetMemObjectDestructorCallback", cached, &clSetMemObjectDestructorCallback);
    if (fn) return fn(memobj, pfn_notify, user_data);
    return CL_INVALID_OPERATION;
}

extern "C" void * CL_API_CALL clSVMAlloc(cl_context       context, cl_svm_mem_flags flags, size_t           size, cl_uint          alignment) {
    static std::atomic<decltype(&clSVMAlloc)> cached{nullptr};
    auto fn = kokoro_cl_loader::resolve("clSVMAlloc", cached, &clSVMAlloc);
    if (fn) return fn(context, flags, size, alignment);
    return nullptr;
}

extern "C" void CL_API_CALL clSVMFree(cl_context        context, void *            svm_pointer) {
    static std::atomic<decltype(&clSVMFree)> cached{nullptr};
    auto fn = kokoro_cl_loader::resolve("clSVMFree", cached, &clSVMFree);
    if (fn) fn(context, svm_pointer);
}

extern "C" cl_sampler CL_API_CALL clCreateSamplerWithProperties(cl_context                     context, const cl_sampler_properties *  sampler_properties, cl_int *                       errcode_ret) {
    static std::atomic<decltype(&clCreateSamplerWithProperties)> cached{nullptr};
    auto fn = kokoro_cl_loader::resolve("clCreateSamplerWithProperties", cached, &clCreateSamplerWithProperties);
    if (fn) return fn(context, sampler_properties, errcode_ret);
    if (errcode_ret) *errcode_ret = CL_INVALID_OPERATION;
    return nullptr;
}

extern "C" cl_int CL_API_CALL clRetainSampler(cl_sampler sampler) {
    static std::atomic<decltype(&clRetainSampler)> cached{nullptr};
    auto fn = kokoro_cl_loader::resolve("clRetainSampler", cached, &clRetainSampler);
    if (fn) return fn(sampler);
    return CL_INVALID_OPERATION;
}

extern "C" cl_int CL_API_CALL clReleaseSampler(cl_sampler sampler) {
    static std::atomic<decltype(&clReleaseSampler)> cached{nullptr};
    auto fn = kokoro_cl_loader::resolve("clReleaseSampler", cached, &clReleaseSampler);
    if (fn) return fn(sampler);
    return CL_INVALID_OPERATION;
}

extern "C" cl_int CL_API_CALL clGetSamplerInfo(cl_sampler         sampler, cl_sampler_info    param_name, size_t             param_value_size, void *             param_value, size_t *           param_value_size_ret) {
    static std::atomic<decltype(&clGetSamplerInfo)> cached{nullptr};
    auto fn = kokoro_cl_loader::resolve("clGetSamplerInfo", cached, &clGetSamplerInfo);
    if (fn) return fn(sampler, param_name, param_value_size, param_value, param_value_size_ret);
    return CL_INVALID_OPERATION;
}

extern "C" cl_program CL_API_CALL clCreateProgramWithSource(cl_context        context, cl_uint           count, const char **     strings, const size_t *    lengths, cl_int *          errcode_ret) {
    static std::atomic<decltype(&clCreateProgramWithSource)> cached{nullptr};
    auto fn = kokoro_cl_loader::resolve("clCreateProgramWithSource", cached, &clCreateProgramWithSource);
    if (fn) return fn(context, count, strings, lengths, errcode_ret);
    if (errcode_ret) *errcode_ret = CL_INVALID_OPERATION;
    return nullptr;
}

extern "C" cl_program CL_API_CALL clCreateProgramWithBinary(cl_context                     context, cl_uint                        num_devices, const cl_device_id *           device_list, const size_t *                 lengths, const unsigned char **         binaries, cl_int *                       binary_status, cl_int *                       errcode_ret) {
    static std::atomic<decltype(&clCreateProgramWithBinary)> cached{nullptr};
    auto fn = kokoro_cl_loader::resolve("clCreateProgramWithBinary", cached, &clCreateProgramWithBinary);
    if (fn) return fn(context, num_devices, device_list, lengths, binaries, binary_status, errcode_ret);
    if (errcode_ret) *errcode_ret = CL_INVALID_OPERATION;
    return nullptr;
}

extern "C" cl_program CL_API_CALL clCreateProgramWithBuiltInKernels(cl_context            context, cl_uint               num_devices, const cl_device_id *  device_list, const char *          kernel_names, cl_int *              errcode_ret) {
    static std::atomic<decltype(&clCreateProgramWithBuiltInKernels)> cached{nullptr};
    auto fn = kokoro_cl_loader::resolve("clCreateProgramWithBuiltInKernels", cached, &clCreateProgramWithBuiltInKernels);
    if (fn) return fn(context, num_devices, device_list, kernel_names, errcode_ret);
    if (errcode_ret) *errcode_ret = CL_INVALID_OPERATION;
    return nullptr;
}

extern "C" cl_program CL_API_CALL clCreateProgramWithIL(cl_context    context, const void*    il, size_t         length, cl_int*        errcode_ret) {
    static std::atomic<decltype(&clCreateProgramWithIL)> cached{nullptr};
    auto fn = kokoro_cl_loader::resolve("clCreateProgramWithIL", cached, &clCreateProgramWithIL);
    if (fn) return fn(context, il, length, errcode_ret);
    if (errcode_ret) *errcode_ret = CL_INVALID_OPERATION;
    return nullptr;
}

extern "C" cl_int CL_API_CALL clRetainProgram(cl_program program) {
    static std::atomic<decltype(&clRetainProgram)> cached{nullptr};
    auto fn = kokoro_cl_loader::resolve("clRetainProgram", cached, &clRetainProgram);
    if (fn) return fn(program);
    return CL_INVALID_OPERATION;
}

extern "C" cl_int CL_API_CALL clReleaseProgram(cl_program program) {
    static std::atomic<decltype(&clReleaseProgram)> cached{nullptr};
    auto fn = kokoro_cl_loader::resolve("clReleaseProgram", cached, &clReleaseProgram);
    if (fn) return fn(program);
    return CL_INVALID_OPERATION;
}

extern "C" cl_int CL_API_CALL clBuildProgram(cl_program           program, cl_uint              num_devices, const cl_device_id * device_list, const char *         options, void (CL_CALLBACK *  pfn_notify)(cl_program program,
                                                void * user_data), void *               user_data) {
    static std::atomic<decltype(&clBuildProgram)> cached{nullptr};
    auto fn = kokoro_cl_loader::resolve("clBuildProgram", cached, &clBuildProgram);
    if (fn) return fn(program, num_devices, device_list, options, pfn_notify, user_data);
    return CL_INVALID_OPERATION;
}

extern "C" cl_int CL_API_CALL clCompileProgram(cl_program           program, cl_uint              num_devices, const cl_device_id * device_list, const char *         options, cl_uint              num_input_headers, const cl_program *   input_headers, const char **        header_include_names, void (CL_CALLBACK *  pfn_notify)(cl_program program,
                                                  void * user_data), void *               user_data) {
    static std::atomic<decltype(&clCompileProgram)> cached{nullptr};
    auto fn = kokoro_cl_loader::resolve("clCompileProgram", cached, &clCompileProgram);
    if (fn) return fn(program, num_devices, device_list, options, num_input_headers, input_headers, header_include_names, pfn_notify, user_data);
    return CL_INVALID_OPERATION;
}

extern "C" cl_program CL_API_CALL clLinkProgram(cl_context           context, cl_uint              num_devices, const cl_device_id * device_list, const char *         options, cl_uint              num_input_programs, const cl_program *   input_programs, void (CL_CALLBACK *  pfn_notify)(cl_program program,
                                               void * user_data), void *               user_data, cl_int *             errcode_ret) {
    static std::atomic<decltype(&clLinkProgram)> cached{nullptr};
    auto fn = kokoro_cl_loader::resolve("clLinkProgram", cached, &clLinkProgram);
    if (fn) return fn(context, num_devices, device_list, options, num_input_programs, input_programs, pfn_notify, user_data, errcode_ret);
    if (errcode_ret) *errcode_ret = CL_INVALID_OPERATION;
    return nullptr;
}

extern "C" cl_int CL_API_CALL clSetProgramReleaseCallback(cl_program          program, void (CL_CALLBACK * pfn_notify)(cl_program program,
                                                            void * user_data), void *              user_data) {
    static std::atomic<decltype(&clSetProgramReleaseCallback)> cached{nullptr};
    auto fn = kokoro_cl_loader::resolve("clSetProgramReleaseCallback", cached, &clSetProgramReleaseCallback);
    if (fn) return fn(program, pfn_notify, user_data);
    return CL_INVALID_OPERATION;
}

extern "C" cl_int CL_API_CALL clSetProgramSpecializationConstant(cl_program  program, cl_uint     spec_id, size_t      spec_size, const void* spec_value) {
    static std::atomic<decltype(&clSetProgramSpecializationConstant)> cached{nullptr};
    auto fn = kokoro_cl_loader::resolve("clSetProgramSpecializationConstant", cached, &clSetProgramSpecializationConstant);
    if (fn) return fn(program, spec_id, spec_size, spec_value);
    return CL_INVALID_OPERATION;
}

extern "C" cl_int CL_API_CALL clUnloadPlatformCompiler(cl_platform_id platform) {
    static std::atomic<decltype(&clUnloadPlatformCompiler)> cached{nullptr};
    auto fn = kokoro_cl_loader::resolve("clUnloadPlatformCompiler", cached, &clUnloadPlatformCompiler);
    if (fn) return fn(platform);
    return CL_INVALID_OPERATION;
}

extern "C" cl_int CL_API_CALL clGetProgramInfo(cl_program         program, cl_program_info    param_name, size_t             param_value_size, void *             param_value, size_t *           param_value_size_ret) {
    static std::atomic<decltype(&clGetProgramInfo)> cached{nullptr};
    auto fn = kokoro_cl_loader::resolve("clGetProgramInfo", cached, &clGetProgramInfo);
    if (fn) return fn(program, param_name, param_value_size, param_value, param_value_size_ret);
    return CL_INVALID_OPERATION;
}

extern "C" cl_int CL_API_CALL clGetProgramBuildInfo(cl_program            program, cl_device_id          device, cl_program_build_info param_name, size_t                param_value_size, void *                param_value, size_t *              param_value_size_ret) {
    static std::atomic<decltype(&clGetProgramBuildInfo)> cached{nullptr};
    auto fn = kokoro_cl_loader::resolve("clGetProgramBuildInfo", cached, &clGetProgramBuildInfo);
    if (fn) return fn(program, device, param_name, param_value_size, param_value, param_value_size_ret);
    return CL_INVALID_OPERATION;
}

extern "C" cl_kernel CL_API_CALL clCreateKernel(cl_program      program, const char *    kernel_name, cl_int *        errcode_ret) {
    static std::atomic<decltype(&clCreateKernel)> cached{nullptr};
    auto fn = kokoro_cl_loader::resolve("clCreateKernel", cached, &clCreateKernel);
    if (fn) return fn(program, kernel_name, errcode_ret);
    if (errcode_ret) *errcode_ret = CL_INVALID_OPERATION;
    return nullptr;
}

extern "C" cl_int CL_API_CALL clCreateKernelsInProgram(cl_program     program, cl_uint        num_kernels, cl_kernel *    kernels, cl_uint *      num_kernels_ret) {
    static std::atomic<decltype(&clCreateKernelsInProgram)> cached{nullptr};
    auto fn = kokoro_cl_loader::resolve("clCreateKernelsInProgram", cached, &clCreateKernelsInProgram);
    if (fn) return fn(program, num_kernels, kernels, num_kernels_ret);
    return CL_INVALID_OPERATION;
}

extern "C" cl_kernel CL_API_CALL clCloneKernel(cl_kernel     source_kernel, cl_int*       errcode_ret) {
    static std::atomic<decltype(&clCloneKernel)> cached{nullptr};
    auto fn = kokoro_cl_loader::resolve("clCloneKernel", cached, &clCloneKernel);
    if (fn) return fn(source_kernel, errcode_ret);
    if (errcode_ret) *errcode_ret = CL_INVALID_OPERATION;
    return nullptr;
}

extern "C" cl_int CL_API_CALL clRetainKernel(cl_kernel    kernel) {
    static std::atomic<decltype(&clRetainKernel)> cached{nullptr};
    auto fn = kokoro_cl_loader::resolve("clRetainKernel", cached, &clRetainKernel);
    if (fn) return fn(kernel);
    return CL_INVALID_OPERATION;
}

extern "C" cl_int CL_API_CALL clReleaseKernel(cl_kernel   kernel) {
    static std::atomic<decltype(&clReleaseKernel)> cached{nullptr};
    auto fn = kokoro_cl_loader::resolve("clReleaseKernel", cached, &clReleaseKernel);
    if (fn) return fn(kernel);
    return CL_INVALID_OPERATION;
}

extern "C" cl_int CL_API_CALL clSetKernelArg(cl_kernel    kernel, cl_uint      arg_index, size_t       arg_size, const void * arg_value) {
    static std::atomic<decltype(&clSetKernelArg)> cached{nullptr};
    auto fn = kokoro_cl_loader::resolve("clSetKernelArg", cached, &clSetKernelArg);
    if (fn) return fn(kernel, arg_index, arg_size, arg_value);
    return CL_INVALID_OPERATION;
}

extern "C" cl_int CL_API_CALL clSetKernelArgSVMPointer(cl_kernel    kernel, cl_uint      arg_index, const void * arg_value) {
    static std::atomic<decltype(&clSetKernelArgSVMPointer)> cached{nullptr};
    auto fn = kokoro_cl_loader::resolve("clSetKernelArgSVMPointer", cached, &clSetKernelArgSVMPointer);
    if (fn) return fn(kernel, arg_index, arg_value);
    return CL_INVALID_OPERATION;
}

extern "C" cl_int CL_API_CALL clSetKernelExecInfo(cl_kernel            kernel, cl_kernel_exec_info  param_name, size_t               param_value_size, const void *         param_value) {
    static std::atomic<decltype(&clSetKernelExecInfo)> cached{nullptr};
    auto fn = kokoro_cl_loader::resolve("clSetKernelExecInfo", cached, &clSetKernelExecInfo);
    if (fn) return fn(kernel, param_name, param_value_size, param_value);
    return CL_INVALID_OPERATION;
}

extern "C" cl_int CL_API_CALL clGetKernelInfo(cl_kernel       kernel, cl_kernel_info  param_name, size_t          param_value_size, void *          param_value, size_t *        param_value_size_ret) {
    static std::atomic<decltype(&clGetKernelInfo)> cached{nullptr};
    auto fn = kokoro_cl_loader::resolve("clGetKernelInfo", cached, &clGetKernelInfo);
    if (fn) return fn(kernel, param_name, param_value_size, param_value, param_value_size_ret);
    return CL_INVALID_OPERATION;
}

extern "C" cl_int CL_API_CALL clGetKernelArgInfo(cl_kernel       kernel, cl_uint         arg_indx, cl_kernel_arg_info  param_name, size_t          param_value_size, void *          param_value, size_t *        param_value_size_ret) {
    static std::atomic<decltype(&clGetKernelArgInfo)> cached{nullptr};
    auto fn = kokoro_cl_loader::resolve("clGetKernelArgInfo", cached, &clGetKernelArgInfo);
    if (fn) return fn(kernel, arg_indx, param_name, param_value_size, param_value, param_value_size_ret);
    return CL_INVALID_OPERATION;
}

extern "C" cl_int CL_API_CALL clGetKernelWorkGroupInfo(cl_kernel                  kernel, cl_device_id               device, cl_kernel_work_group_info  param_name, size_t                     param_value_size, void *                     param_value, size_t *                   param_value_size_ret) {
    static std::atomic<decltype(&clGetKernelWorkGroupInfo)> cached{nullptr};
    auto fn = kokoro_cl_loader::resolve("clGetKernelWorkGroupInfo", cached, &clGetKernelWorkGroupInfo);
    if (fn) return fn(kernel, device, param_name, param_value_size, param_value, param_value_size_ret);
    return CL_INVALID_OPERATION;
}

extern "C" cl_int CL_API_CALL clGetKernelSubGroupInfo(cl_kernel                   kernel, cl_device_id                device, cl_kernel_sub_group_info    param_name, size_t                      input_value_size, const void*                 input_value, size_t                      param_value_size, void*                       param_value, size_t*                     param_value_size_ret) {
    static std::atomic<decltype(&clGetKernelSubGroupInfo)> cached{nullptr};
    auto fn = kokoro_cl_loader::resolve("clGetKernelSubGroupInfo", cached, &clGetKernelSubGroupInfo);
    if (fn) return fn(kernel, device, param_name, input_value_size, input_value, param_value_size, param_value, param_value_size_ret);
    return CL_INVALID_OPERATION;
}

extern "C" cl_int CL_API_CALL clWaitForEvents(cl_uint             num_events, const cl_event *    event_list) {
    static std::atomic<decltype(&clWaitForEvents)> cached{nullptr};
    auto fn = kokoro_cl_loader::resolve("clWaitForEvents", cached, &clWaitForEvents);
    if (fn) return fn(num_events, event_list);
    return CL_INVALID_OPERATION;
}

extern "C" cl_int CL_API_CALL clGetEventInfo(cl_event         event, cl_event_info    param_name, size_t           param_value_size, void *           param_value, size_t *         param_value_size_ret) {
    static std::atomic<decltype(&clGetEventInfo)> cached{nullptr};
    auto fn = kokoro_cl_loader::resolve("clGetEventInfo", cached, &clGetEventInfo);
    if (fn) return fn(event, param_name, param_value_size, param_value, param_value_size_ret);
    return CL_INVALID_OPERATION;
}

extern "C" cl_event CL_API_CALL clCreateUserEvent(cl_context    context, cl_int *      errcode_ret) {
    static std::atomic<decltype(&clCreateUserEvent)> cached{nullptr};
    auto fn = kokoro_cl_loader::resolve("clCreateUserEvent", cached, &clCreateUserEvent);
    if (fn) return fn(context, errcode_ret);
    if (errcode_ret) *errcode_ret = CL_INVALID_OPERATION;
    return nullptr;
}

extern "C" cl_int CL_API_CALL clRetainEvent(cl_event event) {
    static std::atomic<decltype(&clRetainEvent)> cached{nullptr};
    auto fn = kokoro_cl_loader::resolve("clRetainEvent", cached, &clRetainEvent);
    if (fn) return fn(event);
    return CL_INVALID_OPERATION;
}

extern "C" cl_int CL_API_CALL clReleaseEvent(cl_event event) {
    static std::atomic<decltype(&clReleaseEvent)> cached{nullptr};
    auto fn = kokoro_cl_loader::resolve("clReleaseEvent", cached, &clReleaseEvent);
    if (fn) return fn(event);
    return CL_INVALID_OPERATION;
}

extern "C" cl_int CL_API_CALL clSetUserEventStatus(cl_event   event, cl_int     execution_status) {
    static std::atomic<decltype(&clSetUserEventStatus)> cached{nullptr};
    auto fn = kokoro_cl_loader::resolve("clSetUserEventStatus", cached, &clSetUserEventStatus);
    if (fn) return fn(event, execution_status);
    return CL_INVALID_OPERATION;
}

extern "C" cl_int CL_API_CALL clSetEventCallback(cl_event    event, cl_int      command_exec_callback_type, void (CL_CALLBACK * pfn_notify)(cl_event event,
                                                   cl_int   event_command_status,
                                                   void *   user_data), void *      user_data) {
    static std::atomic<decltype(&clSetEventCallback)> cached{nullptr};
    auto fn = kokoro_cl_loader::resolve("clSetEventCallback", cached, &clSetEventCallback);
    if (fn) return fn(event, command_exec_callback_type, pfn_notify, user_data);
    return CL_INVALID_OPERATION;
}

extern "C" cl_int CL_API_CALL clGetEventProfilingInfo(cl_event            event, cl_profiling_info   param_name, size_t              param_value_size, void *              param_value, size_t *            param_value_size_ret) {
    static std::atomic<decltype(&clGetEventProfilingInfo)> cached{nullptr};
    auto fn = kokoro_cl_loader::resolve("clGetEventProfilingInfo", cached, &clGetEventProfilingInfo);
    if (fn) return fn(event, param_name, param_value_size, param_value, param_value_size_ret);
    return CL_INVALID_OPERATION;
}

extern "C" cl_int CL_API_CALL clFlush(cl_command_queue command_queue) {
    static std::atomic<decltype(&clFlush)> cached{nullptr};
    auto fn = kokoro_cl_loader::resolve("clFlush", cached, &clFlush);
    if (fn) return fn(command_queue);
    return CL_INVALID_OPERATION;
}

extern "C" cl_int CL_API_CALL clFinish(cl_command_queue command_queue) {
    static std::atomic<decltype(&clFinish)> cached{nullptr};
    auto fn = kokoro_cl_loader::resolve("clFinish", cached, &clFinish);
    if (fn) return fn(command_queue);
    return CL_INVALID_OPERATION;
}

extern "C" cl_int CL_API_CALL clEnqueueReadBuffer(cl_command_queue    command_queue, cl_mem              buffer, cl_bool             blocking_read, size_t              offset, size_t              size, void *              ptr, cl_uint             num_events_in_wait_list, const cl_event *    event_wait_list, cl_event *          event) {
    static std::atomic<decltype(&clEnqueueReadBuffer)> cached{nullptr};
    auto fn = kokoro_cl_loader::resolve("clEnqueueReadBuffer", cached, &clEnqueueReadBuffer);
    if (fn) return fn(command_queue, buffer, blocking_read, offset, size, ptr, num_events_in_wait_list, event_wait_list, event);
    return CL_INVALID_OPERATION;
}

extern "C" cl_int CL_API_CALL clEnqueueReadBufferRect(cl_command_queue    command_queue, cl_mem              buffer, cl_bool             blocking_read, const size_t *      buffer_origin, const size_t *      host_origin, const size_t *      region, size_t              buffer_row_pitch, size_t              buffer_slice_pitch, size_t              host_row_pitch, size_t              host_slice_pitch, void *              ptr, cl_uint             num_events_in_wait_list, const cl_event *    event_wait_list, cl_event *          event) {
    static std::atomic<decltype(&clEnqueueReadBufferRect)> cached{nullptr};
    auto fn = kokoro_cl_loader::resolve("clEnqueueReadBufferRect", cached, &clEnqueueReadBufferRect);
    if (fn) return fn(command_queue, buffer, blocking_read, buffer_origin, host_origin, region, buffer_row_pitch, buffer_slice_pitch, host_row_pitch, host_slice_pitch, ptr, num_events_in_wait_list, event_wait_list, event);
    return CL_INVALID_OPERATION;
}

extern "C" cl_int CL_API_CALL clEnqueueWriteBuffer(cl_command_queue   command_queue, cl_mem             buffer, cl_bool            blocking_write, size_t             offset, size_t             size, const void *       ptr, cl_uint            num_events_in_wait_list, const cl_event *   event_wait_list, cl_event *         event) {
    static std::atomic<decltype(&clEnqueueWriteBuffer)> cached{nullptr};
    auto fn = kokoro_cl_loader::resolve("clEnqueueWriteBuffer", cached, &clEnqueueWriteBuffer);
    if (fn) return fn(command_queue, buffer, blocking_write, offset, size, ptr, num_events_in_wait_list, event_wait_list, event);
    return CL_INVALID_OPERATION;
}

extern "C" cl_int CL_API_CALL clEnqueueWriteBufferRect(cl_command_queue    command_queue, cl_mem              buffer, cl_bool             blocking_write, const size_t *      buffer_origin, const size_t *      host_origin, const size_t *      region, size_t              buffer_row_pitch, size_t              buffer_slice_pitch, size_t              host_row_pitch, size_t              host_slice_pitch, const void *        ptr, cl_uint             num_events_in_wait_list, const cl_event *    event_wait_list, cl_event *          event) {
    static std::atomic<decltype(&clEnqueueWriteBufferRect)> cached{nullptr};
    auto fn = kokoro_cl_loader::resolve("clEnqueueWriteBufferRect", cached, &clEnqueueWriteBufferRect);
    if (fn) return fn(command_queue, buffer, blocking_write, buffer_origin, host_origin, region, buffer_row_pitch, buffer_slice_pitch, host_row_pitch, host_slice_pitch, ptr, num_events_in_wait_list, event_wait_list, event);
    return CL_INVALID_OPERATION;
}

extern "C" cl_int CL_API_CALL clEnqueueFillBuffer(cl_command_queue   command_queue, cl_mem             buffer, const void *       pattern, size_t             pattern_size, size_t             offset, size_t             size, cl_uint            num_events_in_wait_list, const cl_event *   event_wait_list, cl_event *         event) {
    static std::atomic<decltype(&clEnqueueFillBuffer)> cached{nullptr};
    auto fn = kokoro_cl_loader::resolve("clEnqueueFillBuffer", cached, &clEnqueueFillBuffer);
    if (fn) return fn(command_queue, buffer, pattern, pattern_size, offset, size, num_events_in_wait_list, event_wait_list, event);
    return CL_INVALID_OPERATION;
}

extern "C" cl_int CL_API_CALL clEnqueueCopyBuffer(cl_command_queue    command_queue, cl_mem              src_buffer, cl_mem              dst_buffer, size_t              src_offset, size_t              dst_offset, size_t              size, cl_uint             num_events_in_wait_list, const cl_event *    event_wait_list, cl_event *          event) {
    static std::atomic<decltype(&clEnqueueCopyBuffer)> cached{nullptr};
    auto fn = kokoro_cl_loader::resolve("clEnqueueCopyBuffer", cached, &clEnqueueCopyBuffer);
    if (fn) return fn(command_queue, src_buffer, dst_buffer, src_offset, dst_offset, size, num_events_in_wait_list, event_wait_list, event);
    return CL_INVALID_OPERATION;
}

extern "C" cl_int CL_API_CALL clEnqueueCopyBufferRect(cl_command_queue    command_queue, cl_mem              src_buffer, cl_mem              dst_buffer, const size_t *      src_origin, const size_t *      dst_origin, const size_t *      region, size_t              src_row_pitch, size_t              src_slice_pitch, size_t              dst_row_pitch, size_t              dst_slice_pitch, cl_uint             num_events_in_wait_list, const cl_event *    event_wait_list, cl_event *          event) {
    static std::atomic<decltype(&clEnqueueCopyBufferRect)> cached{nullptr};
    auto fn = kokoro_cl_loader::resolve("clEnqueueCopyBufferRect", cached, &clEnqueueCopyBufferRect);
    if (fn) return fn(command_queue, src_buffer, dst_buffer, src_origin, dst_origin, region, src_row_pitch, src_slice_pitch, dst_row_pitch, dst_slice_pitch, num_events_in_wait_list, event_wait_list, event);
    return CL_INVALID_OPERATION;
}

extern "C" cl_int CL_API_CALL clEnqueueReadImage(cl_command_queue     command_queue, cl_mem               image, cl_bool              blocking_read, const size_t *       origin, const size_t *       region, size_t               row_pitch, size_t               slice_pitch, void *               ptr, cl_uint              num_events_in_wait_list, const cl_event *     event_wait_list, cl_event *           event) {
    static std::atomic<decltype(&clEnqueueReadImage)> cached{nullptr};
    auto fn = kokoro_cl_loader::resolve("clEnqueueReadImage", cached, &clEnqueueReadImage);
    if (fn) return fn(command_queue, image, blocking_read, origin, region, row_pitch, slice_pitch, ptr, num_events_in_wait_list, event_wait_list, event);
    return CL_INVALID_OPERATION;
}

extern "C" cl_int CL_API_CALL clEnqueueWriteImage(cl_command_queue    command_queue, cl_mem              image, cl_bool             blocking_write, const size_t *      origin, const size_t *      region, size_t              input_row_pitch, size_t              input_slice_pitch, const void *        ptr, cl_uint             num_events_in_wait_list, const cl_event *    event_wait_list, cl_event *          event) {
    static std::atomic<decltype(&clEnqueueWriteImage)> cached{nullptr};
    auto fn = kokoro_cl_loader::resolve("clEnqueueWriteImage", cached, &clEnqueueWriteImage);
    if (fn) return fn(command_queue, image, blocking_write, origin, region, input_row_pitch, input_slice_pitch, ptr, num_events_in_wait_list, event_wait_list, event);
    return CL_INVALID_OPERATION;
}

extern "C" cl_int CL_API_CALL clEnqueueFillImage(cl_command_queue   command_queue, cl_mem             image, const void *       fill_color, const size_t *     origin, const size_t *     region, cl_uint            num_events_in_wait_list, const cl_event *   event_wait_list, cl_event *         event) {
    static std::atomic<decltype(&clEnqueueFillImage)> cached{nullptr};
    auto fn = kokoro_cl_loader::resolve("clEnqueueFillImage", cached, &clEnqueueFillImage);
    if (fn) return fn(command_queue, image, fill_color, origin, region, num_events_in_wait_list, event_wait_list, event);
    return CL_INVALID_OPERATION;
}

extern "C" cl_int CL_API_CALL clEnqueueCopyImage(cl_command_queue     command_queue, cl_mem               src_image, cl_mem               dst_image, const size_t *       src_origin, const size_t *       dst_origin, const size_t *       region, cl_uint              num_events_in_wait_list, const cl_event *     event_wait_list, cl_event *           event) {
    static std::atomic<decltype(&clEnqueueCopyImage)> cached{nullptr};
    auto fn = kokoro_cl_loader::resolve("clEnqueueCopyImage", cached, &clEnqueueCopyImage);
    if (fn) return fn(command_queue, src_image, dst_image, src_origin, dst_origin, region, num_events_in_wait_list, event_wait_list, event);
    return CL_INVALID_OPERATION;
}

extern "C" cl_int CL_API_CALL clEnqueueCopyImageToBuffer(cl_command_queue command_queue, cl_mem           src_image, cl_mem           dst_buffer, const size_t *   src_origin, const size_t *   region, size_t           dst_offset, cl_uint          num_events_in_wait_list, const cl_event * event_wait_list, cl_event *       event) {
    static std::atomic<decltype(&clEnqueueCopyImageToBuffer)> cached{nullptr};
    auto fn = kokoro_cl_loader::resolve("clEnqueueCopyImageToBuffer", cached, &clEnqueueCopyImageToBuffer);
    if (fn) return fn(command_queue, src_image, dst_buffer, src_origin, region, dst_offset, num_events_in_wait_list, event_wait_list, event);
    return CL_INVALID_OPERATION;
}

extern "C" cl_int CL_API_CALL clEnqueueCopyBufferToImage(cl_command_queue command_queue, cl_mem           src_buffer, cl_mem           dst_image, size_t           src_offset, const size_t *   dst_origin, const size_t *   region, cl_uint          num_events_in_wait_list, const cl_event * event_wait_list, cl_event *       event) {
    static std::atomic<decltype(&clEnqueueCopyBufferToImage)> cached{nullptr};
    auto fn = kokoro_cl_loader::resolve("clEnqueueCopyBufferToImage", cached, &clEnqueueCopyBufferToImage);
    if (fn) return fn(command_queue, src_buffer, dst_image, src_offset, dst_origin, region, num_events_in_wait_list, event_wait_list, event);
    return CL_INVALID_OPERATION;
}

extern "C" void * CL_API_CALL clEnqueueMapBuffer(cl_command_queue command_queue, cl_mem           buffer, cl_bool          blocking_map, cl_map_flags     map_flags, size_t           offset, size_t           size, cl_uint          num_events_in_wait_list, const cl_event * event_wait_list, cl_event *       event, cl_int *         errcode_ret) {
    static std::atomic<decltype(&clEnqueueMapBuffer)> cached{nullptr};
    auto fn = kokoro_cl_loader::resolve("clEnqueueMapBuffer", cached, &clEnqueueMapBuffer);
    if (fn) return fn(command_queue, buffer, blocking_map, map_flags, offset, size, num_events_in_wait_list, event_wait_list, event, errcode_ret);
    if (errcode_ret) *errcode_ret = CL_INVALID_OPERATION;
    return nullptr;
}

extern "C" void * CL_API_CALL clEnqueueMapImage(cl_command_queue  command_queue, cl_mem            image, cl_bool           blocking_map, cl_map_flags      map_flags, const size_t *    origin, const size_t *    region, size_t *          image_row_pitch, size_t *          image_slice_pitch, cl_uint           num_events_in_wait_list, const cl_event *  event_wait_list, cl_event *        event, cl_int *          errcode_ret) {
    static std::atomic<decltype(&clEnqueueMapImage)> cached{nullptr};
    auto fn = kokoro_cl_loader::resolve("clEnqueueMapImage", cached, &clEnqueueMapImage);
    if (fn) return fn(command_queue, image, blocking_map, map_flags, origin, region, image_row_pitch, image_slice_pitch, num_events_in_wait_list, event_wait_list, event, errcode_ret);
    if (errcode_ret) *errcode_ret = CL_INVALID_OPERATION;
    return nullptr;
}

extern "C" cl_int CL_API_CALL clEnqueueUnmapMemObject(cl_command_queue command_queue, cl_mem           memobj, void *           mapped_ptr, cl_uint          num_events_in_wait_list, const cl_event * event_wait_list, cl_event *       event) {
    static std::atomic<decltype(&clEnqueueUnmapMemObject)> cached{nullptr};
    auto fn = kokoro_cl_loader::resolve("clEnqueueUnmapMemObject", cached, &clEnqueueUnmapMemObject);
    if (fn) return fn(command_queue, memobj, mapped_ptr, num_events_in_wait_list, event_wait_list, event);
    return CL_INVALID_OPERATION;
}

extern "C" cl_int CL_API_CALL clEnqueueMigrateMemObjects(cl_command_queue       command_queue, cl_uint                num_mem_objects, const cl_mem *         mem_objects, cl_mem_migration_flags flags, cl_uint                num_events_in_wait_list, const cl_event *       event_wait_list, cl_event *             event) {
    static std::atomic<decltype(&clEnqueueMigrateMemObjects)> cached{nullptr};
    auto fn = kokoro_cl_loader::resolve("clEnqueueMigrateMemObjects", cached, &clEnqueueMigrateMemObjects);
    if (fn) return fn(command_queue, num_mem_objects, mem_objects, flags, num_events_in_wait_list, event_wait_list, event);
    return CL_INVALID_OPERATION;
}

extern "C" cl_int CL_API_CALL clEnqueueNDRangeKernel(cl_command_queue command_queue, cl_kernel        kernel, cl_uint          work_dim, const size_t *   global_work_offset, const size_t *   global_work_size, const size_t *   local_work_size, cl_uint          num_events_in_wait_list, const cl_event * event_wait_list, cl_event *       event) {
    static std::atomic<decltype(&clEnqueueNDRangeKernel)> cached{nullptr};
    auto fn = kokoro_cl_loader::resolve("clEnqueueNDRangeKernel", cached, &clEnqueueNDRangeKernel);
    if (fn) return fn(command_queue, kernel, work_dim, global_work_offset, global_work_size, local_work_size, num_events_in_wait_list, event_wait_list, event);
    return CL_INVALID_OPERATION;
}

extern "C" cl_int CL_API_CALL clEnqueueNativeKernel(cl_command_queue  command_queue, void (CL_CALLBACK * user_func)(void *), void *            args, size_t            cb_args, cl_uint           num_mem_objects, const cl_mem *    mem_list, const void **     args_mem_loc, cl_uint           num_events_in_wait_list, const cl_event *  event_wait_list, cl_event *        event) {
    static std::atomic<decltype(&clEnqueueNativeKernel)> cached{nullptr};
    auto fn = kokoro_cl_loader::resolve("clEnqueueNativeKernel", cached, &clEnqueueNativeKernel);
    if (fn) return fn(command_queue, user_func, args, cb_args, num_mem_objects, mem_list, args_mem_loc, num_events_in_wait_list, event_wait_list, event);
    return CL_INVALID_OPERATION;
}

extern "C" cl_int CL_API_CALL clEnqueueMarkerWithWaitList(cl_command_queue  command_queue, cl_uint           num_events_in_wait_list, const cl_event *  event_wait_list, cl_event *        event) {
    static std::atomic<decltype(&clEnqueueMarkerWithWaitList)> cached{nullptr};
    auto fn = kokoro_cl_loader::resolve("clEnqueueMarkerWithWaitList", cached, &clEnqueueMarkerWithWaitList);
    if (fn) return fn(command_queue, num_events_in_wait_list, event_wait_list, event);
    return CL_INVALID_OPERATION;
}

extern "C" cl_int CL_API_CALL clEnqueueBarrierWithWaitList(cl_command_queue  command_queue, cl_uint           num_events_in_wait_list, const cl_event *  event_wait_list, cl_event *        event) {
    static std::atomic<decltype(&clEnqueueBarrierWithWaitList)> cached{nullptr};
    auto fn = kokoro_cl_loader::resolve("clEnqueueBarrierWithWaitList", cached, &clEnqueueBarrierWithWaitList);
    if (fn) return fn(command_queue, num_events_in_wait_list, event_wait_list, event);
    return CL_INVALID_OPERATION;
}

extern "C" cl_int CL_API_CALL clEnqueueSVMFree(cl_command_queue  command_queue, cl_uint           num_svm_pointers, void *            svm_pointers[], void (CL_CALLBACK * pfn_free_func)(cl_command_queue queue,
                                                    cl_uint          num_svm_pointers,
                                                    void *           svm_pointers[],
                                                    void *           user_data), void *            user_data, cl_uint           num_events_in_wait_list, const cl_event *  event_wait_list, cl_event *        event) {
    static std::atomic<decltype(&clEnqueueSVMFree)> cached{nullptr};
    auto fn = kokoro_cl_loader::resolve("clEnqueueSVMFree", cached, &clEnqueueSVMFree);
    if (fn) return fn(command_queue, num_svm_pointers, svm_pointers, pfn_free_func, user_data, num_events_in_wait_list, event_wait_list, event);
    return CL_INVALID_OPERATION;
}

extern "C" cl_int CL_API_CALL clEnqueueSVMMemcpy(cl_command_queue  command_queue, cl_bool           blocking_copy, void *            dst_ptr, const void *      src_ptr, size_t            size, cl_uint           num_events_in_wait_list, const cl_event *  event_wait_list, cl_event *        event) {
    static std::atomic<decltype(&clEnqueueSVMMemcpy)> cached{nullptr};
    auto fn = kokoro_cl_loader::resolve("clEnqueueSVMMemcpy", cached, &clEnqueueSVMMemcpy);
    if (fn) return fn(command_queue, blocking_copy, dst_ptr, src_ptr, size, num_events_in_wait_list, event_wait_list, event);
    return CL_INVALID_OPERATION;
}

extern "C" cl_int CL_API_CALL clEnqueueSVMMemFill(cl_command_queue  command_queue, void *            svm_ptr, const void *      pattern, size_t            pattern_size, size_t            size, cl_uint           num_events_in_wait_list, const cl_event *  event_wait_list, cl_event *        event) {
    static std::atomic<decltype(&clEnqueueSVMMemFill)> cached{nullptr};
    auto fn = kokoro_cl_loader::resolve("clEnqueueSVMMemFill", cached, &clEnqueueSVMMemFill);
    if (fn) return fn(command_queue, svm_ptr, pattern, pattern_size, size, num_events_in_wait_list, event_wait_list, event);
    return CL_INVALID_OPERATION;
}

extern "C" cl_int CL_API_CALL clEnqueueSVMMap(cl_command_queue  command_queue, cl_bool           blocking_map, cl_map_flags      flags, void *            svm_ptr, size_t            size, cl_uint           num_events_in_wait_list, const cl_event *  event_wait_list, cl_event *        event) {
    static std::atomic<decltype(&clEnqueueSVMMap)> cached{nullptr};
    auto fn = kokoro_cl_loader::resolve("clEnqueueSVMMap", cached, &clEnqueueSVMMap);
    if (fn) return fn(command_queue, blocking_map, flags, svm_ptr, size, num_events_in_wait_list, event_wait_list, event);
    return CL_INVALID_OPERATION;
}

extern "C" cl_int CL_API_CALL clEnqueueSVMUnmap(cl_command_queue  command_queue, void *            svm_ptr, cl_uint           num_events_in_wait_list, const cl_event *  event_wait_list, cl_event *        event) {
    static std::atomic<decltype(&clEnqueueSVMUnmap)> cached{nullptr};
    auto fn = kokoro_cl_loader::resolve("clEnqueueSVMUnmap", cached, &clEnqueueSVMUnmap);
    if (fn) return fn(command_queue, svm_ptr, num_events_in_wait_list, event_wait_list, event);
    return CL_INVALID_OPERATION;
}

extern "C" cl_int CL_API_CALL clEnqueueSVMMigrateMem(cl_command_queue         command_queue, cl_uint                  num_svm_pointers, const void **            svm_pointers, const size_t *           sizes, cl_mem_migration_flags   flags, cl_uint                  num_events_in_wait_list, const cl_event *         event_wait_list, cl_event *               event) {
    static std::atomic<decltype(&clEnqueueSVMMigrateMem)> cached{nullptr};
    auto fn = kokoro_cl_loader::resolve("clEnqueueSVMMigrateMem", cached, &clEnqueueSVMMigrateMem);
    if (fn) return fn(command_queue, num_svm_pointers, svm_pointers, sizes, flags, num_events_in_wait_list, event_wait_list, event);
    return CL_INVALID_OPERATION;
}

extern "C" void * CL_API_CALL clGetExtensionFunctionAddressForPlatform(cl_platform_id platform, const char *   func_name) {
    static std::atomic<decltype(&clGetExtensionFunctionAddressForPlatform)> cached{nullptr};
    auto fn = kokoro_cl_loader::resolve("clGetExtensionFunctionAddressForPlatform", cached, &clGetExtensionFunctionAddressForPlatform);
    if (fn) return fn(platform, func_name);
    return nullptr;
}

extern "C" cl_int CL_API_CALL clSetCommandQueueProperty(cl_command_queue              command_queue, cl_command_queue_properties   properties, cl_bool                       enable, cl_command_queue_properties * old_properties) {
    static std::atomic<decltype(&clSetCommandQueueProperty)> cached{nullptr};
    auto fn = kokoro_cl_loader::resolve("clSetCommandQueueProperty", cached, &clSetCommandQueueProperty);
    if (fn) return fn(command_queue, properties, enable, old_properties);
    return CL_INVALID_OPERATION;
}

extern "C" cl_mem CL_API_CALL clCreateImage2D(cl_context              context, cl_mem_flags            flags, const cl_image_format * image_format, size_t                  image_width, size_t                  image_height, size_t                  image_row_pitch, void *                  host_ptr, cl_int *                errcode_ret) {
    static std::atomic<decltype(&clCreateImage2D)> cached{nullptr};
    auto fn = kokoro_cl_loader::resolve("clCreateImage2D", cached, &clCreateImage2D);
    if (fn) return fn(context, flags, image_format, image_width, image_height, image_row_pitch, host_ptr, errcode_ret);
    if (errcode_ret) *errcode_ret = CL_INVALID_OPERATION;
    return nullptr;
}

extern "C" cl_mem CL_API_CALL clCreateImage3D(cl_context              context, cl_mem_flags            flags, const cl_image_format * image_format, size_t                  image_width, size_t                  image_height, size_t                  image_depth, size_t                  image_row_pitch, size_t                  image_slice_pitch, void *                  host_ptr, cl_int *                errcode_ret) {
    static std::atomic<decltype(&clCreateImage3D)> cached{nullptr};
    auto fn = kokoro_cl_loader::resolve("clCreateImage3D", cached, &clCreateImage3D);
    if (fn) return fn(context, flags, image_format, image_width, image_height, image_depth, image_row_pitch, image_slice_pitch, host_ptr, errcode_ret);
    if (errcode_ret) *errcode_ret = CL_INVALID_OPERATION;
    return nullptr;
}

extern "C" cl_int CL_API_CALL clEnqueueMarker(cl_command_queue    command_queue, cl_event *          event) {
    static std::atomic<decltype(&clEnqueueMarker)> cached{nullptr};
    auto fn = kokoro_cl_loader::resolve("clEnqueueMarker", cached, &clEnqueueMarker);
    if (fn) return fn(command_queue, event);
    return CL_INVALID_OPERATION;
}

extern "C" cl_int CL_API_CALL clEnqueueWaitForEvents(cl_command_queue  command_queue, cl_uint          num_events, const cl_event * event_list) {
    static std::atomic<decltype(&clEnqueueWaitForEvents)> cached{nullptr};
    auto fn = kokoro_cl_loader::resolve("clEnqueueWaitForEvents", cached, &clEnqueueWaitForEvents);
    if (fn) return fn(command_queue, num_events, event_list);
    return CL_INVALID_OPERATION;
}

extern "C" cl_int CL_API_CALL clEnqueueBarrier(cl_command_queue command_queue) {
    static std::atomic<decltype(&clEnqueueBarrier)> cached{nullptr};
    auto fn = kokoro_cl_loader::resolve("clEnqueueBarrier", cached, &clEnqueueBarrier);
    if (fn) return fn(command_queue);
    return CL_INVALID_OPERATION;
}

extern "C" cl_int CL_API_CALL clUnloadCompiler() {
    static std::atomic<decltype(&clUnloadCompiler)> cached{nullptr};
    auto fn = kokoro_cl_loader::resolve("clUnloadCompiler", cached, &clUnloadCompiler);
    if (fn) return fn();
    return CL_INVALID_OPERATION;
}

extern "C" void * CL_API_CALL clGetExtensionFunctionAddress(const char * func_name) {
    static std::atomic<decltype(&clGetExtensionFunctionAddress)> cached{nullptr};
    auto fn = kokoro_cl_loader::resolve("clGetExtensionFunctionAddress", cached, &clGetExtensionFunctionAddress);
    if (fn) return fn(func_name);
    return nullptr;
}

extern "C" cl_command_queue CL_API_CALL clCreateCommandQueue(cl_context                     context, cl_device_id                   device, cl_command_queue_properties    properties, cl_int *                       errcode_ret) {
    static std::atomic<decltype(&clCreateCommandQueue)> cached{nullptr};
    auto fn = kokoro_cl_loader::resolve("clCreateCommandQueue", cached, &clCreateCommandQueue);
    if (fn) return fn(context, device, properties, errcode_ret);
    if (errcode_ret) *errcode_ret = CL_INVALID_OPERATION;
    return nullptr;
}

extern "C" cl_sampler CL_API_CALL clCreateSampler(cl_context          context, cl_bool             normalized_coords, cl_addressing_mode  addressing_mode, cl_filter_mode      filter_mode, cl_int *            errcode_ret) {
    static std::atomic<decltype(&clCreateSampler)> cached{nullptr};
    auto fn = kokoro_cl_loader::resolve("clCreateSampler", cached, &clCreateSampler);
    if (fn) return fn(context, normalized_coords, addressing_mode, filter_mode, errcode_ret);
    if (errcode_ret) *errcode_ret = CL_INVALID_OPERATION;
    return nullptr;
}

extern "C" cl_int CL_API_CALL clEnqueueTask(cl_command_queue  command_queue, cl_kernel         kernel, cl_uint           num_events_in_wait_list, const cl_event *  event_wait_list, cl_event *        event) {
    static std::atomic<decltype(&clEnqueueTask)> cached{nullptr};
    auto fn = kokoro_cl_loader::resolve("clEnqueueTask", cached, &clEnqueueTask);
    if (fn) return fn(command_queue, kernel, num_events_in_wait_list, event_wait_list, event);
    return CL_INVALID_OPERATION;
}

