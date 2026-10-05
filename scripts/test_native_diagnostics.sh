#!/usr/bin/env bash
set -euo pipefail
project_dir=$(cd -- "$(dirname -- "$0")/.." && pwd)
include_dir=${OPENCL_INCLUDE_DIR:-${ADRENO_LLMS_CACHE:-$HOME/.cache/adreno-llms}/opencl/include}
test_dir=$(mktemp -d)
trap 'rm -rf -- "$test_dir"' EXIT
"${CXX:-c++}" -std=c++17 -DCL_TARGET_OPENCL_VERSION=120 \
  -I"$include_dir" -I"$project_dir/native/kokoro-82m/src" \
  "$project_dir/native/kokoro-82m/tests/diagnostics_test.cpp" -o "$test_dir/diagnostics_test"
NNOPT_DIAGNOSTICS=0 "$test_dir/diagnostics_test" >"$test_dir/quiet.stdout" 2>"$test_dir/quiet.stderr"
[[ ! -s "$test_dir/quiet.stdout" && ! -s "$test_dir/quiet.stderr" ]]
NNOPT_DIAGNOSTICS=1 "$test_dir/diagnostics_test" >"$test_dir/debug.stdout" 2>"$test_dir/debug.stderr"
[[ ! -s "$test_dir/debug.stdout" ]]
grep -Eq '^KOKORO_STAGE command=42 nativePid=[0-9]+ monoMs=[0-9]+ bootMs=-?[0-9]+ diagnostic_test value=7$' "$test_dir/debug.stderr"
printf '%s\n' 'Native diagnostic gating, formatting, IDs and clock fields passed.'
