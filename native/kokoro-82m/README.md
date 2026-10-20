# Kokoro-82M native runtime

This directory contains the Kokoro-82M C++/OpenCL runtime adapted from
`a8nova/adreno-llms` revision
`8129c946db14d33adee935aecf48fc6076c990fc` for the standalone Android system
TTS engine.

The 2026-10-08 upstream change adds PowerVR Rogue tuning and local-memory
checks before choosing cached decoder kernels. This integration also checks
work-group limits. Our existing context-scoped cache and capability check for
unsupported Qualcomm int8 kernels already cover the other upstream change;
they are retained along with the local streaming, cancellation and memory
management changes. Model files and their pinned hashes are unchanged.

The app uses only the persistent `--serve-stream` mode:

- stdin: `SAY <id> <rate> <pitch> <UTF-8 text>`, `CANCEL <id>`, or `QUIT`
- IDs are positive and strictly increase within one process
- stderr: `ready. protocol=3 ...`, `KOKORO_PCM_BEGIN <id> <samples> 24000`,
  and `KOKORO_UTT_END <id> OK|CANCELLED|ERROR`
- stdout: raw little-endian signed 16-bit mono PCM at 24 kHz

`CMakeLists.txt` fetches pinned eSpeak NG source and CLBlast during the native
build. Model weights, the English phonemizer data, and all 28 voice packs are
not included in the APK; the Android model manager downloads and validates them
on first launch.

Build with:

```bash
export ANDROID_NDK=/path/to/android-sdk/ndk/<version>
./scripts/build.sh --release
```

Then run `../../android/scripts/prepare_assets.sh` to stage the executable,
CLBlast, the portable OpenCL loader, kernels, and phoneme vocabulary for Gradle
packaging. No connected phone or vendor library is needed at build time.

See the project-level `NOTICE`, `LICENSE`, and `LICENSES/` files for attribution
and license terms.
