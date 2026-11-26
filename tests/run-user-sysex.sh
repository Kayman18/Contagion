#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "$0")/.."
bash tests/run-presets.sh
python3 tests/prepare-user-sysex.py
python3 base/tools/build.py app-band --host
app-band/build/host/virooos_host tests/user-sysex.txt build/user-sysex
python3 tests/check-user-sysex.py
