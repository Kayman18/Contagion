#!/usr/bin/env bash
# BAND20 exactness check: build the band simulator at VC_VOICES=4 with the
# VC_FLOAT_HASH hook, run every BAND19 script, and compare the hash of every
# rendered float sample (and each script's fail count) with BAND19's
# (validation/band19-float-hashes.txt). Takes a few minutes.
set -euo pipefail
cd "$(dirname "$0")/.."
work="${TMPDIR:-/tmp}/band20-exact"; rm -rf "$work"; mkdir -p "$work"
cp -a . "$work/tree"; cd "$work/tree"; rm -rf app-band/build
sed -i 's/#define VC_VOICES_BUILD [0-9]*/#define VC_VOICES_BUILD 4/' engine/voice_count.h
sed -i 's/o2_cflags = \["-O2",/o2_cflags = ["-O2", "-DVC_FLOAT_HASH",/' app-band/fm1.toml
python3 base/tools/build.py app-band --host >/dev/null
: > "$work/hashes.txt"
while read -r name rest; do
  mkdir -p "out/$name"
  r=$(timeout 900 app-band/build/host/virooos_host "tests/$name.txt" "out/$name" 2>&1 || true)
  echo "$name $(echo "$r" | grep -o 'VC_FLOAT_HASH [0-9a-f]*' || true) fails=$(echo "$r" | grep -c FAIL || true)" >> "$work/hashes.txt"
done < validation/band19-float-hashes.txt
if diff <(sed 's/  */ /g' validation/band19-float-hashes.txt) <(sed 's/  */ /g' "$work/hashes.txt"); then
  echo "PASS: $(wc -l < "$work/hashes.txt") scripts bit-identical to BAND19 at four voices"
else echo "FAIL: see diff above"; exit 1; fi
