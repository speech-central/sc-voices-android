# 0.6.6 (versionCode 23)

Voice names now use the engine's short Kokoro IDs directly (for example,
`af_heart`), rather than adding an application-specific prefix.

# 0.6.5 (versionCode 22)

Complete source replacement for the reviewed 0.6.4 (21) archive. Package remains
`com.labsii.voices`; launcher/system name is SC Kokoro; KokoroVoiceSpec is public.
All 28 voices, low GPU scheduling priority, rate control and unchanged pitch
are retained. This ZIP is source, not a signed APK or a Play-ready certification.

## Reliability fixes

- Every native command and response has an ID. Early cancellation survives
  dequeue; late cancellation cannot affect a replacement command.
- Android cancellation queues control I/O without waiting on a pipe or a
  process monitor. A 2.5-second fallback claims only the exact active worker.
  Worker termination is awaited before starting a replacement.
- A worker owns its event channel for its entire lifetime. Exits between
  startup and submission are remembered, and blocked/partial PCM reads do not
  defeat synthesis timeouts. Diagnostic messages do not fill protocol queues.
- Native cancellation releases request-local buffers and acknowledges only
  after queued GPU work completes. Live generator blocks are cancellation
  boundaries; whole-graph recording is disabled for the system engine because
  it cannot observe cancellation inside the recorded graph.
- Unused FP32 scratch buffers are absent from the default half-math path.
  Cached weights no longer borrow scratch-arena slots. Scratch retention is
  capped at the smaller of 768 MiB and one third of reported device memory.
  This is not a cap on all process memory and not a measured RAM reduction.
- Inputs exceeding 512 phoneme tokens split at word boundaries. Exceptionally
  long predicted speech (over 600 alignment frames, approximately 15 seconds)
  splits before generator allocation, including at slower requested rates.
  Ordinary sentence splitting and pitch are unchanged.
- A failed real duration predictor no longer falls back to dummy durations.
  Failed kernel launches, critical argument/transfer operations, readbacks,
  non-finite output and terminal native errors cannot masquerade as success.
- The input-control thread is joinable and has bounded, validated input.

## Installation and compatibility

- UI and voice discovery use the same per-file SHA-256 verification receipts.
  Existing files without receipts are hashed, not trusted merely by length.
- Newly promoted voices always trigger a voice-data notification. Resumed
  downloads still verify size/hash; oversized responses are rejected.
- Packaged kernels/vocabulary refresh by app-install generation. The Gradle
  staging task tracks whole output directories, including kernels and the loader.
- Builds no longer pull OpenCL from a connected phone. A neutral shared loader
  resolves vendor OpenCL on the running device without shipping vendor binaries.
- Candidate selection checks errors, prefers GPU devices, tries available FP16
  candidates, and tests a real FP16 dispatch/readback. Adreno is not required.
- Other/unknown hardware receives an experimental warning and a real, silent
  TTS-service synthesis test after installation. No extra inference process is
  used for that test. Installed voices remain available if the test is interrupted.
- Onboarding completion persists; Material UI hardware acceleration is enabled.
- Profiling queues and GPU weight-roundtrip diagnostics are off in normal use.

## Validation and remaining release checks

The source has been checked with a complete Release/fp16 host CMake build and
link against CLBlast 1.6.3 and eSpeak NG 1.52.0. All Kotlin production/test sources
are compiled against Android API 35 classes with Kotlin 2.2.0 and Material 1.13.0;
only generated resource IDs and Android logging are replaced in host testing.
The project's requested AGP/Kotlin/SDK/Gradle versions are preserved.

All 15 JVM regression tests pass. These cover normal repeated requests, warm cancellation,
cancellation before startup, stale watchdog ownership, blocked input,
truncated PCM, early worker exit, voice switching without worker overlap,
partial-audio failure, framing, file integrity and sentence splitting.
Native tests cover command parsing, early/late cancellation, buffer ownership,
and reader-thread shutdown without stdin EOF.

No Android SDK/NDK or physical GPU is available in this validation environment.
A complete Android Gradle APK/AAB build, real-driver synthesis quality,
memory/thermal behavior and device cancellation latency are not verified here.
No real-time performance claim is made for this revision.

Before shipping, run on your representative devices without a debugger:

1. Upgrade with existing models; open the app once; verify all 28 voices return.
2. Read at least 30 varied sentences at 0.5x, 1x and 2x. Listen for audio regressions.
3. Repeatedly stop buffered speech and immediately request replacement speech.
   The replacement must finish, with no overlapping native workers.
4. Change voices while speaking; switch between this engine and another engine.
5. Leave a continuous reading session running for 15 minutes; inspect memory,
   responsiveness and sustained throughput, not only the first utterance.
6. Test the new driver loader on each vendor/device family you intend to support.
7. Build/sign an AAB and verify release permissions, native 16-KiB alignment,
   your privacy-policy URL and the exact corresponding-source publication.

Run JVM tests with `cd android && ./gradlew :app:testDebugUnitTest`.
Run native control tests with `bash scripts/test_native_control.sh` after
`scripts/setup_deps.sh` (host C++17 compiler required).
# 0.6.7 (versionCode 24)

Release preparation on the user's updated AGP 9.4.1 / Gradle baseline: source
directories use the built-in Kotlin source-set API; release builds enable R8
code and resource shrinking and allow native symbols to be stripped from the
packaged libraries. Ignore rules cover generated native/runtime outputs, local SDK paths,
IDE files and signing secrets. The README documents Play upload signing. This
source ZIP deliberately excludes build products, staged libraries and machine
configuration. A signed release build and device validation are still required.
