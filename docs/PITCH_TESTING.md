# Pitch listening check — 0.6.27 (44)

Build the source in Android Studio and install as usual. The native ready
banner must report `protocol=3 build=0.6.27`; an old v2 worker is rejected
rather than treating the new pitch number as text. Verified model downloads
can be reused. The vendored pitch dependency needs no separate download.

## Behavior

Android `TextToSpeech.setPitch()` reaches the service as integer hundredths
in `SynthesisRequest.pitch`. The engine converts that value to a ratio.
The internal policy/protocol supports float values without additional rounding;
precision already lost at Android's public TTS boundary cannot be recovered.

- 0.98 through 1.02, inclusive: bypass, unchanged PCM, no processor allocation.
- Other valid ratios: literal pitch shift, clamped to 0.76–1.24. No softened mapping.
- Invalid or nonpositive values: neutral 1.0.
- Sample rate remains 24 kHz. Processing preserves the generated sample count.
- Kokoro continues to control speech rate. There is no automatic rising question
  or exclamation contour and no change to phonemization in this release.

Rubber Band 4.0.0 R3 processes each existing native audio chunk in offline mode
with formant preservation and no helper threads. It attempts to retain the
voice's resonance while changing pitch. Naturalness across all voices is a
listening-test goal, not established by the automated checks.

The processor is created only for non-neutral audio and released after the
chunk. Cancellation is checked between bounded analysis/processing/drain
blocks. A cancelled result is discarded; the next request has no prior pitch
state. The existing deadline includes pitch time. It cannot abort the middle
of a library call, but the native-worker retirement fallback still applies.

## Suggested device test

Use the same text, voice, and rate to compare pitch 0.76, 0.88, 1.0, 1.12 and
1.24. Repeat with at least one US female, US male and British voice. Then
try 0.98 and 1.02: both should use neutral processing.

Examples:

- "You are coming. You are coming? You are coming!"
- "She sees three green trees beside the stream."
- "Well! Really?"
- A normal paragraph containing commas, sibilants and sustained vowels.

Test both normal rate and your usual reading rate. Check whether voice identity
remains plausible, words remain clear, endings are intact, and sentence gaps
remain acceptable. Test a pitch change and stop/seek while generation is active,
including through `synthesizeToFile()`.

The short neutral band has a small step at its boundaries; this is intentional.
There are no stepped pitch levels outside it. Extreme requests clamp instead
of producing a cartoon-like effect; the accepted endpoints still need listening.

## Timing and validation

In debug builds, `KOKORO_PITCH` logs requested/effective ratios and
`KOKORO_STAGE ... pitch_end ... elapsedMs=` reports post-processing time.
Existing inference timing excludes this extra stage; total request time and
the synthesis watchdog include it. Neutral requests have no pitch stage.
Do not infer Android performance from host timings.

Run `scripts/test_native_pitch.sh` for actual-processor tests: known-frequency
signals, pitch ratios, exact duration, untouched neutral PCM, tiny fragments,
60-second limit, silence, and cancellation in both passes. The control tests
cover the new command format; JVM tests cover decimal/locale handling and
changing pitch without restarting the warm worker.

Verified here: the complete native executable builds and links on the host;
Android Kotlin sources compile against Android/AndroidX/Material dependencies
with generated-resource stand-ins; all 23 protocol/session JVM tests pass.
Native pitch and control tests pass, including UndefinedBehaviorSanitizer on
the pitch adapter (the upstream Rubber Band object was not instrumented).
These checks do not measure naturalness or target-device performance.

A full Android APK build and listening on the target hardware are required
before deciding whether this quality/performance tradeoff should ship.
