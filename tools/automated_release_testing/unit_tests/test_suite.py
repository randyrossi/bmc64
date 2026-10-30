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

"""Offline profile rotation, screen matching, failure-evidence and UART regression tests.

Tests against recorded frames need the real character ROMs, which are not part
of this repository. Set BMC64_ROM_ROOT to a copy of the SD card (or a folder
with the same C64/chargen, PLUS4/kernal, ... layout) to run them.
"""

import argparse
import io
import json
import os
import random
import subprocess
import tempfile
import threading
import time
import unittest
from pathlib import Path
ROOT = Path(__file__).resolve().parents[1]
from unittest.mock import patch
from urllib.error import HTTPError, URLError

from tests.basic.diagnostic import HEIGHT, WIDTH
from framework.errors import TestFailure
from framework.reporting import WaitProgress, print_summary
from support.evidence import frame_health
from support.uart_findings import classify_uart
from support.constants import DEFAULT_CAPTURE_SIZE, MACHINES
from framework.discovery import discover
from hardware.capture import SuiteCapture, Capture as HardwareCapture, capture_command
from hardware.pico import Pico
from hardware.uart import UartLog
from support.evidence import failure_evidence
from support.keyboard import type_token, wait_pico
from support.profiles import (
    read_manifest, recover_api, resolve_profiles, switch_profile, profile_values,
)
from tests.boot.screen import wait_screen
from framework.rig import Rig
from framework.runner import run, run_tests
from utils.runner import parser
from tests.boot.screen_text import LETTERS, Font, Geometry, Reading, ScreenText


BOOT_TESTS = {test.machine: test for test in discover() if test.suite == "boot"}


def rotate(api, pico, capture, uart, cases, timeout, output, report):
    """Run the boot tests on a fake rig, as run() does after preflight."""
    profiles = {case["machine"]: case for case in cases}
    tests = [test for test in BOOT_TESTS.values() if test.machine in profiles]
    report["cases"] = [{**profiles[test.machine], "name": test.name, "status": "NOT_RUN",
                        "checks": [], "cleanup_errors": []} for test in tests]
    args = argparse.Namespace(timeout=timeout, execute=True)
    run_tests(args, Rig(api, pico, capture, uart), tests, report["cases"], profiles,
              output, report)


def boot_run(args, manifest):
    return run(args, list(BOOT_TESTS.values()), manifest)
LAYOUTS = {machine: test.layout for machine, test in BOOT_TESTS.items()}
CAPTURE_SIZES = {machine: test.capture_size for machine, test in BOOT_TESTS.items()
                 if test.capture_size != DEFAULT_CAPTURE_SIZE}


