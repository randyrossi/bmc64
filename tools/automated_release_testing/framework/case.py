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

"""The lifecycle contract implemented by every discoverable hardware test."""

from abc import ABC, abstractmethod
from pathlib import Path

from framework.context import TestContext
from framework.reporting import progress, save_json
from framework.rig import Rig
from hardware.api import Api
from hardware.capture import SuiteCapture
from hardware.pico import Pico
from hardware.uart import UartLog
from support.keyboard import wait_pico
from support.profiles import status, switch_profile


class BmcTest(ABC):
    name: str
    description: str
    machine: str
    suite: str
    order = 100
    requirements: tuple[str, ...] = ()
    context: TestContext

    def prepare_test(self) -> None:
        """Optional offline preparation; must never access the rig."""

    def before_test(self) -> None:
        pass

    @abstractmethod
    def run_test(self) -> None:
        """Perform assertions; raise on failure rather than returning a status."""

    def after_test(self) -> None:
        pass

    def invoke(self, context: TestContext) -> None:
        self.context = context
        try:
            if not context.args.execute:
                self.prepare_test()
                context.report.update(status="PREPARED", stage="offline",
                                      detail="No hardware accessed")
                return
            try:
                self.before_test()
                self.run_test()
                # Provisional: a failing after_test() or cleanup revokes it.
                context.report["status"] = "PASS"
            finally:
                context.cleanup("after_test", self.after_test)
        finally:
            context.close()

    # Shortcuts to the context, so test steps read as what they do.

    @property
    def rig(self) -> Rig:
        if self.context.rig is None:
            raise RuntimeError("the rig is not available during offline preparation")
        return self.context.rig

    @property
    def api(self) -> Api:
        return self.rig.api

    @property
    def pico(self) -> Pico:
        return self.rig.pico

    @property
    def capture(self) -> SuiteCapture:
        return self.rig.capture

    @property
    def uart(self) -> UartLog | None:
        return self.rig.uart

    @property
    def report(self) -> dict:
        return self.context.report

    @property
    def output(self) -> Path:
        return self.context.output

    @property
    def timeout(self) -> float:
        return self.context.args.timeout

    @property
    def profile(self) -> dict[str, str]:
        return self.context.profile

    def health(self) -> None:
        self.context.health()

    def stage(self, name: str) -> None:
        self.report["stage"] = name

    def passed(self, check: str) -> None:
        self.report["checks"].append(check)

    # Steps shared by every suite.

    def start_profile(self, *, fresh: bool = True) -> None:
        """Boot this test's profile, verifying the profile ID and core.

        A fresh start always restarts BMC64, even if the profile is already
        running; otherwise a running profile with the right core is kept.
        """
        self.stage("profile_switch")
        current = status(self.api)
        if (not fresh and current["profile_id"] == self.profile["id"]
                and current["machine"] == self.machine):
            progress(f"{self.machine}: test profile {self.profile['id']} is already running")
            save_json(self.output / "status-before.json", current)
        else:
            switch_profile(self.api, self.profile, self.timeout, self.health, self.output)
        self.passed("profile_and_core")

    def open_capture(self, size: tuple[int, int], directory: str = "capture-1",
                     video_size: tuple[int, int] | None = None) -> None:
        """Reopen the capture into this test's evidence, locked to the current mode."""
        self.stage("capture_reopen")
        self.capture.open(self.output / directory, size, self.health, self.timeout,
                          self.machine, video_size)

    def wait_for_keyboard(self) -> None:
        """Wait for the Pico to remount as a USB keyboard after a restart."""
        self.stage("pico_remount")
        wait_pico(self.pico, self.health, self.timeout)
        self.pico.command("RELEASE")
