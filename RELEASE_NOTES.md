# 0.6.23 — Background reading setup and production cleanup

- Checks Android's per-app battery-optimization exemption after setup and when the activity resumes. Shows a one-time explanation to non-exempt users, followed by the system confirmation on their explicit action. Declining never blocks TTS or creates a prompt loop.
- Adds a background-reading card to the experience/status screens. Already-exempt users see confirmation; returning from Settings refreshes the actual state. If the direct dialog is unavailable, falls back to the optimization list, then application settings.
- Adds REQUEST_IGNORE_BATTERY_OPTIMIZATIONS for the confirmed core-function failure. No WAKE_LOCK or foreground-service permissions are added. No system-wide settings change.
- Retains the adaptive deadline and single pre-audio retry as safeguards for real failures; preserves cooperative cancellation, memory reclamation, process retirement and low GPU priority.
- Removes the unused normal-priority option from Android session control. Routine per-request, worker and native memory reports are debug-only. Warnings/errors remain available in release.
- Replaces stale app version strings with BuildConfig.VERSION_NAME; native build marker is 0.6.23.

Validation:
- All 15 Kotlin source files compile against Android API 36 and AndroidX/Material API jars, with stand-ins only for Gradle-generated R and BuildConfig. The project still targets its existing compileSdk/targetSdk 37.
- 30 host regression tests pass (worker lifetime, cancellation, protocol, text splitting and adaptive watchdog).
- Changed native main, weights and generator translation units pass host C++17 fp16 syntax checks.
- Manifest and new strings parse; permission is limited to the per-app exemption request.
- No full Gradle/resource-link/NDK APK build, device UI test or signed release build was available here. Confirm allow/decline/status refresh on the device before publication.

---

# 0.6.22 — Adaptive synthesis watchdog

- Replaces stepped character deadlines with estimated audio duration (15 characters/second, adjusted for requested speech rate) multiplied by learned real-time factor.
- Deadline: 3 seconds overhead plus twice expected generation time, bounded to 10–120 seconds. Startup retains its independent timeout.
- Starts conservatively at RTF 2 until three usable samples. Uses an upper-quartile estimate over the latest nine samples, held for the service lifetime.
- Measures time to generated PCM, subtracting Android audio callback time; includes small phonemization/transport costs. No native profiling is enabled for release builds.
- Learns only after successful uncancelled chunks with no recovery in the request. Ignores sub-second audio, invalid samples, deadline-length samples and extreme slowdown outliers.
- Debug builds report the estimated RTF and chosen deadline.
- Preserves low GPU priority, existing worker retirement/retry policy and text splitting.

Validation: five adaptive-watchdog tests pass; modified service compiles against local Android test doubles. Full Android/NDK build and on-device background recovery remain unverified. This change bounds detection; it does not guarantee execution of recovery under device restrictions.

---

# 0.6.21 (versionCode 38) — native worker retirement recovery

Based on 0.6.20. This fixes two concrete recovery defects; it does not claim
that the original Samsung/long-utterance stall has been reproduced or solved.
The fixes apply to the shared engine session, including synthesizeToFile().

## Fixes

- Observe native process exit independently of Process.destroy(). That method
  can close Java streams and block behind pending pipe I/O. The old single
  retirement thread could then fail to reach either its exit check or forced
  termination. Normal termination, exit observation and forced termination
  now have independent paths. A replacement still requires the old process
  to have exited; heavy workers are never deliberately overlapped.
- Recheck actual process liveness before rejecting a replacement because of a
  previous retirement failure. Previously, an exceptionally completed retirement
  future stayed attached to the worker. Even a later OS-confirmed exit could
  leave all subsequent requests failing in the same session.
- Debug builds mark chunk lengths, Android audio callback entry/return and
  successful done() entry/return. These markers contain no utterance text and
  help distinguish native inference stalls from file/output callback stalls.

The existing scratch limits, sentence splitting, low GPU priority, no-audio
retry policy, voice IDs, speech-rate handling and wake-lock policy are unchanged.
No foreground service or additional permission has been added.

## Reported text

The supplied 421-character Alice/Rabbit sentence is already split by the
existing system-engine text path into 297 and 123 characters, after
"curiosity,". It is not passed to one unbounded inference call. A regression
test preserves the full input and that clause boundary. Native duration/token
limits can split a command further. This test does not execute Kokoro inference
or establish that the generated audio is correct on the affected device.

## Validation

- Three recovery regressions fail against 0.6.20: blocked destroy after exit,
  blocked normal destroy preventing force-kill escalation, and a failed
  retirement poisoning later startup after the old worker eventually exits.
- All 25 session, protocol and text tests pass against 0.6.21 on the host JVM.
  Android logging/context/clock are replaced with host test doubles; native
  workers are simulated. The tests include the no-overlap safety check.
- Modified session/service Kotlin sources compile with local Android API test
  doubles. This is a syntax/type check, not Android SDK or APK validation.
- Full Gradle/NDK/APK build and Samsung device testing have not been performed
  here. Native inference code is unchanged apart from the build identifier.

## Device check

Install the new build and restart the engine service once to clear the old
in-memory session. Confirm `Service created (0.6.21, worker retirement recovery)`
in Logcat. Run the same synthesizeToFile sequence, voice and rate, including the
reported sentence and the following two sentences. Keep the client configuration
fixed during this comparison. A debug APK can be launched without attaching a
debugger if diagnostics are needed.

If it stalls again, the final `KOKORO_CHUNK`, `KOKORO_STAGE`, `KOKORO_CALLBACK`
and `KOKORO_DEADLINE`/retirement lines identify the last reached phase. Audio
callback return does not prove client playback or receipt of its onDone callback.
An engine whose entire process is suspended, or whose driver cannot terminate
its native worker, cannot guarantee recovery through these Java threads.
