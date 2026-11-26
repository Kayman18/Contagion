#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "$0")/.."
python3 tools/check_patch_schema.py
python3 base/tools/build.py app-band --host
mkdir -p build/user-storage
app-band/build/host/virooos_host tests/user-storage.txt build/user-storage