SIZE = WIDTH * HEIGHT * 3
FRAME = bytes([70]) * (SIZE // 2) + bytes([180]) * (SIZE // 2)
SCREENS = ROOT.joinpath("test_data") / "screens"
ROM_ROOT = os.environ.get("BMC64_ROM_ROOT")

# Border, background and ink as seen through the capture for each default screen.
COLOURS = {
    "C64": ((112, 96, 220), (52, 40, 200), (150, 140, 255)),
    "C128": ((80, 220, 80), (80, 80, 80), (160, 255, 120)),
    "VIC20": ((40, 200, 210), (232, 232, 232), (48, 48, 170)),
    "Plus4": ((190, 170, 220), (226, 226, 226), (10, 10, 10)),
    "Plus4Emu": ((190, 170, 220), (226, 226, 226), (10, 10, 10)),
    "Pet": ((8, 8, 8), (8, 8, 8), (40, 240, 40)),
}


def synthetic_font(seed: int = 7) -> bytes:
    """Random but distinct glyphs: exercises the matcher without ROM data."""
    rng = random.Random(seed)
    data = bytearray()
    for code in range(128):
        data += bytes(8) if code == 32 else bytes(
            [rng.randrange(256) & 0x7E for _ in range(7)] + [0])
    return bytes(data)


def screen_geometry(machine: str, width: int, height: int) -> Geometry:
    layout = LAYOUTS[machine]
    if machine == "Pet":
        # As recorded from the 3016: the text sits a little above the centred viewport.
        pixel = height / 270
        return Geometry((width - 320 * pixel) / 2 - 2, (height - 200 * pixel) / 2 - 31, pixel, pixel)
    if machine == "VIC20":
        return Geometry(112.0, 84.0, 6.0, 3.0)
    return Geometry(160.0, 60.0, 3.0, 3.0)


def render(machine: str, font: Font, lines: dict[int, str], *, width: int | None = None,
           height: int | None = None, shift: tuple[int, int] = (0, 0),
           cursor: tuple[int, int] | None = None, scanlines: bool = True,
           soft_scanlines: bool = False) -> bytes:
    """Draw a text screen as the capture shows it: border, pane, glyphs, scanlines."""
    width, height = width or 1280, height or 720
    layout = LAYOUTS[machine]
    geometry = screen_geometry(machine, width, height)
    border, background, ink = (bytes(colour) for colour in COLOURS[machine])
    pane_left, pane_top = int(geometry.left + shift[0]), int(geometry.top + shift[1])
    pane_right = pane_left + int(layout.columns * 8 * geometry.pixel_width)
    pane_bottom = pane_top + int(layout.rows * 8 * geometry.pixel_height)
    rows = []
    for y in range(height):
        row = bytearray(border * width)
        if pane_top <= y < pane_bottom:
            row[pane_left * 3:pane_right * 3] = background * (pane_right - pane_left)
        rows.append(row)
    cells = {(column, row): font.glyph(char) for row, text in lines.items()
             for column, char in enumerate(text) if char != " "}
    if cursor:
        cells[cursor] = (1 << 64) - 1
    left, top = geometry.left + shift[0], geometry.top + shift[1]
    for (column, row), glyph in cells.items():
        for gy in range(8):
            y0 = int(top + (row * 8 + gy) * geometry.pixel_height)
            y1 = int(top + (row * 8 + gy + 1) * geometry.pixel_height)
            for gx in range(8):
                if glyph >> (gy * 8 + gx) & 1:
                    x0 = int(left + (column * 8 + gx) * geometry.pixel_width)
                    x1 = int(left + (column * 8 + gx + 1) * geometry.pixel_width)
                    for y in range(y0, y1):
                        rows[y][x0 * 3:x1 * 3] = ink * (x1 - x0)
    if soft_scanlines:
        # As the Pi 4 draws them: one bright row between two dim ones.
        dim = bytes(value * 65 // 100 for value in range(256))
        for y in range(height):
            if y % 3 != 1:
                rows[y] = rows[y].translate(dim)
    elif scanlines and machine != "Pet":
        dark = bytes(value * 3 // 10 for value in range(256))
        for y in range(2, height, 3):
            rows[y] = rows[y].translate(dark)
    return b"".join(bytes(row) for row in rows)


def boot_lines(machine: str, typed: str = "") -> dict[int, str]:
    lines = dict(LAYOUTS[machine].screen)
    if typed:
        lines[LAYOUTS[machine].input_row] = typed
    return lines


def read_png(path: Path) -> tuple[bytes, int, int]:
    size = subprocess.run(
        ["ffprobe", "-v", "error", "-show_entries", "stream=width,height", "-of", "csv=p=0",
         str(path)], check=True, capture_output=True, text=True, timeout=10).stdout
    width, height = (int(value) for value in size.strip().split(","))
    rgb = subprocess.run(
        ["ffmpeg", "-v", "error", "-i", str(path), "-frames:v", "1", "-pix_fmt", "rgb24",
         "-f", "rawvideo", "pipe:1"], check=True, capture_output=True, timeout=10).stdout
    return rgb, width, height


class Clock:
    def __init__(self):
        self.now = 100.0

    def monotonic(self):
        return self.now

    def sleep(self, seconds):
        self.now += seconds


class ProgressTests(unittest.TestCase):
    def test_wait_updates_are_flushed_throttled_and_show_elapsed_time(self):
        clock = Clock()
        with patch("framework.reporting.time.monotonic", clock.monotonic), \
                patch("builtins.print") as output:
            updates = WaitProgress("C64 typing", 60)
            updates.update("no match")
            clock.sleep(4.9)
            updates.update("no match")
            self.assertEqual(output.call_count, 1)
            clock.sleep(0.1)
            updates.update("character shapes do not match yet")
            updates.update("verified", force=True)
        messages = [call.args[0] for call in output.call_args_list]
        self.assertIn("timeout 60s", messages[0])
        self.assertIn("character shapes do not match yet (5s elapsed)", messages[1])
        self.assertIn("verified (5s elapsed)", messages[2])
        self.assertTrue(all(call.kwargs == {"flush": True} for call in output.call_args_list))


class Capture:
    """Each health check delivers the current frame as a fresh capture."""

    def __init__(self, clock, frame=FRAME, width=WIDTH, height=HEIGHT):
        self.width, self.height = width, height
        self.clock, self.frame = clock, frame
        self.latest_frame = None
        self.opened = []

    def check(self):
        self.clock.now += 0.05
        self.latest_frame = (self.clock.now, self.frame)

    def open(self, output, size, health, timeout, label, video_size=None):
        self.opened.append((output.name, size))
        output.mkdir(exist_ok=True)


def api_status(identity="main", machine="C64", uptime=100, error=""):
    return {"profile_id": identity, "machine": machine, "uptime_secs": uptime,
            "profile_error": error}


class Profiles:
    def __init__(self, suffix=""):
        self.suffix = suffix
        self.files = {
            name.lower() + "-test" + suffix:
                f"name={name.lower()}_test\nmachine={name}/PAL/HDMI\n".encode()
            for name in MACHINES
        }
        self.fonts = {LAYOUTS[name].font_path: bytes(LAYOUTS[name].font_offset) + synthetic_font()
                      for name in MACHINES}
        self.current = api_status()
        self.starts = []
        self.reboots = 0
        self.outage = False

    def get_json(self, endpoint, **query):
        if self.outage:
            raise URLError("network unavailable")
        if endpoint == "/api/status":
            return self.current.copy()
        if endpoint == "/api/fs/list":
            return {"vol": "SD:", "entries": [
                {"name": identity, "dir": True} for identity in self.files
            ], "truncated": False}
        raise AssertionError(endpoint)

    def request(self, endpoint, data=None, **query):
        if endpoint == "/api/fs/download":
            if query["path"].startswith("/profiles/"):
                return self.files[query["path"].split("/")[2]]
            return self.fonts[query["path"]]
        if endpoint == "/api/reboot":
            self.reboots += 1
            self.current = api_status(
                self.current["profile_id"], self.current["machine"], uptime=1)
            return b'{"ok":true}'
        if endpoint == "/api/profiles/start":
            self.starts.append(query)
            identity = query["id"]
            machine = next(name for name in MACHINES
                           if name.lower() + "-test" + self.suffix == identity)
            self.current = api_status(identity, machine, 1)
            return b'{"ok":true}'
        raise AssertionError(endpoint)


class ManifestTests(unittest.TestCase):
    def test_supplied_profiles_match_suite_and_only_override_network(self):
        from support.profiles import profile_values
        root = ROOT.joinpath("sdcard_profiles") / "profiles"
        cases = read_manifest(ROOT.joinpath("suite_profiles.json"))
        self.assertEqual(len(list(root.iterdir())), 6)
        api = Profiles()
        api.files = {}
        for case in cases:
            identity = case["profile"]
            self.assertTrue(identity.endswith("-de56af"))
            directory = root / identity
            profile = (directory / "profile.txt").read_bytes()
            self.assertEqual(profile_values(profile), {
                "name": case["machine"].lower() + "_test",
                "machine": case["machine"] + "/PAL/HDMI",
                "start": "once",
            })
            settings = directory / f"settings-{case['machine'].lower()}.txt"
            self.assertEqual(profile_values(settings.read_bytes()), {
                "network_device": "2", "webui_enabled": "1",
            })
            files = {"profile.txt", settings.name}
            if case["machine"] == "Pet":
                # 40-column 3016: 80-column glyphs are finer than the capture device resolves.
                files.add("vice.ini")
                self.assertEqual(profile_values((directory / "vice.ini").read_bytes()[6:]), {
                    "KeymapSymFile": '"rpi_grus_sym.vkm"', "KeyboardType": "4",
                    "KernalName": '"kernal2"', "EditorName": '"edit2g"', "BasicName": '"basic2"',
                    "RamSize": "16", "IOSize": "2048", "Crtc": "0", "VideoSize": "40",
                })
            self.assertEqual({file.name for file in directory.iterdir()}, files)
            api.files[identity] = profile
        self.assertEqual(len(resolve_profiles(api, cases)), 6)

    def test_default_manifest_has_all_six_distinct_cores(self):
        cases = read_manifest(ROOT.joinpath("suite_profiles.json"))
        self.assertEqual({case["machine"] for case in cases}, set(MACHINES))
        resolved = resolve_profiles(Profiles("-de56af"), cases)
        self.assertEqual(len(resolved), 6)
        self.assertEqual(resolved[0]["id"], "c64-test-de56af")

    def test_ids_work_without_guessing_from_display_names(self):
        api = Profiles()
        api.files["c64-test"] = b"name=Renamed display\nmachine=C64/PAL/HDMI\n"
        result = resolve_profiles(api, [{"machine": "C64", "profile": "c64-test"}])
        self.assertEqual(result[0]["name"], "Renamed display")
        with self.assertRaisesRegex(TestFailure, "found 0"):
            resolve_profiles(api, [{"machine": "C64", "profile": "c64_test"}])

    def test_missing_ambiguous_wrong_core_and_attached_media_fail_before_switch(self):
        for failure in ("missing", "ambiguous", "core", "autostart", "disk"):
            api = Profiles()
            if failure == "missing":
                del api.files["c64-test"]
            elif failure == "ambiguous":
                api.files["duplicate"] = api.files["c64-test"]
            elif failure == "core":
                api.files["c64-test"] = b"name=c64_test\nmachine=Plus4Emu/PAL/HDMI\n"
            else:
                api.files["c64-test"] += (
                    b"autostart=SD:/game.prg\n" if failure == "autostart" else b"disk_8=SD:/game.d64\n")
            with self.subTest(failure=failure), self.assertRaises(TestFailure):
                resolve_profiles(api, [{"machine": "C64", "profile": "c64_test"}])
            self.assertEqual(api.starts, [])

    def test_malformed_and_duplicate_manifest_cases_rejected(self):
        with tempfile.TemporaryDirectory() as directory:
            path = Path(directory) / "manifest.json"
            for value in ({}, [], [{"machine": "C64"}],
                          [{"machine": "C64", "profile": "c64_test"}] * 2,
                          [{"machine": "bogus", "profile": "test"}]):
                path.write_text(json.dumps(value))
                with self.subTest(value=value), self.assertRaises(ValueError):
                    read_manifest(path)

    def test_truncated_listing_fails_closed(self):
        api = Profiles()
        with patch.object(api, "get_json", return_value={
                "entries": [], "vol": "SD:", "truncated": True}):
            with self.assertRaisesRegex(TestFailure, "truncated"):
                resolve_profiles(api, [])


class SwitchTests(unittest.TestCase):
    def run_switch(self, states):
        api = Profiles()
        clock = Clock()
        iterator = iter(states)
        last = [states[-1]]

        def response(endpoint, **query):
            current = next(iterator, last[0])
            if isinstance(current, Exception):
                raise current
            return current

        with tempfile.TemporaryDirectory() as directory, \
                patch.object(api, "get_json", side_effect=response), \
                patch("tests.boot.screen.time.monotonic", clock.monotonic), \
                patch("tests.boot.screen.time.sleep", clock.sleep):
            return switch_profile(api, {"id": "c64-test", "machine": "C64"},
                                  2, lambda: None, Path(directory))

    def test_acceptance_is_not_boot_and_stale_same_profile_is_rejected(self):
        with self.assertRaisesRegex(TestFailure, "no verified switch"):
            self.run_switch([api_status("c64-test"), api_status("c64-test", uptime=101)])

    def test_same_profile_requires_verified_new_boot_not_just_http_outage(self):
        self.assertEqual(self.run_switch([
            api_status("c64-test"), api_status("c64-test", uptime=1),
        ])["uptime_secs"], 1)
        with self.assertRaises(TestFailure):
            self.run_switch([
                api_status("c64-test"), URLError("network interruption"),
                api_status("c64-test", uptime=101),
            ])
        self.assertEqual(self.run_switch([
            api_status("c64-test", uptime=0), URLError("reboot"),
            api_status("c64-test", uptime=0),
        ])["profile_id"], "c64-test")

    def test_active_profile_uses_one_reboot_not_start_once(self):
        for lost_reply in (False, True):
            with self.subTest(lost_reply=lost_reply), tempfile.TemporaryDirectory() as directory:
                api, clock = Profiles(), Clock()
                api.current = api_status("c64-test", uptime=439, error="Already running")
                original = api.request

                def request(endpoint, data=None, **query):
                    reply = original(endpoint, data, **query)
                    if lost_reply:
                        raise TimeoutError("reboot reply lost")
                    return reply

                with patch.object(api, "request", side_effect=request) as requests, \
                        patch("tests.boot.screen.time.monotonic", clock.monotonic), \
                        patch("tests.boot.screen.time.sleep", clock.sleep), \
                        patch("sys.stderr", new_callable=io.StringIO):
                    current = switch_profile(
                        api, {"id": "c64-test", "machine": "C64"}, 2, lambda: None,
                        Path(directory))
                requests.assert_called_once_with("/api/reboot", b"")
                self.assertEqual(api.starts, [])
                self.assertEqual(api.reboots, 1)
                self.assertEqual(current["profile_id"], "c64-test")
                self.assertEqual(current["uptime_secs"], 1)
                saved = json.loads((Path(directory) / "switch-request.json").read_text())
                self.assertEqual(saved["endpoint"], "/api/reboot")
                self.assertEqual(saved["outcome"], "unknown" if lost_reply else "accepted")

    def test_reboot_ignores_only_unchanged_preboot_error(self):
        self.assertEqual(self.run_switch([
            api_status("c64-test", error="Already running"),
            api_status("c64-test", uptime=101, error="Already running"),
            api_status("c64-test", uptime=1),
        ])["uptime_secs"], 1)
        for current in (
            api_status("c64-test", uptime=101, error="new failure"),
            api_status("c64-test", uptime=1, error="Already running"),
        ):
            with self.subTest(current=current), self.assertRaises(TestFailure):
                self.run_switch([api_status("c64-test", error="Already running"), current])

    def test_different_id_is_verified_even_if_short_reboot_was_not_observed(self):
        self.assertEqual(self.run_switch([
            api_status(), api_status("c64-test"),
        ])["profile_id"], "c64-test")

    def test_profile_error_and_wrong_core_cannot_pass(self):
        for current in (api_status("c64-test", error="missing ROM"),
                        api_status("c64-test", machine="Plus4Emu")):
            with self.subTest(current=current), self.assertRaises(TestFailure):
                self.run_switch([api_status(), current])

    def test_authentication_failure_is_not_hidden_as_reboot_outage(self):
        with self.assertRaises(HTTPError):
            self.run_switch([
                api_status(), HTTPError("http://test", 401, "Unauthorized", {}, None),
            ])

    def test_lost_start_reply_is_verified_without_retrying_the_request(self):
        for applied in (True, False):
            with self.subTest(applied=applied), tempfile.TemporaryDirectory() as directory:
                api, clock = Profiles(), Clock()
                original = api.request

                def request(endpoint, data=None, **query):
                    if applied:
                        original(endpoint, data, **query)
                    else:
                        api.starts.append(query)
                    raise TimeoutError("timed out waiting for start reply")

                with patch.object(api, "request", side_effect=request), \
                        patch("tests.boot.screen.time.monotonic", clock.monotonic), \
                        patch("tests.boot.screen.time.sleep", clock.sleep), \
                        patch("sys.stderr", new_callable=io.StringIO):
                    if applied:
                        value = switch_profile(api, {"id": "c64-test", "machine": "C64"},
                                               2, lambda: None, Path(directory))
                        self.assertEqual(value["profile_id"], "c64-test")
                    else:
                        with self.assertRaises(TestFailure) as failure:
                            switch_profile(api, {"id": "c64-test", "machine": "C64"},
                                           2, lambda: None, Path(directory))
                        self.assertEqual(failure.exception.category, "profile_switch_timeout")
                self.assertEqual(len(api.starts), 1)
                saved = json.loads((Path(directory) / "switch-request.json").read_text())
                self.assertEqual(saved["outcome"], "unknown")
                self.assertFalse(saved["retried"])

    def test_explicit_http_rejection_is_not_an_uncertain_success(self):
        api = Profiles()
        with tempfile.TemporaryDirectory() as directory, \
                patch.object(api, "request", side_effect=HTTPError(
                    "http://test", 503, "Unavailable", {}, None)), \
                self.assertRaises(HTTPError):
            switch_profile(api, {"id": "c64-test", "machine": "C64"},
                           2, lambda: None, Path(directory))

    def test_unavailable_control_times_out_explicitly(self):
        clock = Clock()
        api = Profiles()
        api.outage = True
        with patch("tests.boot.screen.time.monotonic", clock.monotonic), \
                patch("tests.boot.screen.time.sleep", clock.sleep), \
                self.assertRaisesRegex(TestFailure, "HTTP control did not return"):
            recover_api(api, 1)


class ScreenTextTests(unittest.TestCase):
    """Matcher behaviour on screens drawn with a synthetic font."""

    @classmethod
    def setUpClass(cls):
        cls.font = Font(synthetic_font(), 0)
        cls.readers, cls.sizes, cls.tokens, cls.typed = {}, {}, {}, {}
        rng = random.Random(5)
        for machine in MACHINES:
            width, height = CAPTURE_SIZES.get(machine, DEFAULT_CAPTURE_SIZE)
            reader = ScreenText(machine, cls.font, width, height, LAYOUTS[machine])
            token = "".join(rng.choice(reader.alphabet) for _ in range(8))
            frame = render(machine, cls.font, boot_lines(machine, token), width=width,
                           height=height, cursor=(8, LAYOUTS[machine].input_row))
            cls.readers[machine], cls.sizes[machine], cls.tokens[machine] = reader, (width, height), token
            cls.typed[machine] = cls.fitted(reader, frame, width, height)

    @staticmethod
    def fitted(reader, frame, width, height):
        plane = reader.plane(frame)
        return plane, reader.fit(plane)

    def read(self, machine, lines, typed="", **options):
        width, height = self.sizes[machine]
        frame = render(machine, self.font, lines, width=width, height=height, **options)
        return self.readers[machine].read(frame, typed)

    def test_every_core_boot_screen_matches_with_blank_or_solid_cursor(self):
        for machine in MACHINES:
            for cursor in (None, (0, LAYOUTS[machine].input_row)):
                with self.subTest(machine=machine, cursor=cursor):
                    reading = self.read(machine, boot_lines(machine), cursor=cursor)
                    self.assertTrue(reading.matched, reading.summary())

    def test_grid_is_found_when_the_text_sits_off_centre(self):
        for machine in ("C64", "VIC20", "Pet"):
            for shift in ((3, -4), (-5, 6)):
                with self.subTest(machine=machine, shift=shift):
                    reading = self.read(machine, boot_lines(machine), shift=shift)
                    self.assertTrue(reading.matched, reading.summary())

    def test_pi_4_style_soft_scanlines_read_the_same(self):
        for machine in ("C64", "C128", "VIC20", "Plus4", "Plus4Emu"):
            with self.subTest(machine=machine):
                reading = self.read(machine, boot_lines(machine), soft_scanlines=True)
                self.assertTrue(reading.matched, reading.summary())

    def test_typed_token_matches_and_every_single_letter_substitution_fails(self):
        for machine in MACHINES:
            reader, token = self.readers[machine], self.tokens[machine]
            plane, fitted = self.typed[machine]
            with self.subTest(machine=machine):
                reading = reader.compare(plane, fitted, token)
                self.assertTrue(reading.matched, reading.summary())
                self.assertEqual(reading.distances, [0] * 8)
            for column in range(8):
                for letter in LETTERS:
                    if letter != token[column]:
                        expected = token[:column] + letter + token[column + 1:]
                        with self.subTest(machine=machine, expected=expected):
                            self.assertFalse(reader.compare(plane, fitted, expected).matched)

    def test_missing_partial_reordered_and_extra_input_fail(self):
        for machine in MACHINES:
            reader, token = self.readers[machine], self.tokens[machine]
            plane, fitted = self.typed[machine]
            with self.subTest(machine=machine):
                self.assertFalse(reader.compare(plane, fitted, "").matched)
                self.assertFalse(reader.compare(plane, fitted, token[:-1]).matched)
                if token[::-1] != token:
                    self.assertFalse(reader.compare(plane, fitted, token[::-1]).matched)
                self.assertFalse(reader.compare(plane, fitted, token + token[0]).matched)

    def test_token_on_wrong_row_and_changed_banner_fail(self):
        machine = "C64"
        token = self.tokens[machine]
        lines = boot_lines(machine)
        lines[LAYOUTS[machine].input_row + 1] = token
        self.assertFalse(self.read(machine, lines, token).matched)
        lines = boot_lines(machine)
        lines[3] = lines[3].replace("38911", "38912")
        reading = self.read(machine, lines)
        self.assertFalse(reading.matched)
        self.assertFalse(reading.unreadable)
        self.assertIn("row 3", reading.summary())

    def test_corrupt_frames_are_unreadable_not_merely_mismatched(self):
        machine = "C64"
        width, height = self.sizes[machine]
        good = render(machine, self.font, boot_lines(machine), width=width, height=height)
        rng = random.Random(3)
        row_bytes = width * 3
        # Horizontal smearing: rows copied from random places, as recorded on the rig.
        smeared = b"".join(good[(row := rng.randrange(height)) * row_bytes:(row + 1) * row_bytes]
                           for _ in range(height))
        # Tiling: the screen repeated at a fraction of its size.
        tile = bytearray()
        for y in range(height):
            source = good[(y * 3 % height) * row_bytes:(y * 3 % height + 1) * row_bytes]
            tile += b"".join(source[(x * 3 % width) * 3:(x * 3 % width) * 3 + 3]
                             for x in range(width))
        noise = bytes(rng.randrange(256) for _ in range(width * height * 3))
        reader = self.readers[machine]
        for name, frame in (("smeared", smeared), ("tiled", bytes(tile)), ("noise", noise)):
            with self.subTest(frame=name):
                reading = reader.read(frame)
                self.assertFalse(reading.matched)
                self.assertTrue(reading.unreadable, reading.summary())

    def test_font_must_be_a_character_set_with_distinct_letters(self):
        with self.assertRaisesRegex(TestFailure, "too short"):
            Font(synthetic_font()[:900], 0)
        with self.assertRaisesRegex(TestFailure, "not a character set"):
            Font(bytes(1024), 0)
        o, q = (ord("O") - 64) * 8, (ord("Q") - 64) * 8
        data = bytearray(synthetic_font())
        data[q:q + 8] = data[o:o + 8]
        data[q + 7] = 0x10
        reader = ScreenText("C64", Font(bytes(data), 0), 1280, 720, LAYOUTS["C64"])
        self.assertNotIn("O", reader.alphabet)
        self.assertNotIn("Q", reader.alphabet)
        self.assertGreaterEqual(len(reader.alphabet), 8)
        free = [(6, bit) for bit in range(8)] + [(7, bit) for bit in range(8)] + [
            (row, bit) for row in range(1, 5) for bit in range(2, 6)]
        for letter in range(1, 27):
            # Every letter one bit away from the same shape: nothing is safe to type.
            data[letter * 8:letter * 8 + 8] = bytes([0x7E, 0x42, 0x42, 0x42, 0x42, 0x7E, 0, 0])
            row, bit = free[letter - 1]
            data[letter * 8 + row] |= 0x80 >> bit
        with self.assertRaisesRegex(TestFailure, "too few distinct letters"):
            ScreenText("C64", Font(bytes(data), 0), 1280, 720, LAYOUTS["C64"])

    def test_reading_reports_frame_level_failures(self):
        self.assertTrue(Reading(None, unreadable=True).unreadable)
        self.assertIn("no character grid", Reading(None, unreadable=True).summary())
        flat = bytes([200]) * (1280 * 720 * 3)
        reading = self.readers["Plus4"].read(flat)
        self.assertTrue(reading.unreadable)
        self.assertFalse(reading.matched)


@unittest.skipUnless(ROM_ROOT, "set BMC64_ROM_ROOT to an SD-card copy holding the ROMs")
class RecordedScreenTests(unittest.TestCase):
    """Frames recorded from the rig, read with the machine's real character ROM."""

    def reader(self, machine, width, height):
        layout = LAYOUTS[machine]
        data = Path(ROM_ROOT, layout.font_path.lstrip("/")).read_bytes()
        return ScreenText(machine, Font(data, layout.font_offset), width, height, layout)

    def read(self, machine, name, typed=""):
        rgb, width, height = read_png(SCREENS / name)
        return self.reader(machine, width, height).read(rgb, typed)

    def test_recorded_boot_screens_match_every_checked_cell(self):
        for machine in ("C64", "C128", "VIC20", "Plus4", "Plus4Emu", "Pet"):
            with self.subTest(machine=machine):
                reading = self.read(machine, f"{machine.lower()}.png")
                self.assertTrue(reading.matched, reading.summary())

    def test_pi_4_boot_screens_match_every_checked_cell(self):
        for machine in ("C64", "Plus4", "Plus4Emu"):
            with self.subTest(machine=machine):
                reading = self.read(machine, f"{machine.lower()}-pi4.png")
                self.assertTrue(reading.matched, reading.summary())

    def test_recorded_typed_tokens_match_and_wrong_letters_fail(self):
        for name in sorted(SCREENS.glob("*-typed-*.png")):
            machine = {layout.lower(): layout for layout in LAYOUTS}[name.stem.split("-")[0]]
            token = name.stem.rsplit("-", 1)[1]
            rgb, width, height = read_png(name)
            reader = self.reader(machine, width, height)
            plane = reader.plane(rgb)
            fitted = reader.fit(plane)
            with self.subTest(name=name.name):
                reading = reader.compare(plane, fitted, token)
                self.assertTrue(reading.matched, reading.summary())
                self.assertFalse(reader.compare(plane, fitted).matched)
                for column in range(8):
                    for letter in reader.alphabet:
                        if letter != token[column]:
                            self.assertFalse(reader.compare(
                                plane, fitted, token[:column] + letter + token[column + 1:]).matched)

    def test_screen_with_a_key_already_typed_is_not_a_clean_boot(self):
        reading = self.read("Plus4", "plus4-first-key.png")
        self.assertFalse(reading.matched)
        self.assertFalse(reading.unreadable)

    def test_recorded_corrupt_captures_are_unreadable(self):
        for machine, name in (("Plus4Emu", "plus4emu-corrupt.png"), ("Pet", "pet-corrupt.png")):
            with self.subTest(name=name):
                reading = self.read(machine, name)
                self.assertTrue(reading.unreadable, reading.summary())

    def test_80_column_pet_screens_are_never_accepted_as_the_3016(self):
        for name in ("pet-720p.png", "pet-8032-1080p.png"):
            with self.subTest(name=name):
                self.assertFalse(self.read("Pet", name).matched)


def reading(matched=True, unreadable=False):
    return Reading(Geometry(0, 0, 1, 1), 100, 100,
                   [] if matched else [(1, 0, "C", 9)], unreadable=unreadable)


class WaitScreenTests(unittest.TestCase):
    def wait(self, capture, timeout, read, after=None, reopen=None):
        clock = capture.clock
        with tempfile.TemporaryDirectory() as directory, \
                patch("tests.boot.screen.time.monotonic", clock.monotonic), \
                patch("tests.boot.screen.time.sleep", clock.sleep), \
                patch("sys.stdout", new_callable=io.StringIO):
            result = wait_screen(capture, capture.check, timeout,
                                 clock.now if after is None else after, Path(directory),
                                 "boot", read, "C64", reopen)
            log = (Path(directory) / "boot-frames.jsonl").read_text().splitlines()
        return result, [json.loads(line) for line in log]

    def test_two_fresh_matching_frames_required_and_old_frames_ignored(self):
        capture = Capture(Clock())
        calls = []
        result, log = self.wait(capture, 2, lambda rgb: calls.append(rgb) or reading())
        self.assertTrue(result.matched)
        self.assertEqual(len(calls), 2)
        self.assertEqual(len(log), 2)
        old = capture.latest_frame
        capture.check = lambda: None
        with self.assertRaises(TestFailure):
            self.wait(capture, 1, lambda rgb: calls.append(rgb) or reading(), after=old[0])
        self.assertEqual(len(calls), 2)

    def test_a_mismatch_between_matches_restarts_the_count(self):
        sequence = iter([reading(), reading(False), reading(), reading()])
        _, log = self.wait(Capture(Clock()), 5, lambda rgb: next(sequence))
        self.assertEqual([entry["matched"] for entry in log], [True, False, True, True])

    def test_unreadable_frames_reopen_capture_once_then_verify(self):
        capture = Capture(Clock())
        reopened = []
        state = {"good": False}

        def reopen():
            reopened.append(True)
            state["good"] = True

        result, _ = self.wait(capture, 10, lambda rgb: reading() if state["good"]
                              else reading(False, True), reopen=reopen)
        self.assertTrue(result.matched)
        self.assertEqual(reopened, [True])

    def test_failure_categories_distinguish_capture_screen_and_guest(self):
        for read, expected in ((lambda rgb: reading(False, True), "screen_unreadable"),
                               (lambda rgb: reading(False), "guest_not_ready")):
            with self.subTest(expected=expected), self.assertRaises(TestFailure) as failure:
                self.wait(Capture(Clock()), 2, read, reopen=lambda: None)
            self.assertEqual(failure.exception.category, expected)
            self.assertIn("last frame:", str(failure.exception))

    def test_black_flat_and_stalled_frames_are_not_guest_failures(self):
        for rgb, expected in ((bytes(SIZE), "capture_black"),
                              (bytes([100]) * SIZE, "capture_flat")):
            capture = Capture(Clock(), rgb)
            with self.subTest(expected=expected), self.assertRaises(TestFailure) as failure:
                self.wait(capture, 1, lambda frame: self.fail("read a black/flat frame"))
            self.assertEqual(failure.exception.category, expected)
            with tempfile.TemporaryDirectory() as directory:
                failure_evidence(capture, Path(directory), {"status": "FAIL"})
                self.assertTrue((Path(directory) / "failure.ppm").exists())
        capture = Capture(Clock())
        capture.latest_frame = (0, FRAME)
        capture.check = lambda: None
        with self.assertRaises(TestFailure) as failure:
            self.wait(capture, 0.01, lambda rgb: reading(), after=0)
        self.assertEqual(failure.exception.category, "capture_stalled")


class SuiteCaptureTests(unittest.TestCase):
    class Recorder:
        instances = []

        def __init__(self, command, output, width, height):
            self.command, self.output, self.width, self.height = command, output, width, height
            self.latest_frame = (time.monotonic(), bytes([20, 200, 90]) * (width * height // 2)
                                 + bytes([200, 20, 90]) * (width * height - width * height // 2))
            self.latest_audio = (time.monotonic(), b"\0\0")
            self.closed = False
            type(self).instances.append(self)

        def check(self):
            pass

        def close(self):
            self.closed = True
            if self.output.name == "fails":
                raise RuntimeError("FFmpeg failed (255)")

    def test_each_open_records_a_new_segment_at_the_cores_native_size(self):
        self.Recorder.instances = []
        with tempfile.TemporaryDirectory() as directory, \
                patch("hardware.capture.Capture", self.Recorder), \
                patch("sys.stdout", new_callable=io.StringIO):
            capture = SuiteCapture("/video", "audio")
            capture.open(Path(directory) / "fails", (64, 36), lambda: None, 1, "C64")
            capture.open(Path(directory) / "capture-1", (96, 54), lambda: None, 1, "Pet")
            first, second = self.Recorder.instances
            self.assertTrue(first.closed)
            self.assertEqual(capture.errors, ["FFmpeg failed (255)"])
            self.assertIn("96x54", second.command)
            self.assertIn("fps=5,scale=96:54", second.command)
            self.assertEqual(str(second.output / "capture.mkv"), next(
                part for part in second.command if part.endswith("capture.mkv")))
            self.assertEqual((capture.width, capture.height), (96, 54))
            capture.close()
            self.assertTrue(second.closed)

    def test_pet_is_captured_at_1080p_and_other_cores_at_720p(self):
        self.assertEqual(CAPTURE_SIZES, {"Pet": (1920, 1080)})
        self.assertEqual(DEFAULT_CAPTURE_SIZE, (1280, 720))
        command = capture_command("/video", "audio", Path("/unused"), 1920, 1080, "1920x1080")
        self.assertEqual(command[command.index("-video_size") + 1], "1920x1080")

    def test_capture_that_stays_black_fails_startup(self):
        class Black(self.Recorder):
            def __init__(self, *args):
                super().__init__(*args)
                self.latest_frame = (0, bytes(self.width * self.height * 3))

        clock = Clock()
        with tempfile.TemporaryDirectory() as directory, \
                patch("hardware.capture.Capture", Black), \
                patch("tests.boot.screen.time.monotonic", clock.monotonic), \
                patch("tests.boot.screen.time.sleep", clock.sleep), \
                patch("sys.stdout", new_callable=io.StringIO), \
                self.assertRaises(TestFailure) as failure:
            SuiteCapture("/video", "audio").open(Path(directory), (8, 8), lambda: None, 1, "C64")
        self.assertEqual(failure.exception.category, "capture_startup")


class UartTests(unittest.TestCase):
    def test_partial_final_critical_line_is_classified_after_logger_closes(self):
        received = threading.Event()

        class Port:
            pending = b"kernel: PANIC"

            def read(self, timeout):
                data, self.pending = self.pending, None
                if data:
                    return data
                received.set()
                time.sleep(0.001)
                return None

        with tempfile.TemporaryDirectory() as directory:
            logger = UartLog(Port(), Path(directory) / "uart.log")
            try:
                self.assertTrue(received.wait(2))
            finally:
                logger.close()
            with self.assertRaises(TestFailure):
                logger.check()
            self.assertEqual(logger.findings[0]["category"], "panic")

    def test_critical_messages_and_expected_autostart_probes(self):
        for line in ("kernel: PANIC", "exceptionhandler: Data abort",
                     "Assertion failed: foo", "KASAN: use-after-free",
                     "Profile start failed: missing ROM"):
            with self.subTest(line=line):
                self.assertEqual(classify_uart(line)["severity"], "critical")
        for line in ("Filesystem Image: Unknown disk image `/test.prg'.",
                     "Tape: Error - Cannot open file `/test.prg'",
                     "Error - CRT header invalid.",
                     "CART: `/test.prg' is not a valid CRT file.",
                     "Error - archdep_mkdir unimpl: ."):
            with self.subTest(line=line):
                self.assertEqual(classify_uart(line)["severity"], "info")
        for line in ("MPS-803: Error - Could not load MPS-803 charset 'mps803'.",
                     "Palette: Error - Palette not found: `1520.vpl'.",
                     "NL10: Error - Could not load NL-10 ROM file 'nl10-cbm'.",
                     "DriveROM: Error - 1001/8050/8250 ROM image not found. "
                     "Hardware-level 1001/8050/8250 emulation is not available.",
                     "Warning - ROM `/C128/dosCMDHD': long file, discarding end."):
            with self.subTest(line=line):
                self.assertEqual(classify_uart(line)["category"], "boot_startup")
        for line in ("Palette: Error - Palette not found: `pepto-pal.vpl'.",
                     "Sound: Warning - Buffer drained"):
            with self.subTest(line=line):
                self.assertEqual(classify_uart(line)["severity"], "warning")
        self.assertEqual(classify_uart("Sound buffer drained: warning")["severity"], "warning")
        self.assertIsNone(classify_uart("kernel: Keyboard 1 connected and registered."))

    def test_no_frame_evidence_explicitly_explains_missing_screenshot(self):
        with tempfile.TemporaryDirectory() as directory:
            output = Path(directory)
            failure_evidence(Capture(Clock()), output, {"status": "FAIL"})
            self.assertEqual(json.loads((output / "capture-health.json").read_text()),
                             {"video": "no frames received"})
            self.assertFalse((output / "failure.ppm").exists())
        self.assertTrue(frame_health(bytes([7]) * SIZE)["black"])
        blue = bytes([7, 10, 130]) * (WIDTH * HEIGHT)
        self.assertFalse(frame_health(blue)["black"])
        self.assertTrue(frame_health(blue)["flat"])


class RotationTests(unittest.TestCase):
    def simulate(self, failure=None):
        clock, api = Clock(), Profiles("-de56af")
        capture = Capture(clock)
        cases = resolve_profiles(api, read_manifest(ROOT.joinpath("suite_profiles.json")))
        commands = []
        if failure == "font":
            api.fonts[LAYOUTS["C128"].font_path] = b"\0" * 100

        class Pico:
            text = ""

            def mark(self, text):
                pass

            def command(self, command):
                commands.append(command)
                if command.startswith("KEY 0 ") and not (
                        api.current["machine"] == "C128" and failure == "input"):
                    self.text += chr(int(command.split()[-1]) - 4 + ord("A"))
                return "OK protocol=1 mounted=1 suspended=0 pending=0"

        pico = Pico()
        test = self

        class Reader:
            alphabet = "ADJKLMPQ"

            def __init__(self, machine, font, width, height, layout):
                test.assertEqual((width, height), CAPTURE_SIZES.get(machine, DEFAULT_CAPTURE_SIZE))
                test.assertEqual(layout, LAYOUTS[machine])

            def read(self, rgb, typed=""):
                if failure == "outage" and api.current["machine"] == "C128":
                    api.outage = True
                return reading(pico.text == typed)

        class Uart:
            def __init__(self):
                self.findings = []

            def mark(self, text):
                pass

            def check(self, since=0):
                if api.current["machine"] == "C128" and not self.findings:
                    self.findings.append({"severity": "critical", "line": "kernel: panic"})
                if any(item["severity"] == "critical" for item in self.findings[since:]):
                    raise TestFailure("uart_critical", "critical UART message: kernel: panic")

        uart = Uart() if failure == "uart" else None
        original_request, original_get = api.request, api.get_json
        status_failures = []

        def request(*args, **kwargs):
            pico.text = ""
            return original_request(*args, **kwargs)

        def get_json(endpoint, **query):
            # One lost status reply after C128 typing, like the 3 s timeout seen on the rig.
            if (failure == "status_timeout" and endpoint == "/api/status"
                    and api.current["machine"] == "C128" and pico.text and not status_failures):
                status_failures.append(endpoint)
                raise URLError("timed out")
            return original_get(endpoint, **query)

        report = {"cases": []}
        with tempfile.TemporaryDirectory() as directory, \
                patch("tests.boot.screen.time.monotonic", clock.monotonic), \
                patch("tests.boot.screen.time.sleep", clock.sleep), \
                patch("tests.boot.base.ScreenText", Reader), \
                patch.object(api, "request", side_effect=request), \
                patch.object(api, "get_json", side_effect=get_json), \
                patch("sys.stdout", new_callable=io.StringIO) as terminal, \
                patch("sys.stderr", new_callable=io.StringIO):
            rotate(api, pico, capture, uart, cases, 2, Path(directory), report)
            messages = terminal.getvalue()
            for message in (
                "[1/6] c64-boot-typing: starting (C64, profile c64-test-de56af)",
                "reading character ROM /C64/chargen from the SD card",
                "checking the BASIC boot screen cell by cell against the ROM font",
                "Pico USB keyboard ready/remounted",
                "typing token", "verified on two fresh frames",
                "releasing USB input and saving test evidence",
            ):
                self.assertIn(message, messages)
            for result in report["cases"]:
                if result["status"] == "FAIL":
                    self.assertTrue((Path(directory) / result["evidence"] / "failure.ppm").exists())
        self.assertEqual(commands[-1], "RELEASE")
        self.assertTrue(all(start["once"] == "1" for start in api.starts))
        self.assertFalse(any(command == "KEY 0 40" for command in commands))
        self.status_failures = status_failures
        return report, api.starts, capture

    def test_all_six_profiles_require_basic_and_usb_typing(self):
        report, starts, capture = self.simulate()
        self.assertEqual(len(starts), 6)
        self.assertEqual([case["status"] for case in report["cases"]], ["PASS"] * 6)
        for case in report["cases"]:
            self.assertEqual(case["checks"],
                             ["profile_and_core", "basic_ready", "usb_typing_visible"])
            self.assertEqual(case["typing_method"], "rom_glyphs")
            self.assertTrue(set(case["token"]) <= set(case["typing_alphabet"]))
            self.assertEqual(case["font"]["path"], LAYOUTS[case["machine"]].font_path)
            self.assertEqual(len(case["font"]["sha256"]), 64)
        # Capture reopens once per case after the core is verified, at its HDMI size.
        self.assertEqual(capture.opened, [
            ("capture-1", CAPTURE_SIZES.get(machine, DEFAULT_CAPTURE_SIZE))
            for machine in ("C64", "C128", "VIC20", "Plus4", "Plus4Emu", "Pet")])

    def test_failed_typing_continues_when_http_control_is_available(self):
        report, starts, _ = self.simulate("input")
        self.assertEqual(len(starts), 6)
        self.assertEqual(report["cases"][1]["status"], "FAIL")
        self.assertEqual(report["cases"][1]["category"], "typing_not_visible")
        self.assertEqual([case["status"] for case in report["cases"][2:]], ["PASS"] * 4)

    def test_unusable_font_fails_that_case_before_switching(self):
        report, starts, _ = self.simulate("font")
        self.assertEqual(len(starts), 5)
        failed = report["cases"][1]
        self.assertEqual(failed["status"], "FAIL")
        self.assertEqual((failed["category"], failed["stage"]), ("font_rom", "font_rom"))
        self.assertEqual(failed["checks"], [])
        self.assertEqual([case["status"] for case in report["cases"][2:]], ["PASS"] * 4)

    def test_lost_final_status_reply_is_retried_not_failed(self):
        report, starts, _ = self.simulate("status_timeout")
        self.assertEqual(self.status_failures, ["/api/status"])
        self.assertEqual([case["status"] for case in report["cases"]], ["PASS"] * 6)

    def test_lost_http_blocks_remaining_cases_without_further_switches(self):
        report, starts, _ = self.simulate("outage")
        self.assertEqual(len(starts), 2)
        self.assertEqual(report["cases"][1]["status"], "FAIL")
        self.assertEqual([case["status"] for case in report["cases"][2:]], ["NOT_RUN"] * 4)
        self.assertIn("control_error", report)

    def test_critical_uart_fails_its_case_without_poisoning_later_cases(self):
        report, starts, _ = self.simulate("uart")
        self.assertEqual(len(starts), 6)
        self.assertEqual(report["cases"][1]["category"], "uart_critical")
        self.assertEqual(len(report["cases"][1]["uart_findings"]), 1)
        self.assertEqual([case["status"] for case in report["cases"][2:]], ["PASS"] * 4)

    def test_pico_remount_deadline_and_bad_protocol(self):
        clock = Clock()
        for reply, expected in (("OK protocol=1 mounted=0", "pico_unavailable"),
                                ("OK protocol=2 mounted=1", "pico_protocol")):
            with self.subTest(reply=reply), \
                    patch("tests.boot.screen.time.monotonic", clock.monotonic), \
                    patch("tests.boot.screen.time.sleep", clock.sleep), \
                    patch("support.keyboard.Pico") as pico:
                pico.command.return_value = reply
                with self.assertRaises(TestFailure) as failure:
                    wait_pico(pico, lambda: None, 1)
                self.assertEqual(failure.exception.category, expected)

    def test_typing_is_bounded_and_never_sends_return(self):
        with patch("tests.boot.screen.time.sleep"), patch("support.keyboard.Pico") as pico:
            type_token(pico, "ACEFKMNT", lambda: None)
            commands = [call.args[0] for call in pico.command.call_args_list]
            self.assertEqual(len(commands), 16)
            self.assertEqual(commands[-1], "KEY 0")
            self.assertNotIn("KEY 0 40", commands)
            with self.assertRaises(ValueError):
                type_token(pico, "PRINT123", lambda: None)

    def manifest(self):
        return read_manifest(ROOT.joinpath("suite_profiles.json"))

    def test_offline_preparation_cannot_open_hardware(self):
        with tempfile.TemporaryDirectory() as directory:
            args = parser().parse_args(["--results", directory])
            with patch("framework.runner.Api", side_effect=AssertionError("HTTP")), \
                    patch("framework.rig.SerialPort", side_effect=AssertionError("UART")), \
                    patch("hardware.capture.Capture", side_effect=AssertionError("capture")), \
                    patch("sys.stdout", new_callable=io.StringIO):
                report = boot_run(args, self.manifest())
            self.assertEqual(report["status"], "PREPARED")
            self.assertEqual([case["status"] for case in report["tests"]], ["PREPARED"] * 6)
            self.assertEqual(json.loads(Path(report["evidence"]).read_text())["status"], "PREPARED")

    def test_preflight_error_or_interruption_records_fail_and_all_unrun_cases(self):
        for error in (TestFailure("profile_preflight", "missing test profiles"), KeyboardInterrupt()):
            with self.subTest(error=error), tempfile.TemporaryDirectory() as directory:
                args = parser().parse_args([
                    "--execute", "--url", "http://192.0.2.1", "--pico-uart", "/unused",
                    "--results", directory,
                ])
                with patch("framework.runner.preflight", side_effect=error), \
                        patch("sys.stdout", new_callable=io.StringIO), \
                        patch("sys.stderr", new_callable=io.StringIO):
                    report = boot_run(args, self.manifest())
                self.assertEqual(report["status"], "FAIL")
                self.assertEqual([case["status"] for case in report["tests"]], ["NOT_RUN"] * 6)
                self.assertEqual([case["name"] for case in report["tests"]],
                                 [machine.lower() + "-boot-typing" for machine in MACHINES])

    def cli(self, *arguments):
        return subprocess.run(["python3", str(ROOT.joinpath("run_smoke")), *arguments],
                              capture_output=True, text=True, timeout=30)

    def test_list_names_every_test(self):
        completed = self.cli("--list")
        self.assertEqual(completed.returncode, 0, completed.stderr)
        names = [line.split()[0] for line in completed.stdout.splitlines()]
        self.assertEqual(names, [machine.lower() + "-boot-typing" for machine in MACHINES]
                         + ["c64-default"])

    def test_no_arguments_prepares_every_test_and_ends_with_a_summary(self):
        with tempfile.TemporaryDirectory() as directory:
            completed = self.cli("--results", directory)
            self.assertEqual(completed.returncode, 0, completed.stderr)
            summary = completed.stdout[completed.stdout.index("Summary:"):].splitlines()
            self.assertEqual([line.split()[:2] for line in summary[1:8]],
                             [["PREPARED", machine.lower() + "-boot-typing"] for machine in MACHINES]
                             + [["PREPARED", "c64-default"]])
            self.assertEqual(summary[8].strip(), "7 PREPARED")
            self.assertEqual(len(list(Path(directory).glob("*/result.json"))), 1)

    def test_names_select_tests_and_unknown_names_are_rejected(self):
        with tempfile.TemporaryDirectory() as directory:
            completed = self.cli("pet-boot-typing", "--results", directory)
            self.assertEqual(completed.returncode, 0, completed.stderr)
            result = json.loads(next(Path(directory).glob("*/result.json")).read_text())
            self.assertEqual([case["name"] for case in result["tests"]], ["pet-boot-typing"])
            self.assertNotIn("c64-default", completed.stdout)
        with tempfile.TemporaryDirectory() as directory:
            completed = self.cli("--results", directory, "pet-boot-typing", "c64-default")
            self.assertEqual(completed.returncode, 0, completed.stderr)
            result = json.loads(next(Path(directory).glob("*/result.json")).read_text())
            self.assertEqual([case["name"] for case in result["tests"]],
                             ["pet-boot-typing", "c64-default"])
            self.assertNotIn("c64-boot-typing", completed.stdout)
        completed = self.cli("c64-typing")
        self.assertEqual(completed.returncode, 2)
        self.assertIn("unknown test c64-typing", completed.stderr)

    def test_custom_manifest_defines_the_boot_typing_tests(self):
        with tempfile.TemporaryDirectory() as directory:
            manifest = Path(directory) / "only-pet.json"
            manifest.write_text('[{"machine":"Pet","profile":"pet-test-de56af"}]')
            completed = self.cli("--manifest", str(manifest), "--list")
            # Every test needs its machine's profile, so the diagnostic is left out too.
            self.assertEqual([line.split()[0] for line in completed.stdout.splitlines()],
                             ["pet-boot-typing"])
            for name in ("c64-boot-typing", "c64-default"):
                completed = self.cli("--manifest", str(manifest), name)
                self.assertEqual(completed.returncode, 2)

    def test_folder_suites_select_and_deduplicate_existing_tests(self):
        for selectors, expected in (
            (["boot"], [machine.lower() + "-boot-typing" for machine in MACHINES]),
            (["basic"], ["c64-default"]),
            (["boot", "c64-boot-typing", "basic"], [
                machine.lower() + "-boot-typing" for machine in MACHINES] + ["c64-default"]),
        ):
            with self.subTest(selectors=selectors), tempfile.TemporaryDirectory() as directory:
                completed = self.cli(*selectors, "--results", directory)
                self.assertEqual(completed.returncode, 0, completed.stderr)
                summary = completed.stdout[completed.stdout.index("Summary:"):].splitlines()
                self.assertEqual([line.split()[1] for line in summary[1:1 + len(expected)]], expected)
                self.assertEqual(len(list(Path(directory).glob("*/result.json"))), 1)

    def test_summary_lists_every_test_last_with_failure_reasons(self):
        results = [
            {"name": "c64-boot-typing", "status": "PASS", "stage": "complete"},
            {"name": "c128-boot-typing", "status": "FAIL", "stage": "typing",
             "error": "typing: no match"},
            {"name": "pet-boot-typing", "status": "NOT_RUN", "error": "HTTP control unavailable"},
            {"name": "c64-default", "status": "PREPARED", "stage": "offline"},
        ]
        with patch("sys.stdout", new_callable=io.StringIO) as terminal:
            print_summary(results, 3725.4)
        self.assertEqual(terminal.getvalue().splitlines()[1:], [
            "Summary:",
            "  PASS     c64-boot-typing",
            "  FAIL     c128-boot-typing [typing]: typing: no match",
            "  NOT_RUN  pet-boot-typing: HTTP control unavailable",
            "  PREPARED c64-default",
            "  1 PASS, 1 PREPARED, 1 FAIL, 1 NOT_RUN",
            "  Total time: 1h 2m 5s",
        ])
        from framework.reporting import duration
        self.assertEqual([duration(value) for value in (4.4, 72.4, 600)], ["4s", "1m 12s", "10m 0s"])


if __name__ == "__main__":
    unittest.main()
