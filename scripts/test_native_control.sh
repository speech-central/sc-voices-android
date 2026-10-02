#!/usr/bin/env bash
set -euo pipefail
project_dir="$(cd "$(dirname "$0")/.." && pwd)"
include_dir="${OPENCL_INCLUDE_DIR:-${ADRENO_LLMS_CACHE:-$HOME/.cache/adreno-llms}/opencl/include}"
test_dir="$(mktemp -d)"
trap 'rm -rf "$test_dir"' EXIT
"${CXX:-c++}" -std=c++17 -pthread -DCL_TARGET_OPENCL_VERSION=120 \
    -I"$include_dir" -I"$project_dir/native/kokoro-82m/src" \
    "$project_dir/native/kokoro-82m/tests/control_test.cpp" -o "$test_dir/control_test"
"$test_dir/control_test"
