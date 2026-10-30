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

from dataclasses import dataclass


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
    # The HDMI picture is this many machine lines tall, which sets the pixel size.
    frame_lines: int = 240
    # Capture pixels are this many times wider than tall (VIC-20: double width).
    pixel_aspect: int = 1
    # Machine lines the text sits below the centre of the frame.
    text_offset: int = 0
    # Fraction of the way from background to ink. 0.50-0.65 read every frame
    # recorded from a Pi 3 and a Pi 4. Low-contrast C64 strokes sit close to the
    # top of that range: at 0.60 a Pi 3 "J" was at the 2-bit limit, at 0.55 it is 0.
    ink_threshold: float = 0.55
