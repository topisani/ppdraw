#!/usr/bin/env python3
# vim: ft=python
#
# Converts a BDF font to a C source file (usage: bdf2c.py INPUT OUTPUT [-n NAME]). Standard library only.

import argparse
import math
import re
from dataclasses import dataclass, field
from pathlib import Path


@dataclass
class Glyph:
    name: str
    encoding: int = -1
    advance: int = 0
    dimensions: tuple[int, int] = (0, 0)
    offset: tuple[int, int] = (0, 0)
    bitmap: list[list[int]] = field(default_factory=list)


def parse_bdf(path: Path):
    """Minimal BDF parser: returns (bounding box (w, h, x, y), glyphs)."""
    bbox = (0, 0, 0, 0)
    glyphs: list[Glyph] = []
    glyph = None
    in_bitmap = False
    for line in path.read_text().splitlines():
        key, _, rest = line.strip().partition(" ")
        args = rest.split()
        if in_bitmap:
            if key == "ENDCHAR":
                in_bitmap = False
            else:
                bits = bin(int(key, 16))[2:].zfill(len(key) * 4)
                glyph.bitmap.append([int(b) for b in bits[: glyph.dimensions[0]]])
        elif key == "FONTBOUNDINGBOX":
            bbox = tuple(map(int, args))
        elif key == "STARTCHAR":
            glyph = Glyph(rest)
            glyphs.append(glyph)
        elif glyph is None:
            continue
        elif key == "ENCODING":
            glyph.encoding = int(args[0])
        elif key == "DWIDTH":
            glyph.advance = int(args[0])
        elif key == "BBX":
            w, h, x, y = map(int, args)
            glyph.dimensions, glyph.offset = (w, h), (x, y)
        elif key == "BITMAP":
            in_bitmap = True
    return bbox, glyphs


def bdf2c(input: Path, output: Path, name: str | None = None):
    if name is None:
        name = "font_" + re.sub(r"\W", "_", output.stem.removesuffix(".bdf"))

    (width, height, font_x, font_y), glyphs = parse_bdf(input)
    glyphs = sorted((g for g in glyphs if 0 < g.encoding <= 0xFFFF), key=lambda g: g.encoding)

    out = [
        '#include "font.h"',
        "",
        "",
        f"// dims: {(width, height)}, offset: {[font_x, font_y]}",
        "static const uint8_t __font_bitmap__[] = {",
    ]
    for g in glyphs:
        out.append(f"// char: (U+{g.encoding:04X}) {g.name}")
        out.append(
            f"// advance_width: {g.advance}, dims: {g.dimensions}, offset: {list(g.offset)}"
        )

        # The glyph offset is the position of its lower left corner
        offs_x = font_x + g.offset[0]
        offs_y = font_y - g.offset[1] + (height - g.dimensions[1])

        nbits = math.ceil(width / 8) * 8
        rows = [[]] * offs_y + g.bitmap
        rows += [[]] * (height - len(rows))
        for row in rows[:height]:
            cells = ([0] * offs_x + row + [0] * nbits)[:nbits]
            out.append(
                "".join(
                    "".join("X" if c else "_" for c in cells[i : i + 8]) + ","
                    for i in range(0, len(cells), 8)
                )
            )
        out.append("")
    out += ["};", "", "static const uint16_t __font_index__[] = {"]
    out += [f"0x{g.encoding:X}," for g in glyphs]
    out += ["};", "", "static const uint8_t __font_widths__[] = {"]
    out += [f"{g.advance}," for g in glyphs]
    out += [
        "};",
        "",
        f"const struct draw_font {name} = {{",
        f"    .width = {width},",
        f"    .height = {height},",
        f"    .chars = {len(glyphs)},",
        "    .bitmap = __font_bitmap__,",
        "    .index = __font_index__,",
        "    .widths = __font_widths__,",
        f'    .name = "{name}",',
        "};",
    ]

    output.parent.mkdir(parents=True, exist_ok=True)
    output.write_text("\n".join(out) + "\n")


if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument("input", type=Path)
    parser.add_argument("output", type=Path)
    parser.add_argument("-n", "--name", default=None)
    bdf2c(**vars(parser.parse_args()))
