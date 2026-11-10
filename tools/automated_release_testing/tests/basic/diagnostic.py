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

"""Signal encoding/decoding and audio measurements; no test orchestration."""

import math
import struct
from dataclasses import dataclass


from support.constants import WIDTH, HEIGHT, SAMPLE_RATE
MAGIC = b"\xb6\x64"


def payload(run_id: int, counter: int, state: int) -> bytes:
    body = MAGIC + run_id.to_bytes(4, "big") + bytes((counter, state))
    return body + bytes((sum(body) & 255, 0xA5))


@dataclass(frozen=True)
class Marker:
    run_id: int
    counter: int
    state: int
    bounds: tuple[int, int, int, int]


def decode_frame(rgb: bytes, run_id: int) -> Marker | None:
    if len(rgb) != WIDTH * HEIGHT * 3:
        raise ValueError("incorrect RGB frame size")
    xs, ys = [], []
    for y in range(HEIGHT):
        for x in range(WIDTH):
            offset = (y * WIDTH + x) * 3
            r, g, b = rgb[offset:offset + 3]
            if r > 65 and r > g * 1.6 + 15 and r > b * 1.4 + 10:
                xs.append(x)
                ys.append(y)
    if len(xs) < 300:
        return None
    left, right, top, bottom = min(xs), max(xs), min(ys), max(ys)
    width, height = right - left + 1, bottom - top + 1
    if width < 100 or height < 70:
        return None
    decoded = bytearray(10)
    for bit in range(80):
        row, column = divmod(bit, 10)
        cx = left + (1 + column * 3 + 1.5) * width / 32
        cy = top + (1 + row * 2 + 1) * height / 18
        levels = []
        for dy in (-1, 0, 1):
            for dx in (-1, 0, 1):
                offset = ((int(cy) + dy) * WIDTH + int(cx) + dx) * 3
                levels.append(sum(rgb[offset:offset + 3]) / 3)
        level = sorted(levels)[4]
        if 80 < level < 150:
            return None
        if level >= 150:
            decoded[bit // 8] |= 1 << (7 - bit % 8)
    if (decoded[:2] != MAGIC or decoded[9] != 0xA5
            or decoded[8] != sum(decoded[:8]) & 255
            or int.from_bytes(decoded[2:6], "big") != run_id
            or decoded[7] not in (0, 1)):
        return None
    return Marker(run_id, decoded[6], decoded[7], (left, top, right, bottom))


def audio_metrics(pcm: bytes) -> dict[str, float]:
    """Measure AC RMS and peak sinusoidal energy near the PAL/NTSC SID tone."""
    if len(pcm) != SAMPLE_RATE * 2:
        raise ValueError("audio assertion requires one second of mono s16le")
    samples = struct.unpack(f"<{SAMPLE_RATE}h", pcm)
    mean = sum(samples) / len(samples)
    centered = [(sample - mean) / 32768 for sample in samples]
    energy = sum(sample * sample for sample in centered)
    rms = math.sqrt(energy / len(samples))
    peak_fraction = 0.0
    # Short windows tolerate PAL/NTSC clock differences and capture resampling.
    for start in range(0, SAMPLE_RATE, 2000):
        window = centered[start:start + 2000]
        window_energy = sum(sample * sample for sample in window)
        if not window_energy:
            continue
        for frequency in range(900, 1101, 4):
            coefficient = 2 * math.cos(2 * math.pi * frequency / SAMPLE_RATE)
            first = second = 0.0
            for sample in window:
                value = sample + coefficient * first - second
                second, first = first, value
            power = first * first + second * second - coefficient * first * second
            fraction = 2 * power / (len(window) * window_energy)
            peak_fraction = max(peak_fraction, fraction)
    return {"rms": rms, "tone_fraction": peak_fraction}


def is_tone(metrics: dict[str, float]) -> bool:
    return metrics["rms"] >= 0.003 and metrics["tone_fraction"] >= 0.2


def is_silence(metrics: dict[str, float], tone_rms: float) -> bool:
    return metrics["rms"] <= max(0.001, tone_rms * 0.15)
