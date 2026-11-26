#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "$0")/.."
python3 base/tools/build.py app-band --host
mkdir -p build/user-naming
app-band/build/host/virooos_host tests/user-naming.txt build/user-naming
