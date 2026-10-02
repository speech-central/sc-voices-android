#!/bin/bash
# Build script for hexgrad/Kokoro-82M
# Cross-compiles for Android using NDK

set -e  # Exit on error

cd "$(dirname "$0")/.."

# Parse arguments. DEFAULT IS RELEASE (fastest code): debug builds add
# per-GEMM clFinish + layer checks (~1.4x slower) and must be opted into
# with --debug. Never benchmark a --debug build.
CLEAN=""
BUILD_TYPE="Release"
NNOPT_DEBUG_FLAG="-DNNOPT_DEBUG=0"

while [[ $# -gt 0 ]]; do
    case $1 in
        --clean)
            CLEAN="yes"
            shift
            ;;
        --debug)
            BUILD_TYPE="Debug"
            NNOPT_DEBUG_FLAG="-DNNOPT_DEBUG=1"
            shift
            ;;
        --release)
            BUILD_TYPE="Release"
            NNOPT_DEBUG_FLAG="-DNNOPT_DEBUG=0"
            shift
            ;;
        *)
            echo "Unknown option: $1"
            echo "Usage: $0 [--clean] [--debug] [--release]"
            exit 1
            ;;
    esac
done

# Dtype: fp16 (default) builds the optimized half-precision binary into build/fp16/;
# NNOPT_DTYPE=fp32 builds the fp32 binary into build/.
NNOPT_DTYPE="${NNOPT_DTYPE:-fp16}"  # fp16 = the optimized binary; fastest is the default
case "$NNOPT_DTYPE" in
    fp16) BUILD_DIR="build/fp16"; CMAKE_DTYPE_ARG="-DNNOPT_DTYPE=fp16" ;;
    fp32|"") BUILD_DIR="build"; CMAKE_DTYPE_ARG="-DNNOPT_DTYPE=fp32"; NNOPT_DTYPE="fp32" ;;
    *)
        echo "ERROR: NNOPT_DTYPE must be fp32 or fp16 (got '$NNOPT_DTYPE')" >&2
        exit 1
        ;;
esac
echo "Build dtype: $NNOPT_DTYPE (build dir: $BUILD_DIR)"

# If a previous configure was the opposite flavor (debug vs release OR
# different dtype), force a clean rebuild — stale CMakeCache.txt entries
# carry -DNNOPT_DEBUG and -DNNOPT_DTYPE across configures otherwise.
if [ -f $BUILD_DIR/CMakeCache.txt ]; then
    CACHED_TYPE=$(grep -E "^CMAKE_BUILD_TYPE:" $BUILD_DIR/CMakeCache.txt | head -1 | cut -d= -f2)
    if [ -n "$CACHED_TYPE" ] && [ "$CACHED_TYPE" != "$BUILD_TYPE" ]; then
        echo "Build type changed ($CACHED_TYPE → $BUILD_TYPE) — forcing clean reconfigure"
        CLEAN="yes"
    fi
    CACHED_DTYPE=$(grep -E "^NNOPT_DTYPE:" $BUILD_DIR/CMakeCache.txt | head -1 | cut -d= -f2)
    if [ -n "$CACHED_DTYPE" ] && [ "$CACHED_DTYPE" != "$NNOPT_DTYPE" ]; then
        echo "NNOPT_DTYPE changed ($CACHED_DTYPE → $NNOPT_DTYPE) — forcing clean reconfigure"
        CLEAN="yes"
    fi
fi

# ============================================
# Android NDK Setup
# ============================================
ANDROID_NDK="${ANDROID_NDK:-${ANDROID_NDK_ROOT:-${ANDROID_NDK_HOME:-}}}"

# Auto-discover NDK if still empty
if [ -z "$ANDROID_NDK" ] || [ ! -d "$ANDROID_NDK" ]; then
    for NDK_BASE in \
        "${ANDROID_SDK_ROOT:-}/ndk" \
        "${ANDROID_HOME:-}/ndk" \
        "$HOME/Library/Android/sdk/ndk" \
        "$HOME/Android/Sdk/ndk"; do
        if [ -d "$NDK_BASE" ]; then
            LATEST=$(ls -1 "$NDK_BASE" 2>/dev/null | sort -V | tail -1)
            if [ -n "$LATEST" ] && [ -d "$NDK_BASE/$LATEST" ]; then
                ANDROID_NDK="$NDK_BASE/$LATEST"
                echo "Auto-discovered NDK: $ANDROID_NDK"
                break
            fi
        fi
    done
fi

if [ -z "$ANDROID_NDK" ] || [ ! -d "$ANDROID_NDK" ]; then
    echo "ERROR: Android NDK not found at: $ANDROID_NDK"
    echo "Please set the ANDROID_NDK environment variable or install the Android NDK."
    exit 1
fi

TOOLCHAIN_FILE="$ANDROID_NDK/build/cmake/android.toolchain.cmake"
if [ ! -f "$TOOLCHAIN_FILE" ]; then
    echo "ERROR: Android toolchain file not found: $TOOLCHAIN_FILE"
    echo "Ensure you have a valid Android NDK installation"
    exit 1
