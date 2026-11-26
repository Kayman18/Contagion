#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "$0")"
: "${JIELI_TOOLCHAIN:?Set JIELI_TOOLCHAIN to your JieLi pi32v2 toolchain root}"
export AC79_SDK="${AC79_SDK:-$PWD/build-deps/sdk}"
python3 base/tools/build.py app --release 0.74.0 --display-version "TEST74" "$@"
