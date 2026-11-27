"""Losslessly pack the original signed Q23 mid-EQ coefficients in 32-pair blocks."""
from pathlib import Path
import re

root = Path(__file__).resolve().parents[1]
source = (root / "engine/eq.inc").read_text()
body = re.search(r"eq_mid_packed\[98304\]=\{(.*?)\};", source, re.S)[1]
raw = bytes(map(int, re.findall(r"\d+", body)))
assert len(raw) == 98304
packed = bytearray()
offsets = [0]
for at in range(0, len(raw), 32 * 6):
    for lane in (0, 3):
        prev = delta = 0
        for i in range(at + lane, at + 32 * 6, 6):
            value = int.from_bytes(raw[i:i+3], "little", signed=True)
            diff = value - prev - delta
            delta, prev = value - prev, value
            encoded = diff * 2 if diff >= 0 else -diff * 2 - 1
            while encoded >= 128:
                packed.append(128 | (encoded & 127))
                encoded >>= 7
            packed.append(encoded)
    offsets.append(len(packed))
assert offsets[-1] <= 65535

def array(kind, name, data):
    lines = [",".join(map(str, data[i:i+24])) for i in range(0, len(data), 24)]
    return f"static const {kind} {name}[]={{\n" + ",\n".join(lines) + "\n};\n"

(root / "engine/eq_blocks.inc").write_text(
    array("uint16_t", "eq_block_offsets", offsets) + array("uint8_t", "eq_blocks", packed))
