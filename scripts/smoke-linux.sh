#!/usr/bin/env bash
# Isolated X11 process/window acceptance smoke, not a visual or usability test.
# Requires xvfb-run, xauth, xwininfo, timeout and software OpenGL.
set -euo pipefail

if [[ $# -ne 1 || ! -f "$1" ]]; then
    echo "Usage: $0 /path/to/goreecloud-terminal" >&2
    exit 2
fi

app="$(realpath "$1")"
test_home="$(mktemp -d /tmp/goreecloud-terminal-smoke.XXXXXX)"
trap 'rm -rf -- "$test_home"' EXIT

# Xvfb prevents interaction with an existing desktop. Clearing the environment
# and using /bin/sh prevents reading the user's shell profile, history or HOME.
xvfb-run -a -s '-screen 0 1280x800x24' bash -c '
    set -u
    home=$1
    binary=$2
    timeout --signal=TERM --kill-after=2s 7s env -i \
      PATH=/usr/bin:/bin \
      HOME="$home" \
      SHELL=/bin/sh \
      DISPLAY="$DISPLAY" \
      XAUTHORITY="${XAUTHORITY:-}" \
      LIBGL_ALWAYS_SOFTWARE=1 \
      LANG=C.UTF-8 \
      "$binary" >"$home/runtime.log" 2>&1 &
    pid=$!
    sleep 2
    if xwininfo -root -tree | grep -Fq "\"GoreeCloud Terminal\""; then
        window_found=yes
    else
        window_found=no
    fi
    if wait "$pid"; then
        status=0
    else
        status=$?
    fi
    if [[ "$window_found" != yes || "$status" -ne 124 ]]; then
        printf "Native window smoke failed: window=%s process=%s\n" "$window_found" "$status" >&2
        cat "$home/runtime.log" >&2
        exit 1
    fi
    echo "Native X11 smoke passed: GoreeCloud Terminal window, 7-second process lifetime."
' sh "$test_home" "$app"
