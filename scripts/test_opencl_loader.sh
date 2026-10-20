#!/usr/bin/env bash
set -euo pipefail
project_dir="$(cd "$(dirname "$0")/.." && pwd)"
include_dir="${OPENCL_INCLUDE_DIR:-${ADRENO_LLMS_CACHE:-$HOME/.cache/adreno-llms}/opencl/include}"
test_dir="$(mktemp -d)"
trap 'rm -rf "$test_dir"' EXIT
# GNU/LLVM linker wrapping keeps these tests independent of the host GPU.
"${CXX:-c++}" -std=c++17 -pthread -Wno-deprecated-declarations \
  -I"$include_dir" "$project_dir/native/kokoro-82m/src/opencl_loader.cpp" \
  "$project_dir/native/kokoro-82m/tests/opencl_loader_test.cpp" \
  -Wl,--wrap=dlopen -Wl,--wrap=dlsym -Wl,--wrap=dlclose -Wl,--wrap=dlerror \
  -o "$test_dir/loader_test"
for scenario in generic pixel_direct pixel_wrapper mali_soname powervr fallback missing_symbol unavailable; do
  timeout 10 "$test_dir/loader_test" "$scenario"
done
"${CXX:-c++}" -std=c++17 -I"$include_dir" -I"$project_dir/native/kokoro-82m/src" \
  "$project_dir/native/kokoro-82m/tests/device_quirks_test.cpp" -o "$test_dir/quirks_test"
"$test_dir/quirks_test"
