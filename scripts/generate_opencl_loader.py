#!/usr/bin/env python3
"""Regenerate src/opencl_loader.cpp from Khronos cl.h. Prints source to stdout.

The checked-in result is used by normal builds; Python is not a build dependency.
Copyright 2026 Labsii Ltd. SPDX-License-Identifier: Apache-2.0
"""
import re
import sys
from pathlib import Path

header = re.sub(r"/\*.*?\*/", "", Path(sys.argv[1]).read_text(), flags=re.S)
header = re.sub(r"//[^\n]*", "", header)
functions = re.findall(
    r"extern CL_API_ENTRY\s+(.*?)\s+CL_API_CALL\s+(cl\w+)\s*\((.*?)\)\s*CL_API_SUFFIX\w+\s*;",
    header, re.S,
)
assert len(functions) > 100, "Unexpected Khronos header format"
print('''// Generated from Khronos OpenCL-Headers v2024.10.24 by generate_opencl_loader.py.
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
''')
for result, name, parameters in functions:
    result = re.sub(r"CL_API_PREFIX\w+\s*", "", result).strip()
    params = []
    depth = 0
    start = 0
    for i, character in enumerate(parameters):
        if character == "(": depth += 1
        if character == ")": depth -= 1
        if character == "," and depth == 0:
            params.append(parameters[start:i].strip()); start = i + 1
    params.append(parameters[start:].strip())
    if params == ["void"] or params == [""]: params = []
    arguments = []
    for param in params:
        callback = re.search(r"\(CL_CALLBACK\s*\*\s*(\w+)\)", param)
        if callback:
            arguments.append(callback.group(1))
        else:
            plain = re.sub(r"\[[^]]*\]", "", param)
            arguments.append(re.search(r"(\w+)\s*$", plain).group(1))
    print(f'extern "C" {result} CL_API_CALL {name}({", ".join(params)}) {{')
    print(f'    static std::atomic<decltype(&{name})> cached{{nullptr}};')
    print(f'    auto fn = kokoro_cl_loader::resolve("{name}", cached, &{name});')
    call = f'fn({", ".join(arguments)})'
    if result == "void":
        print(f'    if (fn) {call};')
    else:
        print(f'    if (fn) return {call};')
        if "errcode_ret" in arguments: print('    if (errcode_ret) *errcode_ret = CL_INVALID_OPERATION;')
        if name == "clGetPlatformIDs": print('    if (num_platforms) *num_platforms = 0;')
        if name == "clGetDeviceIDs": print('    if (num_devices) *num_devices = 0;')
        fallback = 'CL_INVALID_OPERATION' if result == 'cl_int' else 'nullptr'
        print(f'    return {fallback};')
    print('}\n')
