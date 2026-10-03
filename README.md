# SC Kokoro

An Android system text-to-speech engine that runs Kokoro-82M through the
OpenCL runtime from [a8nova/adreno-llms](https://github.com/a8nova/adreno-llms).
The Android `TextToSpeechService` integration is based on the architecture of
[VoxSherpa TTS](https://github.com/CodeBySonu95/VoxSherpa-TTS).

This Kokoro-first release automatically installs **28 English voices**
(20 US and 8 UK) at 24 kHz, with Heart as the default. Pocket-TTS is not wired into this release
yet; the service/model boundary is intentionally small so it can be added as a
second engine without bringing back unrelated Edgi application code.

The application and Android TTS engine use the short display name **SC Kokoro**.
Package ID: `com.labsii.voices`. Version: **0.6.10 (27)**.
See [RELEASE_NOTES.md](RELEASE_NOTES.md) for fixes and validation limits.

## What is included

- Android system `TextToSpeechService` and voice-discovery endpoints
- Automatic, resumable first-run Kokoro download with progress and retry
- A persistent native `--serve-stream` process for warm synthesis
- Framed 16-bit PCM delivery to any Android TTS client
- Native Kokoro speech-rate control from Android synthesis requests (0.5x-2.0x)
- Request-scoped cooperative cancellation with a bounded worker-retirement fallback
- Kokoro-82M C++/OpenCL source and kernels from adreno-llms
- Build-time eSpeak NG phonemizer and CLBlast integration
- Credits and license texts inside the application

No LLM, VLM, camera, chat, music-generation, MMS-TTS, or Edgi UI code is
included.

## Runtime download

On first launch the app downloads about **171 MiB** from the public,
revision-pinned `a8nova/adreno-llms-weights` repository:

- Kokoro fp16 weights and metadata
- all 28 published US and UK English voice packs
- the English eSpeak NG data pack

Partial files remain in private app storage and resume on the next launch.
Every downloaded file is checked against a SHA-256 digest pinned to the same
repository revision before it is installed. A mismatching cached file is
discarded and downloaded again. After validation, the app broadcasts that new
TTS data is available.

## Build

Requirements:

- Linux or macOS
- JDK 17
- Android SDK matching `compileSdk = 37`
- Android NDK r25 or newer
- CMake, Git, and curl (a connected device is not required to build)

The complete build is available as one command:

```bash
export ANDROID_NDK=/path/to/android-sdk/ndk/<version>
./build_android.sh
```

Its individual steps are:

```bash
export ANDROID_NDK=/path/to/android-sdk/ndk/<version>

./scripts/setup_deps.sh

cd native/kokoro-82m
./scripts/build.sh --release

cd ../../android
./scripts/prepare_assets.sh
./gradlew :app:assembleDebug
```

`prepare_assets.sh` packages the native executable as `libkokoro.so`, packages
`libclblast.so` and `libkokoro_opencl.so`, and copies the kernels and phoneme vocabulary into APK assets.
This staging is also a Gradle build task, so **Build APK** in Android Studio and
`./gradlew :app:assembleDebug` perform it automatically on a clean checkout.
The neutral OpenCL loader resolves the installed device driver at runtime.
No vendor driver is copied from a build phone or redistributed in the APK.

Install `android/app/build/outputs/apk/debug/app-debug.apk`, open **SC Kokoro**,
and leave it open until Kokoro reports ready. Then choose **SC Kokoro** in the
Android text-to-speech settings.

### Release builds and signing

The release build enables R8 code optimization and resource shrinking. Gradle
is allowed to strip native debug symbols during packaging; this matters
particularly for CLBlast. The inference executable is still packaged as an
extractable native library so Android can execute it. Debug builds keep their
usual debuggability. The generated native libraries and Kokoro runtime assets
are ignored by Git and recreated by the build.

For Google Play, use Android Studio's **Build > Generate Signed Bundle / APK**
workflow, choose **Android App Bundle**, and sign it using your private upload
key. Play App Signing handles the distributed app signing key. Signing
credentials and the keystore belong outside this repository and are never
needed for a contributor's debug build. Store the upload key securely and
retain the R8 `mapping.txt` from each release to decode stack traces. Before
publishing, inspect the signed bundle's native library sizes and install a
signed release build to verify voice discovery, model download, synthesis,
switching and cancellation on a device without a debugger.

The current native target is arm64-v8a and requires a device OpenCL driver. The
engine tries any accessible, working OpenCL FP16 device, with GPUs preferred.
Vendor names are not a compatibility gate. Missing required capabilities are
reported before downloads; an inconclusive probe may proceed to a real test.
Before downloading model or voice data, the app runs the packaged runtime's
lightweight OpenCL capability probe. A confirmed lack of required capabilities
blocks downloads; a successful fp16 compile/dispatch/readback or an inconclusive
probe permits installation. Adreno GPUs receive estimated experience ratings;
other hardware is experimental and remains unrated. After installation it gets
a short, silent synthesis test through the actual Android TTS service. A test
failure can be retried and does not hide installed voices from other apps.

Existing downloads are retained on upgrade and hashed once to migrate to
per-file verification receipts. Open the app once after updating to complete
verification and refresh packaged kernels/vocabulary. No full model download
is required if the existing files are intact.

### TTS client package visibility

Applications targeting Android 11 (API 30) or newer that enumerate installed
TTS engines must make the standard TTS service visible in their own manifest:

```xml
<queries>
    <intent>
        <action android:name="android.intent.action.TTS_SERVICE" />
    </intent>
</queries>
```

This declaration belongs to the client application. It is not required when a
client constructs `TextToSpeech` with an already known, explicit engine package.

## Source layout

```text
android/              Minimal system-engine app and Kokoro downloader
native/kokoro-82m/    Kokoro C++/OpenCL inference runtime
scripts/              OpenCL build dependency setup
```

## Licensing

The combined application is distributed under GPL-3.0-or-later because its
system-engine integration is based on GPL-licensed VoxSherpa work. Material
from adreno-llms and Kokoro retains its Apache-2.0 terms. eSpeak NG is
GPL-3.0-or-later and includes separately licensed supporting data.

See [`NOTICE`](NOTICE), [`LICENSE`](LICENSE), and [`LICENSES/`](LICENSES/).

### Branding and trademarks

The GPL-3.0-or-later license applies to the covered source code, but it does
not grant rights to the Speech Central or SC Kokoro branding. The supplied
gradient background, the launcher icon derived from it, and Labsii Ltd.'s
Speech Central names and branding are excluded from the project's open-source
license and are all rights reserved.

Forks and redistributed builds must replace these reserved assets and must not
present themselves as official Labsii Ltd. or Speech Central products. See
[`BRANDING.md`](BRANDING.md) and [`branding/README.md`](branding/README.md) for
the exact scope. Third-party names and marks remain the property of their
respective owners.
