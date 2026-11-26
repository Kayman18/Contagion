#!/usr/bin/env python3
"""Generate the FM-1 splash bitmap and rotary-knob sprite headers."""
import math
from pathlib import Path
import sys
from PIL import Image

ROOT = Path(__file__).resolve().parent.parent
OUT = ROOT / "app" / "access_art.inc"


def rgb565(red, green, blue):
    return ((red >> 3) << 11) | ((green >> 2) << 5) | (blue >> 3)


def packbits(data):
    encoded = []
    cursor = 0
    while cursor < len(data):
        run = cursor + 1
        while run < len(data) and run - cursor < 128 and data[run] == data[cursor]:
            run += 1
        if run - cursor >= 3:
            encoded.extend((127 + run - cursor, data[cursor]))
        else:
            run = cursor
            while run < len(data) and run - cursor < 128:
                if run + 2 < len(data) and data[run] == data[run + 1] == data[run + 2]:
                    break
                run += 1
            encoded.append(run - cursor - 1)
            encoded.extend(data[cursor:run])
        cursor = run
    return encoded


def c_array(name, values, kind="uint8_t", width=24):
    rows = [" " + ",".join(map(str, values[i:i + width])) + ","
            for i in range(0, len(values), width)]
    return f"static const {kind} {name}[]={{\n" + "\n".join(rows) + "\n};\n"


def knob_shade(x, y):
    radius = math.hypot(x, y)
    if radius > 24:
        return 0
    angle = math.atan2(y, x)
    light = -(x + y) / max(1, math.sqrt(2) * radius)
    if radius > 23:
        return round(2 + 2 * max(0, light))
    if radius > 20:
        return round(4 + 4 * max(0, light) + 2 * (math.cos(angle * 55) + 1))
    if radius > 18.5:
        return round(5 + 7 * max(0, light))
    return round(4 + 3 * (1 - y / 19) + max(0, light) * radius / 19)


splash = Image.open(sys.argv[1]).convert("RGB").resize((168, 168), Image.Resampling.LANCZOS)
# Navy grid, cyan linework and cream display lettering.
colors = [
    (4, 13, 25), (6, 19, 35), (7, 32, 50), (6, 49, 69),
    (5, 73, 94), (4, 101, 127), (4, 135, 157), (4, 172, 192),
    (4, 210, 222), (22, 243, 248), (101, 249, 253), (185, 252, 253),
    (148, 134, 105), (200, 184, 147), (237, 221, 187), (253, 243, 218),
]
indexed = Image.new("P", (1, 1))
palette = [channel for color in colors for channel in color]
indexed.putpalette(palette + palette[:3] * (256 - len(colors)))
pixels = splash.crop((0, 0, 168, 145)).quantize(
    palette=indexed, dither=Image.Dither.NONE
).tobytes()
assert max(pixels) < 16
packed = packbits(bytes(pixels[i] << 4 | pixels[i + 1] for i in range(0, len(pixels), 2)))
knob = [knob_shade(x, y) for y in range(-24, 25) for x in range(-24, 25)]
knob_palette = [rgb565(16 + k * 6, 16 + k * 6, 16 + k * 6) for k in range(16)]
x_outer, y_outer, x_inner, y_inner = [], [], [], []
for step in range(33):
    angle = math.radians(135 + 270 * step / 32)
    x_outer.append(round(28 * math.cos(angle)))
    y_outer.append(round(28 * math.sin(angle)))
    x_inner.append(round(16 * math.cos(angle)))
    y_inner.append(round(16 * math.sin(angle)))
OUT.write_text(
    "/* Generated FM-1 RGB565 assets. Splash image and rotary knob geometry. */\n"
    + "#define ACCESS_SPLASH_ROWS 207u\n#define ACCESS_ART_WIDTH 168u\n#define ACCESS_ART_HEIGHT 145u\n"
    + c_array("access_splash_palette", [rgb565(*palette[i:i+3]) for i in range(0, len(palette), 3)], "uint16_t")
    + c_array("access_splash_pixels", packed)
    + c_array("access_knob", knob)
    + c_array("access_knob_palette", knob_palette, "uint16_t")
    + c_array("access_arc_x", x_outer, "int8_t")
    + c_array("access_arc_y", y_outer, "int8_t")
    + c_array("access_pointer_x", x_inner, "int8_t")
    + c_array("access_pointer_y", y_inner, "int8_t")
)
print(f"Splash {len(packed)} bytes packed, knob {len(knob)} bytes")
