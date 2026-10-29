# 0.6.32 — Samsung OpenCL provider selection

- Try canonical absolute Android OpenCL paths before generic sonames, matching
  the discovery order used by OpenCL capability tools that work on affected
  Samsung firmware. Open candidates with lazy, local symbol binding.
- Keep rejected vendor handles loaded for the worker lifetime and skip absolute
  aliases that resolve to the same file. This avoids unloading and reopening a
  driver after unsuccessful platform enumeration.
- Pin both the Android plugin and the custom native staging build to NDK
  **30.0.16248370**. A changed NDK invalidates the staging task and forces a
  clean native CMake configuration.
- Include the Android version code in copied device diagnostics so tester
  reports distinguish builds that share the same version name.

This is source version **0.6.32 (50)**. Model files, voice files, protocol 3,
default-voice behavior and synthesis behavior are unchanged.

Validation: all ten mock OpenCL provider scenarios and the native device-limit
checks pass, including absolute-path precedence, lazy/local loading, wrapper
reentry, rejected-provider isolation and fallback. A physical-device result is
still required to confirm the Samsung firmware fix.

# 0.6.31 — Persistent engine default voice

- Add a dedicated Material voice-settings activity, advertised through the
  Android TTS engine metadata and linked from SC Kokoro's ready screen. Group
  all installed voices into US and UK English sections.
- Persist the engine default with Preferences DataStore. Migrate the existing
  onboarding completion, tested renderer and tested generation values from the
  legacy `onboarding` SharedPreferences file.
- Return the saved voice from `onGetDefaultVoiceNameFor()` for supported
  English locales and use it when a synthesis request does not contain a valid
  explicit voice. Valid client-selected voices continue to take priority.
- Cache the selected ID inside the running TTS service and update it from the
  DataStore flow so synchronous Android voice callbacks do not perform storage
  reads. A one-time service startup read establishes the correct value after a
  process restart.

This is source version **0.6.31 (48)**. The Android 16 OpenCL fallback and the
optional alternative-engine recommendation from 0.6.30 are retained.

Validation: the changed production Kotlin sources compile against Android and
AndroidX API stand-ins; all 41 existing JVM tests pass in isolated test-class
runs and the three new default-voice policy tests pass. The nine mock OpenCL
loader scenarios and device-limit checks pass. A complete Gradle build and
physical Android voice-picker test remain required in Android Studio because
this environment cannot download the Gradle 9.8 distribution or DataStore AAR.

# 0.6.30 — Android 16 system OpenCL fallback

- Try Android 16's direct `libOpenCL_system.so` provider, including system and
  system-ext paths, when the generic OpenCL ICD loads but reports no platforms.
  Declare that provider as an optional native library in the Android manifest.
- Add a regression scenario in which `libOpenCL.so` returns
  `CL_PLATFORM_NOT_FOUND_KHR` and `libOpenCL_system.so` provides the working
  platform, while retaining candidate validation and thread-safe dispatch.
- After the actual Kokoro speech test fails, retain retry and diagnostics and
  offer an optional Google Play link to ToBe SAID as a less demanding offline
  TTS alternative. Download and network failures do not show that suggestion.

This is source version **0.6.30 (47)**. Existing models, voices, hashes,
protocol 3, pitch processing, watchdogs, cancellation, and all 0.6.29 OpenCL
routes are preserved. The host checks validate fallback behavior; final support
still requires synthesis testing on each vendor/Android driver combination.

Validation: production Kotlin compilation against Android/AndroidX jars, all
41 JVM tests, full host native compilation and linking, nine mock OpenCL loader
scenarios, and kernel resource-limit tests. APK/AAB packaging and physical
Galaxy S24 synthesis could not be performed in this environment.

# 0.6.29 — OpenCL compatibility and installation

- Synchronize the vendored Kokoro runtime through adreno-llms commit
  `8129c946db14d33adee935aecf48fc6076c990fc` (2026-10-08), the upstream HEAD
  checked for this release. Add PowerVR Rogue work sizing and avoid cached
  decoder kernels that exceed local-memory or work-group limits. Retain our
  existing unsupported-int8-kernel cache and all local inference safeguards.
- Add Pixel OpenCL library discovery and wrapper initialization, plain Mali
  and PowerVR library names, and matching optional Android manifest entries.
  Reject unusable library candidates before committing to a provider. Keep
  successful symbol lookups cached without a lock in the normal dispatch path.
- Associate contexts explicitly with their platform. Report driver discovery,
  context, queue and FP16 test errors with bounded diagnostics.
- Install the model regardless of preliminary GPU-probe results, then attempt
  actual speech through the shared Android TTS service on every GPU. Failed
  tests offer retry and copyable diagnostics, preserve installed files, and
  leave voices available to clients. No claim of Pixel or Mali compatibility
  is made solely from a successful lightweight probe.
- Ask before using a metered connection: Download or Not now. Pause for consent
  if an unapproved download switches to a metered network. Keep partial files
  for retry; show a specific offline message when no internet network exists.
- Display download sizes and required free storage in decimal MB (1,000,000
  bytes), with matching calculations. Update the README download estimate.

