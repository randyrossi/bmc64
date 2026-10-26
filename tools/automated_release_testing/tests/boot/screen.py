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
from typing import Callable
from framework.errors import TestFailure
from framework.reporting import WaitProgress
from hardware.capture import SuiteCapture
from support.evidence import frame_health, save_frame
from .screen_text import Reading


def screen_bounds(rgb: bytes, width: int, height: int) -> tuple[int, int, int, int]:
    def colour(x: int, y: int) -> tuple[int, ...]:
        return tuple(max(rgb[(row * width + x) * 3 + channel]
                         for row in range(max(0, y - 2), min(height, y + 3)))
                     for channel in range(3))

    background = colour(width // 2, height // 2)

    def matches(x: int, y: int) -> bool:
        return max(abs(a - b) for a, b in zip(colour(x, y), background)) <= 35

    left, right = width // 2, width // 2
    while left > 0 and matches(left - 1, height // 2):
        left -= 1
    while right + 1 < width and matches(right + 1, height // 2):
        right += 1
    columns = [left + (right - left) * index // 10 for index in range(1, 10)]
    rows = [row for row in range(height) if sum(matches(x, row) for x in columns) >= 5]
    if right - left < width // 2 or not rows or rows[-1] - rows[0] < height // 2:
        return 0, 0, width, height
    if left == 0 and right == width - 1 and rows[0] == 0 and rows[-1] == height - 1:
        return 0, 0, width, height
    # Retain antialiased edge pixels, but exclude the vertical sampling halo.
    left, right = max(0, left - 1), min(width - 1, right + 1)
    return left, rows[0] + 2, right - left + 1, rows[-1] - rows[0] - 3


def wait_screen(capture: SuiteCapture, health: Callable[[], None], timeout: float,
                after: float, output: Path, stage: str, read: Callable[[bytes], Reading],
                machine: str, reopen: Callable[[], None] | None = None) -> Reading:
    """Wait for two fresh frames whose every checked cell matches."""
    deadline = time.monotonic() + timeout
    seen, matches, unreadable, category = after, 0, 0, "capture_no_frames"
    last: Reading | None = None
    updates = WaitProgress(f"{machine} {stage} screen", timeout)
    detail = "waiting for a fresh captured frame"
    with (output / f"{stage}-frames.jsonl").open("w", encoding="utf-8") as log:
        while time.monotonic() < deadline:
            updates.update(f"{detail}; matching frames {matches}/2")
            health()
            latest = capture.latest_frame
            if not latest or latest[0] <= seen:
                detail = "no new captured frame"
                time.sleep(0.1)
                continue
            timestamp, rgb = latest
            seen = timestamp
            if time.monotonic() - timestamp > 3:
                category = "capture_stalled"
                detail = "latest captured frame is stale"
                continue
            metrics = frame_health(rgb, capture.width, capture.height)
            if metrics["black"] or metrics["flat"]:
                category = "capture_black" if metrics["black"] else "capture_flat"
                detail = "capture is black" if metrics["black"] else "capture is flat/uniform"
                matches = 0
                time.sleep(0.3)
                continue
            save_frame(output / f"{stage}-last.ppm", rgb, capture.width, capture.height)
            last = read(rgb)
            log.write(json.dumps({
                "monotonic": timestamp, "matched": last.matched, "unreadable": last.unreadable,
                "grid": last.grid, "mismatch_count": len(last.mismatches),
                "mismatches": last.mismatches[:12], "typed_distances": last.distances,
            }) + "\n")
            log.flush()
            health()
            if time.monotonic() >= deadline:
                break
            detail = last.summary()
            if last.matched:
                matches += 1
                if matches >= 2:
                    save_frame(output / f"{stage}.ppm", rgb, capture.width, capture.height)
                    updates.update("verified on two fresh frames", force=True)
                    return last
                time.sleep(0.1)
                continue
            matches = 0
            category = ("screen_unreadable" if last.unreadable
                        else "guest_not_ready" if stage == "boot" else "typing_not_visible")
            unreadable = unreadable + 1 if last.unreadable else 0
            if unreadable >= 3 and reopen:
                updates.update("banner unreadable on three frames; reopening capture", force=True)
                reopen()
                reopen, unreadable, seen = None, 0, time.monotonic()
            time.sleep(0.1)
    if capture.latest_frame and time.monotonic() - capture.latest_frame[0] > 3:
        category = "capture_stalled"
    message = f"{stage}: no two fresh matching screens within {timeout:g}s"
    raise TestFailure(category, message + (f"; last frame: {last.summary()}" if last else ""))
