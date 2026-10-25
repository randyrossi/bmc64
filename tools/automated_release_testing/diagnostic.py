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

"""C64-only guest marker and capture assertions; no hardware access."""

import math
import struct
import subprocess
from dataclasses import dataclass
from pathlib import Path


WIDTH, HEIGHT = 480, 360
SAMPLE_RATE = 8000
MAGIC = b"\xb6\x64"


def payload(run_id: int, counter: int, state: int) -> bytes:
    body = MAGIC + run_id.to_bytes(4, "big") + bytes((counter, state))
    return body + bytes((sum(body) & 255, 0xA5))


def listing(run_id: int) -> str:
    values = ",".join(str(value) for value in payload(run_id, 0, 0))
    return f"""10 rem bmc64 smoke run {run_id:08x}
20 poke 53280,0:poke 53281,0:print chr$(147)
30 dim d(9):for i=0 to 9:read d(i):next
40 for i=0 to 999:poke 1024+i,160:poke 55296+i,0:next
50 for y=3 to 20:for x=4 to 35:poke 55296+y*40+x,2:next:next
60 poke 54296,8:poke 54272,132:poke 54273,66
70 poke 54277,0:poke 54278,240:poke 54276,33
80 c=0:s=0:l=0:h=9:gosub 500
90 get a$:if a$=" " then s=1:poke 54276,32
100 c=c+1:if c>255 then c=0
110 d(6)=c:d(7)=s:v=0:for i=0 to 7:v=v+d(i):next
120 d(8)=v-256*int(v/256):l=6:h=8:gosub 500:goto 90
500 for b=l to h:v=d(b)
510 for k=0 to 7:p=int(v/2^(7-k)) and 1
520 n=b*8+k:y=int(n/10):x=n-y*10
530 a=55296+(4+y*2)*40+5+x*3
540 for j=0 to 1:for i=0 to 2:poke a+j*40+i,p:next:next
550 next:next:return
600 data {values}
"""


def build_guest(run_id: int, output: Path) -> Path:
    source = output / "diagnostic.bas"
    program = output / "diagnostic.prg"
    source.write_text(listing(run_id), encoding="ascii")
    completed = subprocess.run(
        ["petcat", "-w2", "-o", str(program), "--", str(source)],
        capture_output=True, text=True, timeout=10, check=False,
    )
    (output / "petcat.log").write_text(
        completed.stdout + completed.stderr, encoding="utf-8"
    )
    if completed.returncode:
        raise RuntimeError(f"petcat failed; see {output / 'petcat.log'}")
    if not program.read_bytes().startswith(b"\x01\x08"):
        raise RuntimeError("petcat did not produce a C64 BASIC PRG at $0801")
    return program


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
