# Compatibility and installation checks — 0.6.29 (46)

Use a release build without a debugger for the final check. Confirm the native
banner reports `build=0.6.29` and `protocol=3`. This source update improves
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
Failure leaves Retry device test and Copy diagnostics available. Check that
installed voices can still be enumerated and requested by a separate TTS app.

Test the reported Pixel 9 Pro, Snapdragon 685 and Helio G99 devices where
available, plus an already-working Adreno device. Include a PowerVR Rogue
device to verify the upstream resource fallback. A library-name match alone
is insufficient: verify non-silent speech, US/UK voice switching, rate/pitch,
cancellation, repeated requests and screen-off reading. Slow speech and an
unsupported driver are separate outcomes.

Copy diagnostics after a failure. They include the app/Android version, device,
renderer, probe exit status and a bounded driver-log tail. They are copied
locally; the app does not upload them automatically. `KOKORO_OPENCL selected=`
identifies the selected library. Native code tries another library for missing
core symbols or failed platform enumeration; it does not promise to recover
from every later vendor kernel or driver failure.

## Host regression checks

`scripts/test_opencl_loader.sh` uses mock libraries and needs a GNU/LLVM linker,
C++17 compiler, `timeout` and OpenCL headers (`OPENCL_INCLUDE_DIR`). It tests
standard/Pixel/Mali/PowerVR discovery, wrapper reentry, library fallback,
concurrent cached dispatch, and local-memory/work-group rejection. Android JVM
tests also cover metered consent and protocol markers surviving verbose probe
output. These complement, and do not replace, the device checks above.
