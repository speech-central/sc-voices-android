# Compatibility and installation checks — 0.6.31 (48)

Use a release build without a debugger for the final check. Confirm the native
banner reports `build=0.6.31` and `protocol=3`. This source update improves
discovery and fallbacks; host tests do not establish support for a GPU/driver.

## Installation

1. On a fresh install with unmetered Wi-Fi, downloads begin without a GPU gate.
   Progress uses decimal MB, a separate percentage and a multiline file label.
2. On mobile data or metered Wi-Fi, Download / Not now appears before transfer.
   Not now leaves an actionable waiting screen. Reopen on unmetered Wi-Fi to
   resume, or tap Download and approve. No model data should download before
   approval on that connection.
3. Switch an unapproved Wi-Fi download to metered data. It pauses on the next
   network check (checks occur between reads, at most every 500 ms while reads
   progress). Some already-in-flight data can arrive during a network change.
   Approval resumes the attempt; partial files remain hash checked on completion.
4. With no connection, the app explains that internet is needed. After connecting,
   Retry visibly starts preparation. Invalid hashes and server errors still show
   an actionable failure. Retry must not create concurrent download workers.
5. Updating an existing installation reuses verified models and refreshes the
   bundled runtime. Completed local files need no mobile-data approval.

## Actual speech

After files are installed, every GPU receives a silent file-synthesis test at
rate 1.0 and pitch 1.0. A failed preliminary probe must still allow that test.
Successful tests are cached per app installation/update and Android build.
Failure leaves Retry device test, Copy diagnostics and the optional ToBe SAID
recommendation available. The recommendation must appear only after synthesis
testing fails, never for download/network errors. Check that installed voices
can still be enumerated and requested by a separate TTS app.

Test the reported Pixel 9 Pro, Snapdragon 685 and Helio G99 devices where
available, plus an already-working Adreno device. Include a PowerVR Rogue
device to verify the upstream resource fallback. A library-name match alone
is insufficient: verify non-silent speech, US/UK voice switching, rate/pitch,
cancellation, repeated requests and screen-off reading. Slow speech and an
unsupported driver are separate outcomes.

On a Galaxy S24 Exynos running Android 16, confirm that a generic
`libOpenCL.so` result with `CL_PLATFORM_NOT_FOUND_KHR` falls through to
`libOpenCL_system.so` (or its system path), logs that provider as selected and
then completes real synthesis on the Xclipse 940. Library selection alone is
not sufficient acceptance.

Copy diagnostics after a failure. They include the app/Android version, device,
renderer, probe exit status and a bounded driver-log tail. They are copied
locally; the app does not upload them automatically. `KOKORO_OPENCL selected=`
identifies the selected library. Native code tries another library for missing
core symbols or failed platform enumeration; it does not promise to recover
from every later vendor kernel or driver failure.

## Default voice and settings migration

1. Upgrade an installation that has completed onboarding and a cached device
   test. Confirm the ready screen remains available without repeating onboarding
   or downloading intact model files.
2. Open **Default voice** from the ready screen and from the settings button for
   this engine in Android's text-to-speech settings. Both routes must display all
   installed US and UK voices.
3. Select a UK voice, restart the engine process, and synthesize from a client
   that supplies only an English language. The saved UK voice must persist.
4. Have a client explicitly request a different installed voice. Its explicit
   choice must override the engine default. An invalid client voice must fall
   back to the saved engine default.

## Host regression checks

`scripts/test_opencl_loader.sh` uses mock libraries and needs a GNU/LLVM linker,
C++17 compiler, `timeout` and OpenCL headers (`OPENCL_INCLUDE_DIR`). It tests
standard/Android-16-system/Pixel/Mali/PowerVR discovery, wrapper reentry,
library fallback,
concurrent cached dispatch, and local-memory/work-group rejection. Android JVM
tests also cover metered consent and protocol markers surviving verbose probe
output. These complement, and do not replace, the device checks above.
