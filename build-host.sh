#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "$0")"
python3 base/tools/build.py app --host
