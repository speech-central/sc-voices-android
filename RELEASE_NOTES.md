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
