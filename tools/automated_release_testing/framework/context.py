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

"""Per-test state and registered cleanup, separate from shared rig resources."""

import argparse
import sys
from contextlib import ExitStack
from dataclasses import dataclass, field
from pathlib import Path
from typing import Callable

from framework.errors import ERRORS
from framework.reporting import progress
from framework.rig import Rig


def no_health_check() -> None:
    pass


class Cleanups:
    """Reverse-order cleanup whose errors are recorded in a report, never raised."""

    def __init__(self, report: dict):
        self.report = report
        self.stack = ExitStack()

    def run(self, label: str, action: Callable[[], object]) -> None:
        progress(f"Cleanup: {label}")
        try:
            action()
        except ERRORS as error:
            if self.report["status"] == "PASS":
                self.report.update(error=f"{label}: {error}",
                                   category=getattr(error, "category", "cleanup"))
                if label == "after_test":
                    self.report["stage"] = "after_test"
            self.report["cleanup_errors"].append(f"{label}: {error}")
            self.report["status"] = "FAIL"
            print(f"{label}: {error}", file=sys.stderr, flush=True)

    def register(self, label: str, action: Callable[[], object]) -> None:
        self.stack.callback(self.run, label, action)

    def close(self) -> None:
        self.stack.close()


@dataclass
class TestContext:
    args: argparse.Namespace
    output: Path
    report: dict
    rig: Rig | None = None
    profile: dict[str, str] = field(default_factory=dict)
    health: Callable[[], None] = no_health_check
    uart_start: int = 0
    cleanups: Cleanups = field(init=False)

    def __post_init__(self) -> None:
        self.cleanups = Cleanups(self.report)

    def cleanup(self, label: str, action: Callable[[], object]) -> None:
        self.cleanups.run(label, action)

    def register(self, label: str, action: Callable[[], object]) -> None:
        self.cleanups.register(label, action)

    def close(self) -> None:
        self.cleanups.close()