fi

echo "Using Android NDK: $ANDROID_NDK"
echo "Target ABI: arm64-v8a"
echo "Target API Level: 21"

# Android Studio installs CMake under the Android SDK, but that directory is
# not normally added to the PATH of Gradle processes launched from the IDE.
# Prefer a PATH installation, then use the newest SDK-managed CMake.
CMAKE_BIN="$(command -v cmake 2>/dev/null || true)"
if [ -z "$CMAKE_BIN" ]; then
    for SDK_BASE in "${ANDROID_SDK_ROOT:-}" "${ANDROID_HOME:-}" "$HOME/Library/Android/sdk" "$HOME/Android/Sdk"; do
        if [ -d "$SDK_BASE/cmake" ]; then
            CMAKE_BIN="$(find "$SDK_BASE/cmake" -type f -path '*/bin/cmake' -perm -111 2>/dev/null | sort -V | tail -1)"
            if [ -n "$CMAKE_BIN" ]; then
                echo "Auto-discovered CMake: $CMAKE_BIN"
                break
            fi
        fi
    done
fi
if [ -z "$CMAKE_BIN" ]; then
    echo "ERROR: CMake not found. Install CMake in Android Studio's SDK Tools, or add cmake to PATH." >&2
    exit 1
fi

# ============================================
# Check OpenCL headers + libOpenCL.so link stub.
#
# Cache lives at $HOME/.cache/adreno-llms/ (override with $ADRENO_LLMS_CACHE).
# If headers are missing, scripts/setup_deps.sh is invoked automatically — it
# downloads the Khronos OpenCL headers and pulls libOpenCL.so from a connected
# Android device. CLBlast is built from source via CMake FetchContent on first
# configure.
# ============================================
ADRENO_LLMS_CACHE="${ADRENO_LLMS_CACHE:-$HOME/.cache/adreno-llms}"
OPENCL_INC="$ADRENO_LLMS_CACHE/opencl/include"
OPENCL_LIB_DIR="$ADRENO_LLMS_CACHE/opencl/lib/android-arm64-v8a"

if [ ! -f "$OPENCL_INC/CL/cl.h" ]; then
    SETUP_SH="$(pwd)/../../scripts/setup_deps.sh"
    if [ -x "$SETUP_SH" ]; then
        echo "OpenCL deps not found in $ADRENO_LLMS_CACHE — running setup_deps.sh"
        bash "$SETUP_SH"
    else
        echo "ERROR: OpenCL headers not found and setup_deps.sh missing at $SETUP_SH" >&2
        exit 1
    fi
fi

if [ ! -f "$OPENCL_INC/CL/cl.h" ]; then
    echo "ERROR: OpenCL headers still not found at $OPENCL_INC after setup" >&2
    exit 1
fi

echo "Using OpenCL headers: $OPENCL_INC"
echo "CLBlast will be built from source via CMake FetchContent"

# OpenCL is resolved by the packaged loader; no device-derived link stub.
# Clean if requested
if [ "$CLEAN" = "yes" ]; then
    echo "Cleaning $BUILD_DIR..."
    rm -rf $BUILD_DIR
fi

# Create build directory (per-dtype so fp32 and fp16 caches coexist)
mkdir -p $BUILD_DIR
PROJECT_ROOT="$(pwd)"
cd $BUILD_DIR

echo "Configuring with CMake for Android ($BUILD_TYPE, $NNOPT_DTYPE)..."
"$CMAKE_BIN" "$PROJECT_ROOT" \
    -DCMAKE_TOOLCHAIN_FILE="$TOOLCHAIN_FILE" \
    -DANDROID_ABI=arm64-v8a \
    -DANDROID_PLATFORM=android-21 \
    -DANDROID_STL=c++_static \
    -DOPENCL_INCLUDE_DIR="$OPENCL_INC" \
    -DKOKORO_PORTABLE_OPENCL=ON \
    $CMAKE_DTYPE_ARG \
    -DCMAKE_BUILD_TYPE=$BUILD_TYPE \
    $NNOPT_DEBUG_FLAG

# Build
echo "Building for Android..."
CORES=$(nproc 2>/dev/null || sysctl -n hw.ncpu 2>/dev/null || echo 4)
"$CMAKE_BIN" --build . -j$CORES

cd "$PROJECT_ROOT"

if [ "$NNOPT_DTYPE" = "fp16" ]; then
    BIN_NAME="Kokoro_82M_inference_fp16"
else
    BIN_NAME="Kokoro_82M_inference"
fi
echo ""
echo "==================================="
echo "Android build complete! ($BUILD_TYPE / $NNOPT_DTYPE)"
echo "Binary: $BUILD_DIR/$BIN_NAME"
echo "==================================="
echo ""
echo "To deploy to Android device:"
echo "  Return to the project root and run ./build_android.sh"
