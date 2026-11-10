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

import json
import time
from pathlib import Path
from typing import Protocol
from support.constants import HEIGHT, WIDTH
from framework.reporting import save_json


class FrameSource(Protocol):
    @property
    def latest_frame(self) -> tuple[float, bytes] | None: ...

    @property
    def width(self) -> int: ...

    @property
    def height(self) -> int: ...


def save_frame(path: Path, rgb: bytes, width: int = WIDTH, height: int = HEIGHT) -> None:
    if len(rgb) != width * height * 3:
        raise ValueError("incorrect RGB frame size")
    path.write_bytes(f"P6\n{width} {height}\n255\n".encode() + rgb)


def frame_health(rgb: bytes, width: int = WIDTH, height: int = HEIGHT) -> dict[str, object]:
    if len(rgb) != width * height * 3:
        raise ValueError("incorrect RGB frame size")
    channels = [rgb[channel::291] for channel in range(3)]
    low = min(min(samples) for samples in channels)
    high = max(max(samples) for samples in channels)
    return {
        "minimum": low, "maximum": high,
        "black": high < 25,
        "flat": max(max(samples) - min(samples) for samples in channels) < 12,
    }

def failure_evidence(capture: FrameSource, output: Path, report: dict) -> None:
    latest = capture.latest_frame
    if latest:
        failed = report["status"] != "PASS" or report.get("cleanup_errors")
        save_frame(output / ("failure.ppm" if failed else "last-frame.ppm"),
                   latest[1], capture.width, capture.height)
        save_json(output / "capture-health.json", {
            "frame_age_seconds": time.monotonic() - latest[0],
            **frame_health(latest[1], capture.width, capture.height),
        })
    else:
        save_json(output / "capture-health.json", {"video": "no frames received"})
