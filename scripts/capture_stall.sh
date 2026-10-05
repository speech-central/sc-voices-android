#!/usr/bin/env bash
# Copyright 2026 Labsii Ltd. SPDX-License-Identifier: Apache-2.0
# Mac/Linux host: start while connected, unplug, reproduce, reconnect, collect.
set -euo pipefail
script_dir=$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)
project_dir=$(cd -- "$script_dir/.." && pwd)
adb_bin=${ADB:-adb}
action=${1:-}
fail() { printf '%s\n' "$*" >&2; exit 1; }
usage() {
  printf '%s\n' \
    'Usage: bash scripts/capture_stall.sh start' \
    '       bash scripts/capture_stall.sh collect /path/printed/by/start' \
    'Use ANDROID_SERIAL to choose a device; ADB may be a full adb binary path.'
}
command -v "$adb_bin" >/dev/null || fail "adb not found. Set ADB to its full path."
case "$action" in start|collect) ;; *) usage; exit 2 ;; esac

if [[ "$action" == start ]]; then
  serial=$("$adb_bin" get-serialno | tr -d '\r')
  [[ -n "$serial" && "$serial" != unknown ]] || fail 'Connect exactly one authorized device, or set ANDROID_SERIAL.'
  device=("$adb_bin" -s "$serial")
  "${device[@]}" shell 'command -v perfetto && command -v nohup && command -v timeout && command -v pidof' >/dev/null
  mkdir -p "$project_dir/diagnostics"
  out=$(mktemp -d "$project_dir/diagnostics/kokoro-stall-XXXXXXXX")
  key=${out##*/}
  remote_dir="/data/local/tmp/$key"
  remote_trace="/data/misc/perfetto-traces/$key.pftrace"
  printf '%s\n' "$serial" >"$out/device-serial.txt"
  printf '%s\n' "$key" >"$out/session-key.txt"
  date -u '+%Y-%m-%dT%H:%M:%SZ' >"$out/started-host-utc.txt"
  cp "$script_dir/diagnostics/stall.pbtxt" "$out/stall.pbtxt"
  cp "$project_dir/docs/SCREEN_OFF_DIAGNOSTICS.md" "$out/READ_ME.md"
  "${device[@]}" shell "mkdir -m 700 '$remote_dir'"
  "${device[@]}" push "$script_dir/diagnostics/device_logcat.sh" "$remote_dir/device_logcat.sh" >/dev/null
  # These are pre-test observations, not samples taken during the later stall.
  "${device[@]}" shell 'getprop ro.build.fingerprint; getprop ro.build.version.sdk; date; cat /proc/uptime' >"$out/device-before.txt"
  "${device[@]}" shell dumpsys package com.labsii.voices >"$out/package-before.txt"
  "${device[@]}" shell "sh '$remote_dir/device_logcat.sh' start '$remote_dir'"
  if ! "${device[@]}" shell "perfetto --txt -c - --detach=$key -o '$remote_trace'" \
      <"$out/stall.pbtxt" >"$out/perfetto-start.txt" 2>&1; then
    "${device[@]}" shell "sh '$remote_dir/device_logcat.sh' stop '$remote_dir'" || true
    fail "Trace start failed; inspect $out/perfetto-start.txt. No power settings were changed."
  fi
  if ! "${device[@]}" shell "perfetto --is_detached=$key"; then
    "${device[@]}" shell "sh '$remote_dir/device_logcat.sh' stop '$remote_dir'" || true
    fail "Detached trace could not be confirmed. Keep $out; do not interpret this as a successful recording."
  fi
  "${device[@]}" shell log -t KokoroCapture "recording_started key=$key" || true
  printf '\nRecording is armed for at most 10 minutes. No debugger should be attached.\n'
  printf 'Unplug USB, use the same reader/text, and turn the screen off.\n'
  printf 'After a stall, note when it began and when you wake/reconnect the device.\n'
  printf 'Then run:\n  bash scripts/capture_stall.sh collect %q\n' "$out"
  printf 'Trace/logs contain app/process metadata; review before sharing.\n'
else
  [[ $# == 2 ]] || { usage; exit 2; }
  out=$(cd -- "$2" && pwd)
  [[ -f "$out/session-key.txt" && -f "$out/device-serial.txt" ]] || fail 'Not a capture directory.'
  IFS= read -r key <"$out/session-key.txt"
  IFS= read -r serial <"$out/device-serial.txt"
  case "$key" in kokoro-stall-*) ;; *) fail 'Invalid capture key.' ;; esac
  case "$key" in *[!a-zA-Z0-9._-]*) fail 'Invalid capture key.' ;; esac
  [[ "${out##*/}" == "$key" && -n "$serial" ]] || fail 'Capture metadata mismatch.'
  command -v zip >/dev/null || fail 'zip is required to package the evidence.'
  device=("$adb_bin" -s "$serial")
  [[ $("${device[@]}" get-state | tr -d '\r') == device ]] || fail 'Reconnect the same authorized device.'
  remote_dir="/data/local/tmp/$key"
  remote_trace="/data/misc/perfetto-traces/$key.pftrace"
  date -u '+%Y-%m-%dT%H:%M:%SZ' >"$out/collected-host-utc.txt"
  "${device[@]}" shell log -t KokoroCapture "collection_after_user_reconnect key=$key" || true
  if "${device[@]}" shell "perfetto --is_detached=$key"; then
    "${device[@]}" shell "perfetto --attach=$key --stop" >"$out/perfetto-stop.txt" 2>&1
  else
    printf '%s\n' 'Session already ended (duration limit, file cap or recording error); inspect trace coverage.' >"$out/perfetto-stop.txt"
  fi
  "${device[@]}" shell "sh '$remote_dir/device_logcat.sh' stop '$remote_dir'"
  "${device[@]}" pull "$remote_dir" "$out/device" >/dev/null
  "${device[@]}" pull "$remote_trace" "$out/stall.pftrace" >/dev/null
  [[ -s "$out/stall.pftrace" ]] || fail 'Trace is empty. Keep the diagnostic folder; recording was not successful.'
  # A reconnect can resolve the fault. These snapshots are explicitly labelled
  # POST-reconnect and must not be used as the state at the time of the stall.
  "${device[@]}" shell dumpsys activity processes >"$out/activity-processes-after.txt" 2>&1 || true
  "${device[@]}" shell dumpsys activity services com.labsii.voices >"$out/engine-services-after.txt" 2>&1 || true
  "${device[@]}" shell dumpsys power >"$out/power-after.txt" 2>&1 || true
  "${device[@]}" shell dumpsys deviceidle >"$out/deviceidle-after.txt" 2>&1 || true
  "${device[@]}" shell ps -A -T >"$out/threads-after.txt" 2>&1 || true
  archive="$out.zip"
  [[ ! -e "$archive" ]] || fail "Archive already exists: $archive. Evidence files remain in $out."
  (cd -- "$(dirname -- "$out")" && zip -qr "$archive" "$key")
  printf '\nEvidence saved: %s\n' "$archive"
  printf 'Share this ZIP and the approximate stall/wake times, after reviewing it for private metadata.\n'
  printf 'Device-side evidence is retained under %s and %s.\n' "$remote_dir" "$remote_trace"
fi
