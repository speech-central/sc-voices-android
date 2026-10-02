#!/usr/bin/env bash
# Derived from a8nova/adreno-llms, copyright 2026 a8nova.
# Modified in 2026 for the standalone Kokoro system TTS distribution.
# SPDX-License-Identifier: Apache-2.0
# One-time dependency setup for adreno-llms.
#
# Downloads:
#   1. Khronos OpenCL C headers (CL/cl.h, etc.) into the local cache.
# No connected device or vendor binary is required at build time.
# The APK includes a small loader, not a vendor OpenCL implementation.
#
# Cache lives at:
#   ${ADRENO_LLMS_CACHE:-$HOME/.cache/adreno-llms}/opencl/
#
# Idempotent — re-running is a no-op once the headers are present.
#
# CLBlast and eSpeak NG are downloaded and built by the Kokoro CMake project.

set -euo pipefail

CACHE="${ADRENO_LLMS_CACHE:-$HOME/.cache/adreno-llms}"
OPENCL_DIR="${CACHE}/opencl"
HEADERS_DIR="${OPENCL_DIR}/include/CL"

HEADERS_TAG="${OPENCL_HEADERS_TAG:-v2024.10.24}"
HEADERS_URL="https://github.com/KhronosGroup/OpenCL-Headers/archive/refs/tags/${HEADERS_TAG}.tar.gz"

mkdir -p "${HEADERS_DIR}"

# ── 1. OpenCL headers ───────────────────────────────────────────────────
echo ">>> OpenCL headers (Khronos ${HEADERS_TAG})"
if [ -f "${HEADERS_DIR}/cl.h" ]; then
    echo "    already present at ${HEADERS_DIR}/cl.h"
else
    echo "    downloading ${HEADERS_URL}"
    TMP=$(mktemp -d)
    trap 'rm -rf "${TMP}"' EXIT
    curl --location --fail-with-body --silent --output "${TMP}/headers.tar.gz" "${HEADERS_URL}"
    tar -xzf "${TMP}/headers.tar.gz" -C "${TMP}"
    SRC_HEADERS_DIR="${TMP}/OpenCL-Headers-${HEADERS_TAG#v}/CL"
    if [ ! -d "${SRC_HEADERS_DIR}" ]; then
        echo "    ERROR: extracted tarball doesn't contain CL/ directory at ${SRC_HEADERS_DIR}" >&2
        exit 1
    fi
    cp "${SRC_HEADERS_DIR}"/*.h "${HEADERS_DIR}/"
    echo "    installed $(ls "${HEADERS_DIR}" | wc -l | tr -d ' ') headers to ${HEADERS_DIR}"
    trap - EXIT
    rm -rf "${TMP}"
fi

echo "OpenCL headers ready. Device drivers are resolved on the device at runtime."
