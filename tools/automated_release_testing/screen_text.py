#!/usr/bin/env python3
#
# BMC64 - A bare metal Commodore emulator for Raspberry Pi
# Copyright (C) 2026 aminch <https://github.com/aminch>
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program. If not, see <https://www.gnu.org/licenses/>.

"""Read captured text screens cell by cell against the machine's character ROM.

The font is the ROM the emulator itself draws with, read from the SD card at
run time. No ROM data is stored in this repository.
"""

from dataclasses import dataclass, field
from harness import TestFailure


@dataclass(frozen=True)
class Layout:
    columns: int
    rows: int
    input_row: int
    channel: int
    inverted: bool
    font_path: str
    font_offset: int
    screen: tuple[tuple[int, str], ...]


LAYOUTS = {
    "C64": Layout(40, 25, 6, 0, False, "/C64/chargen", 0, (
        (1, "    **** COMMODORE 64 BASIC V2 ****"),
        (3, " 64K RAM SYSTEM  38911 BASIC BYTES FREE"),
        (5, "READY."),
    )),
    "C128": Layout(40, 25, 7, 1, False, "/C128/chargen", 0, (
        (1, " COMMODORE BASIC V7.0 122365 BYTES FREE"),
        (2, "   (C)1986 COMMODORE ELECTRONICS, LTD."),
        (3, "         (C)1977 MICROSOFT CORP."),
        (4, "           ALL RIGHTS RESERVED"),
        (6, "READY."),
    )),
    "VIC20": Layout(22, 23, 5, 0, True, "/VIC20/chargen", 0, (
        (0, "**** CBM BASIC V2 ****"),
        (2, "3583 BYTES FREE"),
        (4, "READY."),
    )),
    # TED machines keep the character set inside the kernal ROM at $D000.
    "Plus4": Layout(40, 25, 5, 0, True, "/PLUS4/kernal", 0x1000, (
        (1, " COMMODORE BASIC V3.5 60671 BYTES FREE"),
        (2, " 3-PLUS-1 ON KEY F1"),
        (4, "READY."),
    )),
    "Plus4Emu": Layout(40, 25, 4, 0, True, "/PLUS4EMU/p4kernal.rom", 0x1000, (
        (1, " COMMODORE BASIC V3.5 60671 BYTES FREE"),
        (3, "READY."),
    )),
    # The test profile selects a 40-column 3016: 80-column glyphs are only two
    # capture pixels wide, finer than the capture device resolves. Its
    # graphics keyboard editor boots with the upper case set (first 1K).
    "Pet": Layout(40, 25, 5, 1, False, "/PET/chargen", 0, (
        (0, "### COMMODORE BASIC ###"),
        (2, " 15359 BYTES FREE"),
        (4, "READY."),
    )),
}

LETTERS = "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
MAX_GLYPH_ERROR = 3
MAX_TYPED_ERROR = 2
MIN_TYPED_MARGIN = 2
# A wrong key must never pass as the right one: type only letters whose ROM
# glyph differs from every other letter by at least this many bits.
TYPING_SEPARATION = 6
# Fraction of the way from background to ink. 0.55-0.70 read every recorded
# frame correctly; 0.65 reads them all with zero bit errors.
INK_THRESHOLD = 0.65
SOLID = (1 << 64) - 1


def screen_code(char: str) -> int:
    if char == "@":
        return 0
    if "A" <= char <= "Z" or char == "[":
        return ord(char) - 64
    if " " <= char <= "?":
        return ord(char)
    raise ValueError(f"no screen code for {char!r}")


class Font:
    def __init__(self, data: bytes, offset: int):
        if len(data) < offset + 128 * 8:
            raise TestFailure("font_rom", f"character ROM too short ({len(data)} bytes)")
        self.glyphs = []
        for code in range(128):
            rows = data[offset + code * 8:offset + code * 8 + 8]
            self.glyphs.append(sum(1 << (gy * 8 + gx) for gy in range(8) for gx in range(8)
                                   if rows[gy] & (0x80 >> gx)))
        if self.glyphs[32] or len(set(self.glyphs[1:27])) != 26:
            raise TestFailure("font_rom", "data at the configured offset is not a character set")

    def glyph(self, char: str) -> int:
        return self.glyphs[screen_code(char)]


@dataclass(frozen=True)
class Geometry:
    left: float
    top: float
    pixel_width: float
    pixel_height: float

    def moved(self, dx: float = 0, dy: float = 0, sx: float = 1, sy: float = 1) -> "Geometry":
        return Geometry(self.left + dx, self.top + dy,
                        self.pixel_width * sx, self.pixel_height * sy)