This is source version **0.6.29 (46)**. Native model weights, pinned file hashes,
low GPU priority, startup/inference watchdogs, cancellation, British voice
selection and pitch processing are preserved. No CPU inference fallback,
engine wake lock or foreground service is introduced.

Validation: production Kotlin compilation against Android/AndroidX jars with
generated-resource stand-ins, all 41 JVM tests, full host native compilation
and linking, eight mock OpenCL loader scenarios and kernel resource-limit tests.
The checks exercise library fallback, wrapper reentry, rejected-provider
function-cache isolation, concurrent dispatch, metered consent and bounded
probe output. Android APK/AAB packaging and physical-device synthesis could
not be tested in this environment. See
[device acceptance checks](docs/COMPATIBILITY_TESTING.md) before publishing.

# 0.6.28 — Bound native startup and trace British voice initialization

- Apply a 30-second startup deadline before setup begins, covering voice-file
  metadata, previous-worker retirement, process creation and native readiness.
  Previously the 120-second coroutine timeout covered only native readiness;
  the adaptive inference watchdog did not run during startup.
- Run blocking startup on a dedicated thread. Timeout and Android onStop can
  release the synthesis caller without waiting for process creation to return.
  Track and retire a process that arrives after cancellation or timeout, and
  refuse overlapping launches until the earlier startup/worker has retired.
- Add debug-only startup markers for OpenCL, weights, voice-pack loading and
  eSpeak initialization, actual voice selection and vocabulary loading.
- Preserve low GPU priority, neutral-pitch bypass and existing inference
  recovery. No engine wake lock or foreground service is added.

Validation: all 27 JVM tests pass, including startup timeout, cancellation,
late-process retirement, recovery and stale-timer isolation. Production Kotlin
compiles against Android/AndroidX stand-ins. Changed native files compile and
link into the full host executable. The pinned eSpeak NG 1.52.0 library and
checksum-verified English data pass US and British integration checks; the
British voice identifier is verified as `gmw/en`.

This fixes a confirmed timeout-coverage gap, not a proven cause of the reported
British-voice stall. A complete Android APK build and device test remain needed.
See [startup testing](docs/STARTUP_TESTING.md).

# 0.6.27 — Formant-preserving pitch trial

- Honor Android synthesis pitch requests, clamped to 0.76–1.24. Ratios are
  literal, without the proposed optional softened mapping.
- Inclusive 0.98–1.02 neutral band skips pitch processing completely. Invalid
  or nonpositive ratios use 1.0. Default PCM is untouched.
- Use Rubber Band 4.0.0 R3 offline, formant-preserving processing in the native
  worker. Output sample rate and duration are unchanged; speech rate remains
  controlled by Kokoro as before. No automatic punctuation pitch contours.
- Check cancellation between analysis, processing and output blocks. Pitch
  changes are per request and do not restart the worker. The watchdog includes
  processing time, and failures propagate through the existing error path.
- Advance the worker protocol to v3 to carry pitch safely and reject stale
  v2 binaries. Include the pinned dependency source and license notices.
- Retain 0.6.26 punctuation fixes, theme changes and Android 11 minimum.

See docs/PITCH_TESTING.md for device listening checks and validation limits.

# 0.6.26 — Restore punctuation for native Kokoro intonation

- Restore sentence/clause-ending punctuation omitted by eSpeak NG's IPA API:
  periods, questions, exclamations, commas, semicolons, colons, ellipses and
  clause-ending dashes. Feed these as existing Kokoro vocabulary tokens.
- Account for eSpeak NG 1.52's UTF-8 lookahead so punctuation is attached to
  the correct clause. Keep its original input and word pronunciation intact.
- Avoid introducing sentence-ending periods after common abbreviations and
  initials within a sentence; retain decimal, number and time handling.
- Correct British eSpeak voice lookup: its downloaded name is `en`, while
  its language tag is `en-gb`. Android voice names remain unchanged.
- No added pitch post-processing, artificial pitch contours or audio chunking.
- Include Speech Central's exact Vibrant light/dark colors and an optional
  high-contrast theme overlay. Automatic contrast selection is not enabled.
- Minimum Android version is now Android 11 (API 30); ARM64 remains required.

Validation: host C++ integration tests use the pinned eSpeak NG 1.52.0 library,
checksum-verified English download files and the shipped Kokoro vocabulary.
See docs/PUNCTUATION_TESTING.md for listening checks and validation limits.

# 0.6.25 — Download retry feedback

- Every download attempt publishes an immediate preparing state with a spinner.
- Immediate failures retain visible attempt feedback for at least 700 ms before showing the result.
- Retry remains single-flight and is available again when the failure is displayed.
- Offline, unreachable-host and timeout failures receive actionable connection messages.
- Adds ACCESS_NETWORK_STATE (no runtime permission prompt) to distinguish no active network.
- Existing file validation, resumable downloads and synthesis behavior remain unchanged.

Validation: Kotlin compilation against Android API 36 and AndroidX/Material dependencies with resource-ID/BuildConfig stand-ins; XML parsing. Full Gradle/NDK APK build and on-device network testing were not available.
