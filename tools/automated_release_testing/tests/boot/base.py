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

import hashlib
import secrets
import time
from framework.case import BmcTest
from framework.errors import TestFailure
from framework.reporting import progress, save_json
from support.keyboard import type_token
from support.profiles import recover_api
from .screen import wait_screen
from .layouts import Layout
from .screen_text import Font, Reading, ScreenText


class BootTypingTest(BmcTest):
    """Boot-suite steps; each test file calls them in its own order."""

    suite = "boot"

    def prepare_screen_reader(self, layout: Layout, size: tuple[int, int],
                              token_length: int) -> None:
        self.stage("font_rom")
        progress(f"{self.machine}: reading character ROM {layout.font_path} from the SD card")
        data = self.api.request("/api/fs/download", vol=self.profile["volume"],
                                path=layout.font_path)
        self.report["font"] = {"path": layout.font_path, "offset": layout.font_offset,
                               "bytes": len(data), "sha256": hashlib.sha256(data).hexdigest()}
        self.reader = ScreenText(self.machine, Font(data, layout.font_offset), *size, layout)
        self.token = "".join(secrets.choice(self.reader.alphabet) for _ in range(token_length))
        self.report.update(token=self.token, typing_method="rom_glyphs",
                           typing_alphabet=self.reader.alphabet)
        self.capture_size = size

    def open_native_capture(self) -> None:
        self.segments = 1
        self.open_capture(self.capture_size)

    def reopen_capture(self) -> None:
        """Called once by a screen wait that sees only unreadable frames."""
        self.segments += 1
        # Not open_capture(): the failing stage stays the screen wait's.
        self.capture.open(self.output / f"capture-{self.segments}", self.capture_size,
                          self.health, self.timeout, self.machine)

    def read_screen(self, rgb: bytes, typed: str = "") -> Reading:
        return self.reader.read(rgb, typed)

    def assert_boot_screen(self) -> None:
        self.stage("basic_boot")
        progress(f"{self.machine}: checking the BASIC boot screen cell by cell against the ROM font")
        reading = wait_screen(self.capture, self.health, self.timeout, time.monotonic(),
                              self.output, "boot", self.read_screen, self.machine,
                              self.reopen_capture)
        self.report["screen_grid"] = reading.grid
        self.passed("basic_ready")

    def type_usb_token(self) -> None:
        self.stage("typing")
        progress(f"{self.machine}: typing token {self.token!r} through USB (no Return); "
                 f"alphabet {self.reader.alphabet}")
        type_token(self.pico, self.token, self.health)

    def assert_typed_token(self) -> None:
        # The capture is reopened at most once per test, in either wait.
        reading = wait_screen(self.capture, self.health, self.timeout, time.monotonic(),
                              self.output, "typing",
                              lambda rgb: self.read_screen(rgb, self.token), self.machine,
                              self.reopen_capture if self.segments == 1 else None)
        self.report["typing_glyph_distances"] = reading.distances
        self.passed("usb_typing_visible")

    def assert_profile_unchanged(self) -> None:
        self.stage("final_status")
        progress(f"{self.machine}: typing verified (bit distances "
                 f"{self.report['typing_glyph_distances']}); checking final profile/core")
        final = recover_api(self.api, self.timeout, f"{self.machine} final profile/core status")
        save_json(self.output / "status-after.json", final)
        if (final["profile_id"] != self.profile["id"] or final["machine"] != self.machine
                or final["profile_error"]):
            raise TestFailure("profile_changed", "profile/core changed during the typing assertion")
        self.health()