@dataclass
class Reading:
    """One frame compared cell by cell with the expected screen."""
    geometry: Geometry | None
    threshold: float = 0
    contrast: int = 0
    # (row, column, expected character, bit distance); row -1 is frame-level.
    mismatches: list[tuple[int, int, str, int]] = field(default_factory=list)
    distances: list[int] = field(default_factory=list)
    unreadable: bool = False

    @property
    def matched(self) -> bool:
        return not self.mismatches and not self.unreadable

    @property
    def grid(self) -> dict[str, float] | None:
        if self.geometry is None:
            return None
        return {"left": round(self.geometry.left, 2), "top": round(self.geometry.top, 2),
                "pixel_width": round(self.geometry.pixel_width, 4),
                "pixel_height": round(self.geometry.pixel_height, 4),
                "threshold": round(self.threshold, 1)}

    def summary(self) -> str:
        if self.matched:
            return "every checked cell matches the character ROM"
        if self.geometry is None:
            return "no character grid fits the frame"
        cells = [item for item in self.mismatches if item[0] >= 0]
        text = "; ".join(f"row {row} col {column} expected {char!r} ({distance} bits off)"
                         for row, column, char, distance in cells[:3])
        prefix = "banner unreadable (corrupt capture or unexpected screen); " if self.unreadable else ""
        if self.contrast < 30:
            prefix += f"contrast {self.contrast}; "
        return f"{prefix}{len(cells)} cell(s) differ: {text}"


