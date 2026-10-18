#!/usr/bin/env bash
set -euo pipefail
project_dir="$(cd "$(dirname "$0")/.." && pwd)"
test_dir="$(mktemp -d)"
trap 'rm -rf "$test_dir"' EXIT
platform_flags=()
if [[ "$(uname -s)" == Darwin ]]; then platform_flags=(-framework Accelerate); fi
"${CXX:-c++}" -std=c++17 -O2 \
    -I"$project_dir/native/kokoro-82m/src" \
    -I"$project_dir/native/third_party/rubberband" \
    "$project_dir/native/kokoro-82m/tests/pitch_test.cpp" \
    "$project_dir/native/kokoro-82m/src/pitch_shift.cpp" \
    "$project_dir/native/third_party/rubberband/single/RubberBandSingle.cpp" \
    "${platform_flags[@]}" -o "$test_dir/pitch_test"
"$test_dir/pitch_test"
