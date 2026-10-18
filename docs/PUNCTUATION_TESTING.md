# Punctuation listening check — 0.6.26

This version restores sentence/clause-ending punctuation that eSpeak NG's IPA
API omitted before the input reached Kokoro. It uses the model's existing
punctuation tokens, without pitch post-processing or additional audio chunks.

## Install and identify the build

Build this source in Android Studio as usual. App version is 0.6.26 (43).
The native worker startup banner must also show `build: 0.6.26 protocol=2`.
If it shows an older native build, rebuild the `stageKokoroRuntime` task and
reinstall. Existing verified model and voice downloads can be reused.

This release includes the pending Android 11 minimum and Speech Central
Vibrant light/dark palette. Its high-contrast overlay is available in resources
but is not selected automatically.

## Listen using the same voice and speaking rate

Try these as separate requests, first with Heart at normal rate:

- You are coming.
- You are coming?
- You are coming!
- Really?
- Well!
- Hello, Alice; wait: really? Yes!
- She said, "Are you sure?!" Then left.
- Wait... Go!
- Dr. Smith paid $3.14 at 10:30 a.m. today.

Then try a British voice and your normal long-form reading through
`synthesizeToFile()`. Both file and playback requests use this native conversion.
The model decides how punctuation changes intonation, duration and pauses;
a question is not guaranteed to have a rising ending. Compare against 0.6.25
with identical text/voice/rate if possible. Check for improved phrasing as well
as unwanted pauses, repeated words, clipped endings or changes in latency.

## Verified here

- Reproduced identical original IPA output for the period/question/exclamation
  versions of "You are coming" with the pinned eSpeak NG 1.52.0 library.
- Confirmed distinct Kokoro punctuation tokens after the fix, with identical
  word tokens for that comparison.
- Passed host integration tests for US/British English, punctuation sequences,
  quotes, UTF-8 lookahead, ellipses, clause-ending dashes, common abbreviations,
  initials, decimal numbers, times, contractions, long input and repeated calls.
- Passed UndefinedBehaviorSanitizer checks on the changed C++ conversion
  (upstream eSpeak static library is not sanitizer-instrumented). Address/leak
  sanitizer execution was blocked by this environment’s process-inspection restrictions.
- Compiled the Android Kotlin sources with Android/AndroidX/Material libraries
  and generated-resource/BuildConfig stand-ins; checked XML and theme references.

A full Gradle/NDK APK build and audible GPU synthesis on an Android device were
not available here. This ZIP contains buildable source, not a prebuilt APK.

The change preserves endings of the clauses eSpeak actually returns. It does
not add a new text normalizer or reconstruct punctuation inside an unsplit
clause. Apostrophes, hyphens and numeric punctuation remain under eSpeak's
existing pronunciation rules. Common abbreviation dots are suppressed before
following text to avoid introducing false sentence endings.

Run `scripts/test_phonemizer_punctuation.sh` with `ESPEAK_SOURCE_DIR`,
`ESPEAK_BUILD_DIR` and `ESPEAK_DATA_PARENT` pointing to a host eSpeak NG 1.52.0
build and the same English data downloaded by the app. The test uses no model,
GPU or network connection.