class ScreenText:
    def __init__(self, machine: str, font: Font, width: int, height: int):
        self.machine = machine
        self.layout = LAYOUTS[machine]
        self.font = font
        self.width, self.height = width, height
        self.expected = {row: text for row, text in self.layout.screen}
        self.reference_cells = [(column, row, char) for row, text in self.layout.screen
                                for column, char in enumerate(text) if char != " "]
        longest = max(self.expected, key=lambda row: len(self.expected[row].strip()))
        self.coarse_cells = [cell for cell in self.reference_cells if cell[1] == longest]
        edges = {0, self.layout.columns - 1}
        self.fit_cells = self.reference_cells + [
            (column, row, " ") for row in range(self.layout.input_row)
            for column in range(self.layout.columns)
            if self.expected.get(row, "")[column:column + 1].strip() == ""
            and (column in edges or column < len(self.expected.get(row, "")))]
        self.alphabet = "".join(
            letter for letter in LETTERS
            if min((font.glyph(letter) ^ font.glyph(other)).bit_count()
                   for other in LETTERS if other != letter) >= TYPING_SEPARATION)
        if len(self.alphabet) < 8:
            raise TestFailure("font_rom", f"{machine} font has too few distinct letters for typing")

    def plane(self, rgb: bytes) -> list[bytes]:
        """One colour channel, text bright, with dark scanlines suppressed."""
        if len(rgb) != self.width * self.height * 3:
            raise ValueError("incorrect RGB frame size")
        channel = rgb[self.layout.channel::3]
        rows = [channel[y * self.width:(y + 1) * self.width] for y in range(self.height)]
        # Suppress scanlines before inverting dark text, or they would read as ink.
        plane = [bytes(map(max, rows[max(0, y - 1)], rows[y], rows[min(self.height - 1, y + 1)]))
                 for y in range(self.height)]
        if self.layout.inverted:
            invert = bytes(range(255, -1, -1))
            plane = [line.translate(invert) for line in plane]
        return plane

    def nominal(self, bounds: tuple[int, int, int, int]) -> Geometry:
        if self.machine == "Pet":
            # The PET has no pane edge. Its 40-column PAL HDMI entry scales a
            # 376x270 frame to the full output height and centres the 320x200
            # text area in it.
            pixel = self.height / 270
            bounds = (round((self.width - 320 * pixel) / 2), round((self.height - 200 * pixel) / 2),
                      round(320 * pixel), round(200 * pixel))
        left, top, width, height = bounds
        return Geometry(left, top, width / (self.layout.columns * 8),
                        height / (self.layout.rows * 8))

    def samples(self, plane: list[bytes], geometry: Geometry,
                column: int, row: int) -> list[int] | None:
        """The centre of each of the cell's 8x8 character pixels."""
        values = []
        for gy in range(8):
            y = int(geometry.top + (row * 8 + gy + 0.5) * geometry.pixel_height)
            if not 0 <= y < self.height:
                return None
            line = plane[y]
            for gx in range(8):
                x = int(geometry.left + (column * 8 + gx + 0.5) * geometry.pixel_width)
                if not 0 <= x < self.width:
                    return None
                values.append(line[x])
        return values

    @staticmethod
    def bits(values: list[int], threshold: float) -> int:
        return sum(1 << index for index, value in enumerate(values) if value > threshold)

    def score(self, plane: list[bytes], geometry: Geometry,
              cells: list[tuple[int, int, str]]) -> tuple[int, float, int] | None:
        """Total bit errors against the font; the threshold comes from non-blank cells."""
        samples = [self.samples(plane, geometry, column, row) for column, row, _ in cells]
        if any(values is None for values in samples):
            return None
        ordered = sorted(value for values, (_, _, char) in zip(samples, cells) if char != " "
                         for value in values)
        low, high = ordered[len(ordered) // 10], ordered[-len(ordered) // 10]
        # MJPEG stores colour at half horizontal resolution, so ink bleeds into
        # its neighbours at about half strength: threshold above that.
        threshold = low + (high - low) * INK_THRESHOLD
        errors = sum((self.bits(values, threshold) ^ self.font.glyph(char)).bit_count()
                     for values, (_, _, char) in zip(samples, cells))
        return errors, threshold, high - low

    def fit(self, plane: list[bytes], start: Geometry) -> tuple[Geometry, float, int] | None:
        # Pane edges are only approximate: search one character cell for the
        # longest banner line, then refine position and scale using every known
        # character and the blank cells beside the text.
        best = None
        steps_x = int(4 * start.pixel_width) + 1
        steps_y = int(4 * start.pixel_height) + 1
        for iy in range(-steps_y, steps_y + 1):
            for ix in range(-steps_x, steps_x + 1):
                candidate = start.moved(ix * 2, iy * 2)
                result = self.score(plane, candidate, self.coarse_cells)
                if result and (best is None or result[0] < best[0]):
                    best = (result[0], candidate)
        if best is None:
            return None
        geometry, current = best[1], None
        for candidate in [best[1].moved(dx / 2, dy / 2, 1 + sx / 200, 1 + sy / 200)
                          for dx in range(-2, 3) for dy in range(-2, 3)
                          for sx in (-1, 0, 1) for sy in (-1, 0, 1)]:
            result = self.score(plane, candidate, self.fit_cells)
            if result and (current is None or result[0] < current[0]):
                geometry, current = candidate, result
        moves = [(dx, dy) for dx in (-1, 0, 1) for dy in (-1, 0, 1) if dx or dy]
        for step in (0.5, 0.25):
            improved = True
            while improved and current:
                improved = False
                candidates = [geometry.moved(dx * step, dy * step) for dx, dy in moves]
                candidates += [geometry.moved(sx=1 + sx * step / 100, sy=1 + sy * step / 100)
                               for sx, sy in moves]
                for candidate in candidates:
                    result = self.score(plane, candidate, self.fit_cells)
                    if result and result[0] < current[0]:
                        geometry, current, improved = candidate, result, True
        if current is None:
            return None
        return geometry, current[1], current[2]

    def read(self, rgb: bytes, bounds: tuple[int, int, int, int], typed: str = "") -> Reading:
        """Compare the frame with the boot screen, plus `typed` on the input row.

        Every cell in the checked rows must match: expected characters within
        MAX_GLYPH_ERROR bits of their ROM glyph and everything else blank. The
        cell after the input is the blinking cursor, so blank or solid passes.
        """
        plane = self.plane(rgb)
        return self.compare(plane, self.fit(plane, self.nominal(bounds)), typed)

    def compare(self, plane: list[bytes], fitted: tuple[Geometry, float, int] | None,
                typed: str = "") -> Reading:
        if fitted is None:
            return Reading(None, unreadable=True)
        geometry, threshold, contrast = fitted
        reading = Reading(geometry, threshold, contrast)
        if contrast < 30:
            reading.mismatches.append((-1, -1, "contrast", contrast))
        cursor = (len(typed), self.layout.input_row)
        reference_errors = 0
        for row in range(min(self.layout.rows, self.layout.input_row + 3)):
            text = typed if row == self.layout.input_row else self.expected.get(row, "")
            for column in range(self.layout.columns):
                char = text[column] if column < len(text) else " "
                values = self.samples(plane, geometry, column, row)
                if values is None:
                    reading.mismatches.append((row, column, char, 64))
                    continue
                observed = self.bits(values, threshold)
                distance = (observed ^ self.font.glyph(char)).bit_count()
                if (column, row) == cursor:
                    distance = min(distance, (observed ^ SOLID).bit_count())
                ok = distance <= MAX_GLYPH_ERROR
                if row == self.layout.input_row and column < len(typed):
                    reading.distances.append(distance)
                    nearest = min((observed ^ self.font.glyph(letter)).bit_count()
                                  for letter in LETTERS if letter != char)
                    ok = distance <= MAX_TYPED_ERROR and nearest - distance >= MIN_TYPED_MARGIN
                if not ok:
                    reading.mismatches.append((row, column, char, distance))
                    if row != self.layout.input_row and char != " ":
                        reference_errors += 1
        reading.unreadable = contrast < 30 or reference_errors * 2 > len(self.reference_cells)
        return reading
