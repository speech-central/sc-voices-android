#!/usr/bin/env bash
# Derived from a8nova/adreno-llms, copyright 2026 a8nova.
# Modified in 2026 for the standalone Kokoro system TTS application.
# SPDX-License-Identifier: Apache-2.0
set -euo pipefail

ANDROID_DIR="$(cd "$(dirname "$0")/.." && pwd)"
PROJECT_DIR="$(cd "$ANDROID_DIR/.." && pwd)"
MODEL_DIR="$PROJECT_DIR/native/kokoro-82m"
BUILD_DIR="$MODEL_DIR/build/fp16"
JNI_DIR="$ANDROID_DIR/app/src/main/jniLibs/arm64-v8a"
ASSET_DIR="$ANDROID_DIR/app/src/main/assets/kokoro"
BIN="$BUILD_DIR/Kokoro_82M_inference_fp16"

fail() { echo "prepare_assets: $*" >&2; exit 1; }

[[ -f "$BIN" ]] || fail "missing $BIN; run native/kokoro-82m/scripts/build.sh first"
[[ -f "$BUILD_DIR/libkokoro_opencl.so" ]] || fail "portable OpenCL loader missing; rebuild the native runtime"

CLBLAST=""
for candidate in \
    "$BUILD_DIR/_deps/clblast-build/libclblast.so" \
    "$BUILD_DIR/libclblast.so" \
    "$BUILD_DIR/lib/libclblast.so"; do
    if [[ -f "$candidate" ]]; then
        CLBLAST="$candidate"
        break
    fi
done
[[ -n "$CLBLAST" ]] || fail "libclblast.so was not found under $BUILD_DIR"

rm -rf "$ANDROID_DIR/app/src/main/jniLibs" "$ASSET_DIR"
mkdir -p "$JNI_DIR" "$ASSET_DIR/kernels" "$ASSET_DIR/assets"

# Android extracts native libraries to an executable directory. The inference
# executable is intentionally packaged with a .so name for this purpose.
install -m 0755 "$BIN" "$JNI_DIR/libkokoro.so"
install -m 0755 "$CLBLAST" "$JNI_DIR/libclblast.so"
install -m 0755 "$BUILD_DIR/libkokoro_opencl.so" "$JNI_DIR/libkokoro_opencl.so"
cp "$MODEL_DIR/kernels/"*.cl "$ASSET_DIR/kernels/"
cp "$MODEL_DIR/assets/phoneme_vocab.tsv" "$ASSET_DIR/assets/phoneme_vocab.tsv"

echo "Kokoro runtime staged. Model weights and English voice data will download automatically in the app."
echo "Build with: cd $ANDROID_DIR && ./gradlew :app:assembleDebug"
