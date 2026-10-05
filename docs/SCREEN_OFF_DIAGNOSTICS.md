> Historical developer troubleshooting guide. Version 0.6.23 adds a user-approved
> per-app battery-optimization exemption after this resolved screen-off stalls in
> device testing. Normal users should use the app's background-reading button;
> the capture procedure below is only for failures that persist afterward.
> Record the exemption state when reporting a new failure. Current native marker:
> `ready. protocol=2 build=0.6.23`.

# Stall capture guide (updated for 0.6.21)

Version 0.6.21 fixes worker retirement; see the release notes for its tests and
limits. This guide can capture any remaining stall. Normal synthesis and the
single no-audio recovery attempt still use low GPU priority. It adds no
wake lock, foreground service, permission, polling heartbeat, extra `clFinish`,
or model/chunking change. The existing retry limit and memory policy remain.

## What this test adds

Debug builds identify input uploads, allocations, BERT, duration encoding,
duration readback, alignment/F0N, text encoding, decoder, PCM output and cleanup.
Every native stage now includes the command ID, native PID and timestamps taken
**in the native process**. Java records when it receives that line. A delayed
stderr reader can no longer make an old native event look like recent progress.
The deadline log records its due time and lateness. Pipe writes/reads and worker
retirement have entry/return markers, without logging utterance text or PCM.

The independent system trace records thread scheduling, available wait and
freeze events, Binder activity and power transitions. It does not depend on the
TTS app's logging thread continuing to run. The app's own logs remain useful:
logs actually emitted during a stall prove execution of those particular
threads at those times; they do not prove progress on the inference thread.
Silence in Logcat alone does **not** prove a freeze. A running device uptime
alone does **not** prove that every app thread was running.

## Run on the affected device

Use the **debug** variant of 0.6.21, but launch normally: **do not attach a
debugger**. Confirm `Service created (0.6.21, ...)` and `KOKORO_NATIVE_READY ...
ready. protocol=2 build=0.6.21` in logs. The latter comes from the native binary
itself, so it also detects a stale staged runtime. Keep the same controller,
text, rate and existing power
settings. Do not add a wake lock, raise priority or change battery exemptions
for this run. Start with only one reader using the engine.

The capture script runs on your Mac/Linux terminal from the project folder.
It requires `adb` (Android SDK platform-tools) and `zip`. If `adb` is not on
PATH, use `export ADB="$HOME/Library/Android/sdk/platform-tools/adb"` on a Mac
with the default SDK location. Use `ANDROID_SERIAL` when multiple devices are
connected. No root is required. If the device rejects capture permissions,
stop and report the startup output; do not change global device settings.

1. Connect USB, authorize adb, and run:

   ```bash
   bash scripts/capture_stall.sh start
   ```

2. The script prints a collection command. **Unplug USB**, start/resume the
   same reading test, and turn the screen off. Both recorders continue on the
   device without USB. They stop automatically after **10 minutes**; the trace
   retains recent history in a roughly 26 MiB ring, and filtered logs rotate
   at roughly 8 MiB. No periodic disk flush is requested for the trace, though
   recording still has overhead and can influence scheduling.

3. When the unwanted pause occurs, note the approximate time. Unless you need
   to stop earlier, give it about 90 seconds to capture the deadline/retry.
   Then reconnect USB and run the **exact collection command printed in step
   1**. Do not attach the debugger or restart the app first. Note separately
   when you connected USB or turned the screen on and whether that restored
   playback. Collect promptly, within the ten-minute recording window where
   practical. If no fault happens, collect at ten minutes and report that;
   that is an inconclusive run, not a pass or a request to repeat indefinitely.

4. Share the evidence ZIP printed by the script, plus the stall and manual-wake
   times and which controller was used. The ZIP contains a `.pftrace`, filtered
   engine/system logs, installed-build details and **post-reconnect** snapshots.
   Those snapshots cannot establish the earlier stalled state. Reconnection
   itself can change it; the trace is what preserves the earlier history.

The script changes no power settings and clears no existing logs. Device-side
capture files remain in the two exact locations it prints, for recovery if
transfer fails. Inspect the evidence before sharing: system traces/logs can
contain app/process names, activity/intent metadata and device identifiers.
No audio or utterance text is intentionally collected by the new engine logs.
Do not commit recordings; `diagnostics/` and `*.pftrace` are ignored by Git.

## Evidence required for the conclusion

First verify the recording covers the incident, identifies the engine/native
PIDs, contains scheduling data and has no relevant loss. Unsupported ftrace
events, overwritten buffers or absent symbols limit conclusions; absence of an
event in an unavailable data source is not evidence that it did not occur.

| Evidence during the same stall | Justified next action |
| --- | --- |
| The engine's threads run; a specific pipe, callback, lock or retirement step does not return | Fix that engine-side path. Continuing app logs support this branch, not a whole-process freeze claim. |
| Native inference is waiting after a GPU stage while the deadline thread runs and recovery starts on time | Inspect the native wait and the exact retirement/replacement result. A blocked OpenCL call alone does not prove a vendor bug; check our queue/lifetime use. |
| The deadline is late while its thread is runnable but not scheduled | Investigate scheduling/priority/lifecycle with the captured thread evidence. Do not disguise scheduler delay as slow model inference. |
| Explicit system freeze/suspend events coincide with the relevant process/thread gap | Treat this as execution availability/lifecycle evidence. Check the controller's supported background-playback lifecycle and service binding; do not infer it merely from screen-off state. |
| A fresh worker also cannot progress until manual screen/USB activity, despite correct retirement and available execution | Document the affected device/build/backend combination as unreliable for this use case unless the trace exposes a fixable native mistake. Do not claim recovery works autonomously. |

An `S` (sleeping) or `D` (uninterruptible wait) thread state does not by itself
identify the cause, and an `*_end` marker only says the host function returned;
it does not assert all GPU work completed. We must combine stage timestamps,
thread scheduling/waits, deadline timing and power transitions. A successful
fresh worker after screen-on/USB is **not** evidence of autonomous recovery.

## Stopping rule and production choices

The bounded plan is: **this capture, one evidence-based correction if warranted,
and one validation round**. If the evidence is incomplete or tracing prevents
reproduction, say so. Do not invent a root cause or silently extend the plan
into another series of speculative builds; agree on any additional work first.

If an app-level correction is supported, validate unplugged/screen-off with the
same controller, including buffer exhaustion, stop/seek and voice changes.
Allow one recovery attempt only, verify the old worker exits before replacement,
and require recovery without manual wake. Then do a release-build reading run
without these diagnostics; a traced/debug-only success is insufficient.

If no supported correction is demonstrated, the honest end point is **a scoped
compatibility limitation**, not “Android GPUs do not work in the background.”
State the tested device/OS/driver, reproduction conditions, what was observed,
what remains unknown, and what the fallback costs. Options include an explicit
error so the controller can offer another engine, an optional CPU backend
(additional implementation and performance testing required), or warning that
uninterrupted screen-off reading is not supported on that configuration. A
timeout can only run when its process is scheduled; it is not an unconditional
wall-clock recovery guarantee. Keeping the screen on is a workaround, not a fix.

## Primary references

- [Perfetto CPU scheduling](https://perfetto.dev/docs/data-sources/cpu-scheduling)
- [Perfetto detached sessions](https://perfetto.dev/docs/concepts/detached-mode)
- [Android cached-app freezer](https://source.android.com/docs/core/perf/cached-apps-freezer)
- [Perfetto Android log source: userdebug-only](https://perfetto.dev/docs/data-sources/android-log)
