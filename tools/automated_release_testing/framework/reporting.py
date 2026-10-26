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
from datetime import datetime
from pathlib import Path


def save_json(path: Path, value: object) -> None:
    path.write_text(json.dumps(value, indent=2) + "\n", encoding="utf-8")

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
