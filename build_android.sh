#!/usr/bin/env bash
# SPDX-License-Identifier: GPL-3.0-or-later
set -euo pipefail

PROJECT_DIR="$(cd "$(dirname "$0")" && pwd)"
export KOKORO_NDK_VERSION="30.0.16248370"

"$PROJECT_DIR/scripts/setup_deps.sh"
"$PROJECT_DIR/native/kokoro-82m/scripts/build.sh" --release
"$PROJECT_DIR/android/scripts/prepare_assets.sh"

cd "$PROJECT_DIR/android"
./gradlew :app:assembleDebug

echo "APK: $PROJECT_DIR/android/app/build/outputs/apk/debug/app-debug.apk"
