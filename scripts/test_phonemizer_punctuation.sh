#!/usr/bin/env bash
set -euo pipefail
project_dir=$(cd -- "$(dirname -- "$0")/.." && pwd)
# Use a host build of the same pinned eSpeak NG 1.52.0 (audio/async/sonic off).
: "${ESPEAK_SOURCE_DIR:?Set to the eSpeak NG 1.52.0 source directory}"
: "${ESPEAK_BUILD_DIR:?Set to its host CMake build directory}"
: "${ESPEAK_DATA_PARENT:?Set to the directory containing the downloaded espeak-ng-data}"
test_dir=$(mktemp -d)
trap 'rm -rf -- "$test_dir"' EXIT
"${CXX:-c++}" -std=c++17 -Wall -Wextra -Werror -DNNOPT_TTS_STREAMING=1 \
  -I"$ESPEAK_SOURCE_DIR/src/include" -I"$project_dir/native/kokoro-82m/src" \
  "$project_dir/native/kokoro-82m/tests/phonemizer_punctuation_test.cpp" \
  "$project_dir/native/kokoro-82m/src/phonemizer.cpp" \
  "$ESPEAK_BUILD_DIR/src/libespeak-ng/libespeak-ng.a" \
  "$ESPEAK_BUILD_DIR/src/ucd-tools/libucd.a" \
  -lm -o "$test_dir/phonemizer_test"
for voice in en-us en-gb; do
  "$test_dir/phonemizer_test" "$ESPEAK_DATA_PARENT" \
    "$project_dir/native/kokoro-82m/assets/phoneme_vocab.tsv" "$voice"
done
