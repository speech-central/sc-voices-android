#!/system/bin/sh
# Copyright 2026 Labsii Ltd. SPDX-License-Identifier: Apache-2.0
# Invoked by capture_stall.sh. Shell UID, no root, no power setting changes.
set -eu
action=${1:-}
record_dir=${2:-}
case "$record_dir" in /data/local/tmp/kokoro-stall-*) ;; *) exit 2 ;; esac
base=${record_dir##*/}
case "$base" in *[!a-zA-Z0-9._-]*) exit 2 ;; esac
[ "$record_dir" = "/data/local/tmp/$base" ] || exit 2
logfile="$record_dir/logcat.txt"

case "$action" in
  start)
    command -v nohup >/dev/null
    command -v timeout >/dev/null
    command -v pidof >/dev/null
    # logcat -T 1 avoids importing a previous session. Rotation caps logs at
    # roughly 8 MiB; timeout also bounds lifetime if collection is forgotten.
    # Perfetto's android.log source is userdebug-only, so use logcat here.
    nohup timeout -s TERM -k 5 600 logcat \
      -b main -b system -b events -v threadtime -T 1 \
      -f "$logfile" -r 2048 -n 3 \
      AdrenoTtsService:I KokoroSession:I KokoroCapture:I \
      ActivityManager:I CachedAppOptimizer:I DeviceIdleController:I \
      PowerManagerService:I lmkd:I am_freeze:I am_unfreeze:I \
      am_kill:I am_proc_died:I '*:S' \
      </dev/null >"$record_dir/logcat-launch.txt" 2>&1 &
    launcher=$!
    sleep 1
    if ! kill -0 "$launcher" 2>/dev/null; then
      cat "$record_dir/logcat-launch.txt" >&2
      exit 1
    fi
    ;;
  stop)
    # Never kill a process by a saved PID alone (PID reuse) or kill all logcat
    # sessions. Select a live logcat whose argv contains this exact unique file.
    for logger in $(pidof logcat 2>/dev/null || true); do
      case "$logger" in ''|*[!0-9]*) continue ;; esac
      if [ -r "/proc/$logger/cmdline" ] &&
         tr '\000' '\n' <"/proc/$logger/cmdline" | grep -Fqx "$logfile"; then
        kill -TERM "$logger" 2>/dev/null || true
      fi
    done
    ;;
  *) exit 2 ;;
esac
