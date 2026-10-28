# Startup and British-voice checks — 0.6.31 (48)

This build fixes an identified startup timeout gap and adds diagnostics. The
cause of the device-specific British-voice stall is not yet established.

## Device check

1. Build and install the updated source. Open SC Kokoro once to refresh packaged
   runtime assets; verified model downloads can be reused.
2. Use a debug build, preferably without attaching the debugger. Confirm the
   native banner contains `build=0.6.31` and `protocol=3`.
3. Send the same short sentence first to Heart (`af_heart`), then to Alice
   (`bf_alice`), with rate 1.0 and pitch 1.0. For example: "This is a short test."
4. If neutral synthesis works, repeat Alice with pitch 1.12, then switch back
   to Heart. Exercise stop/seek while startup or synthesis is in progress.
5. If a request fails, keep the log from its request entry through the error or
   cancellation and the next request. Record any client timeout used.

## Reading the log

`KOKORO_STARTUP` identifies Java setup stages and native begin/end pairs:
OpenCL, weight loading/preallocation, model initialization, voice-pack loading,
phonemizer initialization, eSpeak initialization, voice selection and vocabulary.
The last unmatched begin marker narrows the blocking call. British selection
should log `espeak_voice_end gmw/en`; US selection should log `gmw/en-US`.
These records are enabled by the debug build's existing diagnostics flag.

- `KOKORO_STARTUP_DEADLINE`: startup exceeded its separate 30-second budget.
- `KOKORO_DEADLINE`: the existing synthesis deadline expired after startup.
- Android `onStop`: a cancellation request, which can originate in a client
  watchdog; it is not by itself evidence that the engine deadline fired.
- `phonemize_end` followed by `graph_end rc=0`: phonemization and native model
  inference completed for that command. A subsequent stall is later in the path.

The 0.6.27 capture contained one Alice request that passed phonemization and
model inference, then entered pitch processing before Android onStop. Earlier
requests were cancelled during startup without detailed initialization markers.
That capture alone does not identify the cause of a neutral-pitch stall.

## Recovery limits

The startup deadline is scheduled independently of the startup worker and
shared coroutine IO pool. It reports failure and starts retirement if a process
exists. Cancellation similarly releases the caller without awaiting a blocked
launch. A process returned after expiry is tracked and retired, not reused.

This cannot force a blocked OS process-creation or process-exit call to return,
or make Android schedule a suspended application. Until an earlier launch or
worker is confirmed finished, a replacement fails promptly instead of spawning
competing native workers. No wake lock, foreground service or GPU priority
increase is introduced. The existing inference watchdog remains separate.

## Validation performed

- Production Kotlin compilation against Android/AndroidX dependencies with
  generated-resource and BuildConfig stand-ins; not a full Gradle APK build.
- 27 passing JVM tests, including blocked process creation, readiness timeout,
  cancellation, late-worker cleanup, replacement and stale deadline isolation.
- Changed native sources compile and link into the complete host executable.
- Pinned eSpeak NG 1.52.0 with the checksum-verified English data passes the
  US/GB integration test, including actual selected identifiers and punctuation.

Host tests do not establish Android driver behavior, end-to-end GB voice
reliability, device timing or pitch naturalness. Those need device testing.
