"""Build a sector-aligned, bit-exact cache of 34 oscillator and LFO tables."""
import json
import struct
import sys
import zlib
from collections import Counter
from pathlib import Path

root = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(root / "base/tools"))
from fm1pkg_make import crc16

base, end = 0x97000, 0xDC000
table_size = 2049 * 4
band_count = 14
preloaded = (0, 1, 2, 3, 4, 13)
harmonics = (128, 96, 64, 48, 32, 24, 16, 12, 8, 6, 4, 3, 2, 1)
patches = json.loads((root / "presets/factory-decoded.json").read_text())
needs = []
for row in patches:
    p = row["patch"]
    osc = {p["osc1Wave"], p["osc2Wave"], p["osc3Mode"] - 4 if p["osc3Mode"] >= 4 else p["osc2Wave"]}
    lfo = {(p[f"lfo{i}Shape"] - 4, 0) for i in (1, 2, 3) if p[f"lfo{i}Shape"] >= 6}
    needs.append({(wave, band) for wave in osc for band in preloaded} | lfo)
counts = Counter(key for keys in needs for key in keys)
ranked = sorted(counts, key=lambda key: (-harmonics[key[1]] * counts[key], key))
pinned = {(wave, band) for wave in (0, 1, 63) for band in preloaded}
selected = sorted(pinned | set([key for key in ranked if key not in pinned][:34 - len(pinned)]))
assert len(selected) == 34

source = (root / "data/source-waves.f32").read_bytes()
assert len(source) == 64 * band_count * table_size
entries, data = bytearray(), bytearray()
for wave, band in selected:
    start = (wave * band_count + band) * table_size
    table = source[start:start + table_size]
    assert len(table) == table_size and table[:4] == table[-4:]
    entries += struct.pack("<BBHI", wave, band, 0, zlib.crc32(table))
    data += table
header = struct.pack("<8sIII", b"CTWAVE01", len(selected), table_size, zlib.crc32(entries))
image = header + entries + data
assert len(image) <= end - base
image = image.ljust(end - base, b"\xff")
out = root / "app/build"
out.mkdir(parents=True, exist_ok=True)
(out / "wave-cache.bin").write_bytes(image)
(root / "runtime/wavetable_flash_layout.h").write_text(
    "#pragma once\n"
    f"#define CONTAGION_CACHE_BASE 0x{base:X}u\n"
    f"#define CONTAGION_CACHE_END 0x{end:X}u\n"
    f"#define CONTAGION_CACHE_IMAGE_CRC16 0x{crc16(image):04X}u\n"
)
print(json.dumps({"tables": len(selected), "allocation": len(image),
                  "full_cold_recall": sum(keys <= set(selected) for keys in needs),
                  "some_hits": sum(bool(keys & set(selected)) for keys in needs)}))
