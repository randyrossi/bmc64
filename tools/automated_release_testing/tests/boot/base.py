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

import argparse
import hashlib
import secrets
import time
from framework.case import BmcTest
from framework.errors import TestFailure
from framework.reporting import progress, save_json
from support.keyboard import wait_pico, type_token
from support.profiles import switch_profile, recover_api
from .screen import screen_bounds, wait_screen
from .layouts import Layout
from .screen_text import Font, ScreenText


class BootTypingTest(BmcTest):
    """Shared rig lifecycle and operations, not the test procedure."""

    suite = "boot"
    profile_required = True

    @classmethod
    def run_suite(cls, args: argparse.Namespace, tests: list[type[BmcTest]],
                  manifest: list[dict[str, str]]) -> tuple[list[dict], list[str], str | None]:
        from .session import main
        by_name = {test.name: test for test in tests}
        machines = {test.machine for test in tests}
        cases = [case for case in manifest if case["machine"] in machines]
        report = main(args, cases, by_name)
        blocked = report.get("control_error")
        if report.get("category") == "interrupted":
            blocked = report["error"]
        return report["cases"], [report["evidence"]], blocked

    def before_test(self) -> None:
        context = self.context
        assert context.rig is not None
        pico = context.rig.pico
        if context.rig.uart:
            context.register("check case UART", lambda: context.rig.uart.check(context.uart_start))
        context.register("release inputs", lambda: pico.command("RELEASE"))

    def prepare_screen_reader(self, layout: Layout, size: tuple[int, int],
                              token_length: int) -> None:
        context = self.context
        assert context.rig is not None
        api = context.rig.api
        report, case, machine = context.report, context.profile, self.machine
        report["stage"] = "font_rom"
        progress(f"{machine}: reading character ROM {layout.font_path} from the SD card")
        data = api.request("/api/fs/download", vol=case["volume"], path=layout.font_path)
        report["font"] = {"path": layout.font_path, "offset": layout.font_offset,
                          "bytes": len(data), "sha256": hashlib.sha256(data).hexdigest()}
        reader = ScreenText(machine, Font(data, layout.font_offset), *size, layout)
        token = "".join(secrets.choice(reader.alphabet) for _ in range(token_length))
        report.update(token=token, typing_method="rom_glyphs", typing_alphabet=reader.alphabet)
        self.reader, self.token, self.capture_size = reader, token, size

    def start_profile(self) -> None:
        context = self.context
        assert context.rig is not None
        report = context.report
        report["stage"] = "profile_switch"
        switch_profile(context.rig.api, context.profile, context.args.timeout,
                       context.health, context.output)
        report["checks"].append("profile_and_core")

    def open_native_capture(self) -> None:
        context = self.context
        assert context.rig is not None
        capture = context.rig.capture
        report, output, health = context.report, context.output, context.health
        timeout, machine, size = context.args.timeout, self.machine, self.capture_size
        report["stage"] = "capture_reopen"
        capture.open(output / "capture-1", size, health, timeout, machine)
        segments = [1]

        def reopen() -> None:
            segments.append(len(segments) + 1)
            capture.open(output / f"capture-{segments[-1]}", size, health, timeout, machine)

        def bounds(rgb: bytes) -> tuple[int, int, int, int]:
            return screen_bounds(rgb, *size)

        self.bounds, self.reopen, self.segments = bounds, reopen, segments

    def assert_boot_screen(self) -> None:
        context = self.context
        assert context.rig is not None
        capture = context.rig.capture
        report, output, health = context.report, context.output, context.health
        timeout = context.args.timeout
        reader, machine = self.reader, self.machine
        bounds, reopen = self.bounds, self.reopen
        report["stage"] = "basic_boot"
        progress(f"{machine}: checking the BASIC boot screen cell by cell against the ROM font")
        reading = wait_screen(capture, health, timeout, time.monotonic(), output, "boot",
                              lambda rgb: reader.read(rgb, bounds(rgb)), machine, reopen)
        report["screen_grid"] = reading.grid
        report["checks"].append("basic_ready")

    def wait_for_keyboard(self) -> None:
        context = self.context
        assert context.rig is not None
        pico, health, timeout = context.rig.pico, context.health, context.args.timeout
        report = context.report
        report["stage"] = "pico_remount"
        wait_pico(pico, health, timeout)
        pico.command("RELEASE")

    def type_usb_token(self) -> None:
        context = self.context
        assert context.rig is not None
        report, pico, health = context.report, context.rig.pico, context.health
        machine, token, reader = self.machine, self.token, self.reader
        report["stage"] = "typing"
        progress(f"{machine}: typing token {token!r} through USB (no Return); "
                 f"alphabet {reader.alphabet}")
        type_token(pico, token, health)

    def assert_typed_token(self) -> None:
        context = self.context
        assert context.rig is not None
        capture, health, timeout = context.rig.capture, context.health, context.args.timeout
        output, report = context.output, context.report
        reader, token, machine = self.reader, self.token, self.machine
        bounds, reopen, segments = self.bounds, self.reopen, self.segments
        reading = wait_screen(capture, health, timeout, time.monotonic(), output, "typing",
                              lambda rgb: reader.read(rgb, bounds(rgb), token), machine,
                              reopen if len(segments) == 1 else None)
        report["typing_glyph_distances"] = reading.distances
        report["checks"].append("usb_typing_visible")

    def assert_profile_unchanged(self) -> None:
        context = self.context
        assert context.rig is not None
        api, timeout, machine = context.rig.api, context.args.timeout, self.machine
        report, output, case, health = context.report, context.output, context.profile, context.health
        report["stage"] = "final_status"
        progress(f"{machine}: typing verified (bit distances {report['typing_glyph_distances']}); "
                 "checking final profile/core")
        final = recover_api(api, timeout, f"{machine} final profile/core status")
        save_json(output / "status-after.json", final)
        if (final["profile_id"] != case["id"] or final["machine"] != machine
                or final["profile_error"]):
            raise TestFailure("profile_changed", "profile/core changed during the typing assertion")
        health()
