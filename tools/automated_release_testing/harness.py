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

"""Capture evidence and conservative UART classification shared by tests."""

import re
import time
from datetime import datetime
from pathlib import Path

from diagnostic import HEIGHT, WIDTH


def progress(message: str) -> None:
    print(f"[{datetime.now():%H:%M:%S}] {message}", flush=True)


class WaitProgress:
    def __init__(self, label: str, timeout: float):
        self.label = label
        self.started = time.monotonic()
        self.next_update = self.started + 5
        progress(f"{label}: waiting (timeout {timeout:g}s)")

    def update(self, detail: str, *, force: bool = False) -> None:
        now = time.monotonic()
        if force or now >= self.next_update:
            progress(f"{self.label}: {detail} ({now - self.started:.0f}s elapsed)")
            self.next_update = now + 5


class TestFailure(RuntimeError):
    def __init__(self, category: str, message: str):
        super().__init__(message)
        self.category = category


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


def classify_uart(line: str) -> dict[str, str] | None:
    # VICE probes PRGs as disk/tape/cartridge images before accepting them.
    probes = (
        r"Filesystem Image: Unknown disk image ",
        r"Tape: Error - Cannot open file ",
        r"Error - CRT header invalid\.",
        r"CART: .* is not a valid CRT file\.",
        r"Error - archdep_mkdir unimpl:",
    )
    if any(re.search(pattern, line) for pattern in probes):
        return {"severity": "info", "category": "autostart_probe", "line": line}
    critical = (
        ("panic", r"\bpanic\b"),
        ("exception", r"\b(?:data abort|prefetch abort|unhandled exception)\b"),
        ("assertion", r"\bassert(?:ion)?\s+(?:failed|failure)\b"),
        ("memory", r"\b(?:KASAN|out of memory|failed to allocate)\b"),
        ("profile", r"\bprofile\b.*\b(?:failed|error|fallback)\b"),
    )
    for category, pattern in critical:
        if re.search(pattern, line, re.IGNORECASE):
            return {"severity": "critical", "category": category, "line": line}
    if re.search(r"\b(?:error|failed|warning)\b", line, re.IGNORECASE):
        return {"severity": "warning", "category": "unclassified", "line": line}
    return None


def print_summary(results: list[dict]) -> None:
    """One line per test at the very end of a run, so results need no scrolling."""
    print("\nSummary:", flush=True)
    for result in results:
        line = f"  {result['status']:<8} {result['name']}"
        if result["status"] not in ("PASS", "PREPARED") and result.get("error"):
            stage = f" [{result['stage']}]" if result.get("stage") else ""
            line += f"{stage}: {result['error']}"
        print(line, flush=True)
    counts = {status: sum(result["status"] == status for result in results)
              for status in ("PASS", "PREPARED", "FAIL", "NOT_RUN")}
    print("  " + ", ".join(f"{count} {status}" for status, count in counts.items() if count),
          flush=True)
