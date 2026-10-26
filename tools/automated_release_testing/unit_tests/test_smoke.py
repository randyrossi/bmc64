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

"""Offline assertions, simulated serial/HTTP, and synthetic FFmpeg capture."""

import io
import fcntl
import json
import math
import os
import pty
import queue
import random
import re
import select
import struct
import subprocess
import sys
import tempfile
import threading
import time
import unittest
from http.server import BaseHTTPRequestHandler, ThreadingHTTPServer
from pathlib import Path
ROOT = Path(__file__).resolve().parents[1]
from unittest.mock import patch
from urllib.parse import parse_qs, urlsplit

from tests.basic.diagnostic import (
    HEIGHT, SAMPLE_RATE, WIDTH, audio_metrics, decode_frame,
    is_silence, is_tone, payload,
)
from utils.configure_test_setup import (
    LAUNCHER, LEGACY_LAUNCHER, ask_device, configure, discover_devices,
)
from utils.preview_video import main as preview_main, preview_command
from hardware.api import Api
from hardware.capture import Capture, capture_command, wait_for
from hardware.pico import Pico, SerialPort
from hardware.uart import UartLog
from utils.runner import HERE, load_setup, parse_arguments, parser
from framework.context import TestContext
from framework.rig import Rig
from framework.runner import execute_case
from tests.basic.test_c64_default import Test as DiagnosticCase, build_guest


RUN_ID = 0x1234ABCD


class UartLogTests(unittest.TestCase):
    def test_fragmented_lines_preserve_bytes_and_partial_final_line(self):
        source = b"Oct 25 kernel: ready\r\nsecond line\n\rthird \xffline"
        for chunks in ([bytes([value]) for value in source],
                       [source], [source[:12], source[12:23], source[23:]]):
            with self.subTest(chunks=len(chunks)), tempfile.TemporaryDirectory() as directory:
                exhausted = threading.Event()

                class Port:
                    def __init__(self):
                        self.chunks = iter(chunks)

                    def read(self, timeout):
                        data = next(self.chunks, None)
                        if data is None:
                            exhausted.set()
                            time.sleep(0.001)
                        return data

                path = Path(directory) / "uart.log"
                logger = UartLog(Port(), path)
                try:
                    self.assertTrue(exhausted.wait(2))
                    logger.mark("===== marker =====")
                finally:
                    logger.close()
                self.assertEqual((Path(directory) / "uart.raw").read_bytes(), source)
                lines = path.read_text(encoding="utf-8").splitlines()
                self.assertTrue(all(re.match(r"\d\d:\d\d:\d\d\.\d{3}  ", line)
                                    for line in lines))
                # CRLF and BMC64's LF CR each end one line; the partial line is kept.
                self.assertEqual([line[14:] for line in lines], [
                    "Oct 25 kernel: ready", "second line", "===== marker =====",
                    "third \ufffdline"])

    def test_read_failure_is_explicit(self):
        class Port:
            def read(self, timeout):
                raise RuntimeError("disconnected")

        with tempfile.TemporaryDirectory() as directory:
            logger = UartLog(Port(), Path(directory) / "uart.log")
            logger.thread.join(2)
            with self.assertRaisesRegex(RuntimeError, "disconnected"):
                logger.close()


class VideoPreviewTests(unittest.TestCase):
    def test_saved_video_uses_baseline_without_contacting_target(self):
        with tempfile.TemporaryDirectory() as directory:
            config = Path(directory) / "test_setup.json"
            config.write_text(json.dumps({
                "url": "http://192.0.2.1/", "pico_uart": "/dev/unused",
                "video": "/dev/v4l/by-id/HDMI capture",
            }))
            with patch("utils.preview_video.shutil.which", return_value="/usr/bin/ffplay"), \
                    patch("utils.preview_video.subprocess.run") as run, \
                    patch("sys.stdout", new_callable=io.StringIO), \
                    patch.object(Api, "request", side_effect=AssertionError("HTTP accessed")):
                run.return_value.returncode = 0
                self.assertEqual(preview_main(["--config", str(config)]), 0)
            command = run.call_args.args[0]
            self.assertEqual(command, preview_command("/dev/v4l/by-id/HDMI capture"))
            self.assertIn("-an", command)
            self.assertIn("mjpeg", command)

    def test_override_without_config_and_exit_status(self):
        with patch("utils.preview_video.shutil.which", return_value="/usr/bin/ffplay"), \
                patch("utils.preview_video.subprocess.run") as run, \
                patch("utils.preview_video.load_setup", side_effect=AssertionError("config accessed")), \
                patch("sys.stdout", new_callable=io.StringIO):
            run.return_value.returncode = 3
            self.assertEqual(preview_main(["--video", "/dev/video4"]), 3)

    def test_missing_config_or_ffplay_does_not_launch(self):
        with tempfile.TemporaryDirectory() as directory:
            for argv in (["--config", str(Path(directory) / "missing.json")],
                         ["--video", "/dev/video4"],
                         ["--video", "relative"]):
                with self.subTest(argv=argv), \
                        patch("utils.preview_video.shutil.which", return_value=None), \
                        patch("utils.preview_video.subprocess.run") as run, \
                        patch("sys.stderr", new_callable=io.StringIO):
                    self.assertEqual(preview_main(argv), 1)
                    run.assert_not_called()

    def test_preview_launcher_resolves_its_directory_without_opening_device(self):
        script = ROOT.joinpath("run_video_test")
        completed = subprocess.run(
            [str(script.resolve()), "--help"], cwd=script.parent / "mock_hid",
            capture_output=True, text=True, timeout=10,
        )
        self.assertEqual(completed.returncode, 0, completed.stderr)
        self.assertIn("--video", completed.stdout)


class SetupTests(unittest.TestCase):
    def setUp(self):
        discovery = patch("utils.configure_test_setup.discover_devices", return_value=[])
        self.discovery = discovery.start()
        self.addCleanup(discovery.stop)
        replacements = {
            "preview": {}, "check_preview": {}, "Api": {}, "apply_network": {},
            "recover_api": {"return_value": {"machine": "C64", "profile_id": "main"}},
            "resolve_profiles": {"return_value": [
                {"machine": "C64", "id": "c64-test", "volume": "SD"}]},
            "begin_restart": {"return_value": ({}, 0)},
            "verify_restart": {}, "getpass.getpass": {"return_value": "private-pin"},
        }
        for name, options in replacements.items():
            mock = patch(f"utils.configure_test_setup.{name}", **options)
            mock.start()
            self.addCleanup(mock.stop)

    def test_numbered_wizard_saves_selected_device_paths(self):
        self.discovery.side_effect = [
            ["/dev/v4l/by-id/WEBCAM", "/dev/v4l/by-id/HDMI"],
            ["/dev/serial/by-id/PICO", "/dev/serial/by-id/BMC64"],
        ]
        answers = ["2", "192.0.2.1", "n", "wifi", "1", "1", "", "", "", "y", "", ""]
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            with patch("builtins.input", side_effect=answers), \
                    patch("sys.stdout", new_callable=io.StringIO) as output:
                self.assertTrue(configure(root))
            settings = load_setup(root / "test_setup.json")
            self.assertEqual(settings["pico_uart"], "/dev/serial/by-id/PICO")
            self.assertEqual(settings["bmc_uart"], "/dev/serial/by-id/BMC64")
            self.assertEqual(settings["video"], "/dev/v4l/by-id/HDMI")
            self.assertIn("2) /dev/v4l/by-id/HDMI", output.getvalue())

    def test_device_menu_rejects_invalid_choices_and_duplicate_uart(self):
        with patch("builtins.input", side_effect=["0", "9", "/dev/PICO", "1"]), \
                patch("sys.stdout", new_callable=io.StringIO):
            selected = ask_device("BMC64 UART", ["/dev/PICO", "/dev/BMC"],
                                  exclude="/dev/PICO")
        self.assertEqual(selected, "/dev/BMC")
        with patch("builtins.input", return_value="-"), \
                patch("sys.stdout", new_callable=io.StringIO):
            self.assertEqual(ask_device("Optional UART", []), "")
        with patch("builtins.input", return_value=""), \
                patch("sys.stdout", new_callable=io.StringIO):
            self.assertEqual(ask_device("Video", [], "/dev/video4", required=True),
                             "/dev/video4")

    def test_discovery_prefers_stable_names_and_deduplicates_aliases(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            (root / "serial/by-id").mkdir(parents=True)
            (root / "serial/by-path").mkdir(parents=True)
            (root / "v4l/by-id").mkdir(parents=True)
            for name in ("ttyUSB0", "ttyACM0", "ttyS0", "video0", "video1"):
                (root / name).touch()
            (root / "serial/by-id/adapter").symlink_to("../../ttyUSB0")
            (root / "serial/by-path/port").symlink_to("../../ttyUSB0")
            (root / "serial/by-id/disconnected").symlink_to("../../ttyUSB9")
            (root / "v4l/by-id/hdmi-index0").symlink_to("../../video0")
            uarts = discover_devices("uart", root)
            self.assertEqual(uarts, [
                str(root / "serial/by-id/adapter"),
                str(root / "ttyACM0"), str(root / "ttyS0"),
            ])
            self.assertEqual(discover_devices("video", root), [
                str(root / "v4l/by-id/hdmi-index0"), str(root / "video1"),
            ])
            with patch("builtins.input", side_effect=["1"]), \
                    patch("sys.stdout", new_callable=io.StringIO) as output:
                self.assertEqual(ask_device("UART", uarts), uarts[0])
            self.assertIn(f"-> {root / 'ttyUSB0'}", output.getvalue())

    def settings(self):
        return {
            "url": "http://192.0.2.1:8080/",
            "pico_uart": "/dev/serial/by-id/PICO",
            "bmc_uart": "/dev/serial/by-id/BMC64",
            "video": "/dev/video4", "audio": "plughw:CARD=TEST,DEV=0",
            "setup": "test rig", "timeout": 30, "ask_pin": True,
        }

    def test_config_defaults_cli_overrides_and_no_implicit_execution(self):
        with tempfile.TemporaryDirectory() as directory:
            path = Path(directory) / "setup.json"
            path.write_text(json.dumps(self.settings()), encoding="utf-8")
            args = parse_arguments(["--config", str(path), "--timeout", "90"])
            self.assertEqual(args.url, "http://192.0.2.1:8080/")
            self.assertEqual(args.timeout, 90)
            self.assertTrue(args.ask_pin)
            self.assertFalse(args.execute)
            self.assertTrue(parse_arguments(["--config", str(path), "--execute"]).execute)

    def test_invalid_configs_rejected(self):
        invalid = [
            [], {"execute": True}, {"url": "http://host/"},
            {**self.settings(), "pin": "must-not-be-saved"},
            {**self.settings(), "timeout": float("nan")},
            {**self.settings(), "timeout": float("inf")},
            {**self.settings(), "timeout": True},
            {**self.settings(), "timeout": -1},
            {**self.settings(), "ask_pin": "yes"},
            {**self.settings(), "url": "ftp://host"},
            {**self.settings(), "pico_uart": "relative-device"},
            {**self.settings(), "audio": ""},
            {**self.settings(), "setup": "line\nbreak"},
            {**self.settings(), "network": "auto"},
            {**self.settings(), "network": 1},
        ]
        with tempfile.TemporaryDirectory() as directory:
            path = Path(directory) / "setup.json"
            for value in invalid:
                with self.subTest(value=value):
                    path.write_text(json.dumps(value), encoding="utf-8")
                    with self.assertRaises(ValueError):
                        load_setup(path)
            path.write_text("{broken", encoding="utf-8")
            with self.assertRaises(ValueError):
                load_setup(path)
            with patch("sys.stderr", new_callable=io.StringIO):
                with self.assertRaises(SystemExit) as failure:
                    parse_arguments(["--config", str(path)])
            self.assertEqual(failure.exception.code, 2)

    def test_wizard_creates_private_setup_and_executable_launcher_without_hardware(self):
        answers = ["", "http://192.0.2.1/", "not-an-ip", "192.0.2.999", "192.0.2.1",
                   "yes", "wifi", "/dev/PICO", "/dev/BMC", "", "Pi test", "nan", "45",
                   "yes", "", ""]
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            with patch("builtins.input", side_effect=answers), \
                    patch("sys.stdout", new_callable=io.StringIO), \
                    patch.object(Api, "request", side_effect=AssertionError("HTTP accessed")):
                self.assertTrue(configure(root))
            config = root / "test_setup.json"
            settings = load_setup(config)
            self.assertEqual(settings["url"], "http://192.0.2.1/")
            self.assertEqual(settings["timeout"], 45)
            self.assertTrue(settings["ask_pin"])
            self.assertNotIn("pin", settings)
            self.assertEqual(config.stat().st_mode & 0o777, 0o600)
            launcher = root / "run_tests"
            self.assertEqual(launcher.read_text(), LAUNCHER)
            self.assertEqual(launcher.stat().st_mode & 0o777, 0o755)
            self.assertFalse((root / "local").exists())

    def test_existing_generated_launcher_can_be_upgraded_without_changing_settings(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            settings = self.settings()
            (root / "test_setup.json").write_text(json.dumps(settings))
            (root / "run_tests").write_text(LEGACY_LAUNCHER)
            with patch("builtins.input", side_effect=[""] * 12), \
                    patch("sys.stdout", new_callable=io.StringIO):
                self.assertTrue(configure(root))
            self.assertEqual((root / "run_tests").read_text(), LAUNCHER)
            self.assertEqual(load_setup(root / "test_setup.json"), {**settings, "network": "wifi"})

    def test_reconfigure_reuses_values_and_can_clear_optional_fields(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            config = root / "test_setup.json"
            config.write_text(json.dumps(self.settings()), encoding="utf-8")
            with patch("builtins.input", side_effect=[
                    "", "", "", "", "", "-", "", "-", "", "", "", ""]), \
                    patch("sys.stdout", new_callable=io.StringIO):
                self.assertTrue(configure(root))
            updated = load_setup(config)
            self.assertEqual(updated["url"], self.settings()["url"])
            self.assertEqual(updated["bmc_uart"], "")
            self.assertEqual(updated["setup"], "")
            self.assertEqual(updated["timeout"], 30)
            self.assertTrue(updated["ask_pin"])

    def test_ip_prompt_handles_ipv6_and_preserves_saved_port(self):
        for saved, address, expected in (
            (None, "2001:db8::1", "http://[2001:db8::1]/"),
            ("http://192.0.2.1:8080/", "192.0.2.2", "http://192.0.2.2:8080/"),
        ):
            with self.subTest(address=address), tempfile.TemporaryDirectory() as directory:
                root = Path(directory)
                if saved:
                    settings = self.settings()
                    settings["url"] = saved
                    (root / "test_setup.json").write_text(json.dumps(settings))
                answers = ["", address, "n", "", "/dev/PICO", "-", "", "", "", "y", "", ""]
                with patch("builtins.input", side_effect=answers) as prompt, \
                        patch("sys.stdout", new_callable=io.StringIO):
                    self.assertTrue(configure(root))
                self.assertEqual(load_setup(root / "test_setup.json")["url"], expected)
                first_prompt = prompt.call_args_list[1].args[0]
                self.assertIn("BMC64 IP address", first_prompt)
                self.assertNotIn("http://", first_prompt)
                self.assertNotIn(":8080", first_prompt)

    def test_cancel_and_unrecognised_launcher_preserve_files(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            config = root / "test_setup.json"
            config.write_text(json.dumps(self.settings()), encoding="utf-8")
            original = config.read_bytes()
            with patch("builtins.input", side_effect=[""] * 9 + ["n"]), \
                    patch("sys.stdout", new_callable=io.StringIO):
                self.assertFalse(configure(root))
            self.assertEqual(config.read_bytes(), original)
            self.assertFalse((root / "run_tests").exists())
            (root / "run_tests").write_text("do not overwrite", encoding="utf-8")
            with self.assertRaisesRegex(RuntimeError, "not the generated launcher"):
                configure(root)
            self.assertEqual((root / "run_tests").read_text(), "do not overwrite")

    def test_launcher_works_from_other_directory_with_spaces_and_propagates_exit(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory) / "test rig"
            root.mkdir()
            (root / "test_setup.json").write_text(json.dumps(self.settings()))
            with patch("builtins.input", side_effect=[""] * 12), \
                    patch("sys.stdout", new_callable=io.StringIO):
                configure(root)
            (root / "run_smoke").write_text(
                "import json,sys\nprint(json.dumps(sys.argv[1:]))\nsys.exit(7)\n",
                encoding="utf-8",
            )
            completed = subprocess.run(
                [str(root / "run_tests"), "--setup", "literal $(not-a-command)"],
                cwd=directory, capture_output=True, text=True, timeout=10,
            )
            self.assertEqual(completed.returncode, 7)
            self.assertEqual(json.loads(completed.stdout), [
                "--config", str(root / "test_setup.json"), "--execute",
                "--setup", "literal $(not-a-command)",
            ])

    def test_saved_setup_offline_cli_generates_prepared_result(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            config = root / "setup.json"
            config.write_text(json.dumps(self.settings()), encoding="utf-8")
            completed = subprocess.run([
                sys.executable, str(ROOT.joinpath("run_smoke")),
                "--config", str(config), "--results", str(root / "results"),
            ], capture_output=True, text=True, timeout=10)
            self.assertEqual(completed.returncode, 0, completed.stderr)
            # With no test names, every test is prepared in one run folder.
            reports = list((root / "results").glob("*/result.json"))
            self.assertEqual(len(reports), 1)
            report = json.loads(reports[0].read_text())
            self.assertEqual(report["status"], "PREPARED")
            self.assertEqual(len(report["tests"]), 7)
            self.assertEqual({test["status"] for test in report["tests"]}, {"PREPARED"})
            self.assertEqual(report["url"], self.settings()["url"])
            self.assertEqual(report["pico_uart"], self.settings()["pico_uart"])
            self.assertTrue(list((root / "results").glob("*/07-c64-default/diagnostic.prg")))

    def test_root_launchers_work_from_an_unrelated_directory(self):
        with tempfile.TemporaryDirectory(prefix="test rig ") as directory:
            for command in ("configure_test_setup", "run_smoke", "run_video_test"):
                with self.subTest(command=command):
                    completed = subprocess.run(
                        [str(ROOT / command), "--help"], cwd=directory,
                        capture_output=True, text=True, timeout=10)
                    self.assertEqual(completed.returncode, 0, completed.stderr)
                    self.assertIn("usage:", completed.stdout)
            config = Path(directory) / "test setup.json"
            config.write_text(json.dumps(self.settings()))
            completed = subprocess.run(
                [str(ROOT / "run_tests"), "--config", str(config), "--list"],
                cwd=directory, capture_output=True, text=True, timeout=10)
            self.assertEqual(completed.returncode, 0, completed.stderr)
            self.assertEqual(len(completed.stdout.splitlines()), 7)
            self.assertIn("c64-default", completed.stdout)

    def test_moved_utilities_keep_root_paths_and_the_supplied_launcher_matches(self):
        self.assertEqual(HERE, ROOT)
        args = parser().parse_args([])
        self.assertEqual(args.manifest, ROOT / "suite_profiles.json")
        self.assertEqual(args.results, ROOT / "results")
        self.assertEqual((ROOT / "run_tests").read_text(), LAUNCHER)


def render_marker(data: bytes, bounds=(64, 72, 384, 252)) -> bytes:
    left, top, right, bottom = bounds
    image = bytearray(WIDTH * HEIGHT * 3)
    width, height = right - left, bottom - top
    for y in range(top, bottom):
        for x in range(left, right):
            gx, gy = (x - left) * 32 / width, (y - top) * 18 / height
            if gx < 1 or gx >= 31 or gy < 1 or gy >= 17:
                color = (150, 45, 45)
            else:
                column, row = int((gx - 1) / 3), int((gy - 1) / 2)
                bit = row * 10 + column
                level = 225 if data[bit // 8] & (1 << (7 - bit % 8)) else 15
                color = (level, level, level)
            offset = (y * WIDTH + x) * 3
            image[offset:offset + 3] = bytes(color)
    return bytes(image)


def pcm_tone(frequency: float = 1000, amplitude: float = 0.2) -> bytes:
    return struct.pack(f"<{SAMPLE_RATE}h", *(
        int(amplitude * 32767 * math.sin(2 * math.pi * frequency * i / SAMPLE_RATE))
        for i in range(SAMPLE_RATE)
    ))


class GuestTests(unittest.TestCase):
    def test_current_marker_and_scaled_bounds(self):
        for bounds in ((64, 72, 384, 252), (31, 29, 447, 335)):
            with self.subTest(bounds=bounds):
                marker = decode_frame(render_marker(payload(RUN_ID, 255, 1), bounds), RUN_ID)
                self.assertIsNotNone(marker)
                assert marker is not None
                self.assertEqual((marker.counter, marker.state), (255, 1))

    def test_stale_corrupt_blank_and_ambiguous_frames_rejected(self):
        self.assertIsNone(decode_frame(render_marker(payload(RUN_ID, 0, 0)), RUN_ID + 1))
        data = bytearray(payload(RUN_ID, 0, 0))
        data[6] ^= 1
        self.assertIsNone(decode_frame(render_marker(data), RUN_ID))
        self.assertIsNone(decode_frame(bytes(WIDTH * HEIGHT * 3), RUN_ID))
        self.assertIsNone(decode_frame(bytes([120]) * WIDTH * HEIGHT * 3, RUN_ID))
        with self.assertRaises(ValueError):
            decode_frame(b"", RUN_ID)

    def test_guest_build_and_detokenize(self):
        with tempfile.TemporaryDirectory() as directory:
            output = Path(directory)
            program = build_guest(RUN_ID, output)
            decoded = subprocess.run(
                ["petcat", "-2", "--", str(program)], check=True,
                capture_output=True, text=True,
            ).stdout.lower()
            self.assertIn("get a$", decoded)
            self.assertIn("1234abcd", decoded)
            self.assertIn("poke 54276,32", decoded)
            self.assertIn("for b=l to h", decoded)
            self.assertIn("data 182,100,18,52,171,205,0,0,216,165", decoded)

    def test_audio_tone_noise_silence_and_thresholds(self):
        for frequency in (995, 1000, 1038, 1045):
            with self.subTest(frequency=frequency):
                self.assertTrue(is_tone(audio_metrics(pcm_tone(frequency))))
        self.assertFalse(is_tone(audio_metrics(pcm_tone(400))))
        self.assertFalse(is_tone(audio_metrics(bytes(SAMPLE_RATE * 2))))
        rng = random.Random(4)
        noise = struct.pack(f"<{SAMPLE_RATE}h", *(
            rng.randint(-10000, 10000) for _ in range(SAMPLE_RATE)
        ))
        self.assertFalse(is_tone(audio_metrics(noise)))
        self.assertTrue(is_silence(audio_metrics(bytes(SAMPLE_RATE * 2)), 0.1))
        self.assertFalse(is_silence(audio_metrics(pcm_tone()), 0.1))
        self.assertTrue(is_tone({"rms": 0.003, "tone_fraction": 0.2}))
        self.assertFalse(is_tone({"rms": 0.0029, "tone_fraction": 0.8}))
        self.assertTrue(is_silence({"rms": 0.015}, 0.1))
        self.assertFalse(is_silence({"rms": 0.0151}, 0.1))


class SerialTests(unittest.TestCase):
    def test_first_command_after_pico_restart_resynchronizes_without_replaying_input(self):
        clock = [0.0]

        class Port:
            def __init__(self):
                self.restart()
                self.executed = []
                self.writes = []

            def restart(self):
                # Like the firmware: any non-ASCII RX byte poisons the current
                # line until LF. A USB-powered restart can leave this state.
                self.pending = b"\0"
                self.received = b"\x80\0" + b"0 EVENT ready protocol=1\r\n"

            def write(self, data):
                self.writes.append(data)
                self.pending += data
                while b"\n" in self.pending:
                    line, self.pending = self.pending.split(b"\n", 1)
                    if any(value < 32 or value > 126 for value in line):
                        self.received += b"0 ERROR invalid-or-long-line\r\n"
                    elif not line:
                        self.received += b"0 ERROR syntax\r\n"
                    else:
                        request_id, command = line.split(b" ", 1)
                        self.executed.append(command)
                        reply = (b"OK protocol=1 mounted=1 suspended=0 pending=0"
                                 if command == b"STATUS" else b"OK")
                        self.received += request_id + b" " + reply + b"\r\n"

            def read(self, timeout):
                if self.received:
                    result, self.received = self.received, b""
                    return result
                clock[0] += timeout
                return None

        port, log = Port(), io.BytesIO()
        with patch("hardware.pico.time.monotonic", side_effect=lambda: clock[0]):
            pico = Pico(port, log)
            self.assertIn("mounted=1", pico.command("STATUS"))
            port.restart()
            self.assertEqual(pico.command("KEY 0 44"), "OK")
        self.assertEqual(port.executed, [b"STATUS", b"KEY 0 44"])
        self.assertEqual(len(port.writes), 2)
        self.assertTrue(all(data.startswith(b"\n") for data in port.writes))
        self.assertIn(b"invalid-or-long-line", log.getvalue())

    def test_busy_uart_has_actionable_error(self):
        master, slave = pty.openpty()
        try:
            fcntl.flock(slave, fcntl.LOCK_EX | fcntl.LOCK_NB)
            with self.assertRaisesRegex(RuntimeError, "close its serial terminal"):
                SerialPort(os.ttyname(slave))
        finally:
            os.close(master)
            os.close(slave)

    def test_correlated_exchange_events_and_error(self):
        master, slave = pty.openpty()
        port = None
        errors = []

        def peer():
            try:
                pending = b""
                for reply in ("OK protocol=1 mounted=1", "ERROR usb-unavailable"):
                    while True:
                        while b"\n" not in pending:
                            if not select.select([master], [], [], 2)[0]:
                                raise RuntimeError("test UART peer timed out")
                            pending += os.read(master, 1024)
                        request, pending = pending.split(b"\n", 1)
                        if request.strip():
                            break
                    request_id = request.split()[0]
                    os.write(master, b"0 EVENT ready\r\n1 OK\r\n" +
                             request_id + b" " + reply.encode() + b"\r\n")
            except (OSError, RuntimeError) as error:
                errors.append(str(error))

        thread = threading.Thread(target=peer)
        try:
            port = SerialPort(os.ttyname(slave))
            thread.start()
            log = io.BytesIO()
            pico = Pico(port, log)
            self.assertEqual(pico.command("STATUS"), "OK protocol=1 mounted=1")
            with self.assertRaisesRegex(RuntimeError, "usb-unavailable"):
                pico.command("KEY 0 44")
            self.assertIn(b"EVENT ready", log.getvalue())
            self.assertIn(b"TX", log.getvalue())
        finally:
            thread.join(3)
            if port:
                port.close()
            os.close(master)
            os.close(slave)
        self.assertFalse(thread.is_alive())
        self.assertEqual(errors, [])


class ApiTests(unittest.TestCase):
    def test_actual_endpoint_shapes_auth_and_no_overwrite(self):
        requests = []

        class Handler(BaseHTTPRequestHandler):
            def do_GET(self):
                self.reply()

            def do_POST(self):
                self.reply()

            def reply(self):
                body = self.rfile.read(int(self.headers.get("Content-Length", "0")))
                requests.append((self.command, self.path, dict(self.headers), body))
                self.send_response(200)
                self.end_headers()
                self.wfile.write(b'{"machine":"C64","vol":"SD:"}')

            def log_message(self, *args):
                return

        server = ThreadingHTTPServer(("127.0.0.1", 0), Handler)
        thread = threading.Thread(target=server.serve_forever)
        thread.start()
        try:
            api = Api(f"http://127.0.0.1:{server.server_port}", "1234")
            self.assertEqual(api.get_json("/api/status")["machine"], "C64")
            api.request("/api/fs/upload", b"\x01\x08", vol="SD:", path="/test.prg")
            api.request("/api/fs/autostart", b"", vol="SD:", path="/test.prg")
        finally:
            server.shutdown()
            thread.join()
            server.server_close()
        self.assertEqual([item[0] for item in requests], ["GET", "POST", "POST"])
        self.assertEqual(requests[1][3], b"\x01\x08")
        self.assertEqual(parse_qs(urlsplit(requests[1][1]).query),
                         {"vol": ["SD:"], "path": ["/test.prg"]})
        self.assertEqual(requests[1][2]["X-Bmc64-Web"], "1")
        self.assertEqual(requests[1][2]["Authorization"], "Basic Ym1jNjQ6MTIzNA==")
        for url in ("ftp://host", "http://user:secret@host", "http://host/path"):
            with self.assertRaises(ValueError):
                Api(url)


class CaptureTests(unittest.TestCase):
    def test_real_ffmpeg_with_synthetic_sources(self):
        for width, height in ((WIDTH, HEIGHT), (960, 720)):
            with self.subTest(width=width, height=height):
                self.capture_resolution(width, height)

    def capture_resolution(self, width, height):
        with tempfile.TemporaryDirectory() as directory:
            output = Path(directory)
            source = output / "source.mkv"
            subprocess.run([
                "ffmpeg", "-hide_banner", "-loglevel", "error", "-n",
                "-f", "lavfi", "-i", "color=black:size=480x360:rate=5",
                "-f", "lavfi", "-i", "sine=frequency=1000:sample_rate=48000",
                "-t", "8", "-c:v", "mjpeg", "-c:a", "pcm_s16le", str(source),
            ], check=True, capture_output=True, timeout=15)
            baseline = capture_command("unused", "unused", output, width, height)
            command = [
                "ffmpeg", "-hide_banner", "-loglevel", "error", "-n",
                "-re", "-i", str(source), "-re", "-i", str(source),
            ] + baseline[baseline.index("-map"):]
            capture = Capture(command, output, width, height)
            try:
                wait_for(capture, capture.check, 15, lambda: (
                    not capture.frames.empty() and not capture.audio.empty()
                ), "synthetic capture")
                _, frame = capture.frames.get(timeout=1)
                _, pcm = capture.audio.get(timeout=1)
                self.assertEqual(len(frame), width * height * 3)
                self.assertTrue(is_tone(audio_metrics(pcm)))
            finally:
                capture.close()
            self.assertGreater((output / "capture.mkv").stat().st_size, 1000)
            verified = subprocess.run([
                "ffmpeg", "-hide_banner", "-v", "error", "-i",
                str(output / "capture.mkv"), "-map", "0:v", "-map", "0:a",
                "-enc_time_base:v", "1:1000", "-fps_mode", "passthrough",
                "-f", "null", "-",
            ], capture_output=True, text=True, check=True, timeout=15)
            self.assertEqual(verified.stderr, "")

    def test_deadline_failure(self):
        class HealthyCapture:
            def check(self):
                pass

        with self.assertRaisesRegex(RuntimeError, "timeout: expected failure"):
            wait_for(HealthyCapture(), lambda: None, 0.03, lambda: False, "expected failure")

    def test_offline_mode_cannot_access_hardware(self):
        with tempfile.TemporaryDirectory() as directory:
            args = parser().parse_args([])
            report = {"run_id": f"{RUN_ID:08x}", "status": "NOT_RUN", "checks": [],
                      "cleanup_errors": []}
            # No rig: any hardware access would raise and fail the test.
            execute_case(DiagnosticCase(), TestContext(args, Path(directory), report))
            self.assertEqual(report["status"], "PREPARED", report.get("error"))
            self.assertTrue((Path(directory) / "diagnostic.prg").exists())


class RunnerTests(unittest.TestCase):
    def simulate(self, failure=None, active=("c64-test-de56af", "C64")):
        clock = [0.0]
        state = [0]
        commands = []
        requests = []
        current = {"profile_id": active[0], "machine": active[1], "uptime_secs": 500,
                   "profile_error": ""}
        tone = pcm_tone()
        quiet = bytes(SAMPLE_RATE * 2)
        frames = {
            (counter, value): render_marker(payload(RUN_ID, counter, value))
            for counter in (0, 1) for value in (0, 1)
        }

        def tick():
            clock[0] += 0.25
            return clock[0]

        class FakeApi:
            def __init__(self, *args):
                pass

            def get_json(self, endpoint, **query):
                requests.append(endpoint)
                if endpoint == "/api/status":
                    return dict(current)
                if query.get("path") == "/profiles":
                    return {"vol": "SD:", "truncated": False, "entries": [
                        {"name": "c64-test-de56af", "dir": True},
                        {"name": "pet-test-de56af", "dir": True}]}
                return {"vol": "SD:", "entries": [], "truncated": False}

            def request(self, endpoint, data=b"", **query):
                requests.append(endpoint)
                if endpoint == "/api/fs/download":
                    machine = "C64" if "c64" in query["path"] else "Pet"
                    return f"name={machine.lower()}_test\nmachine={machine}/PAL/HDMI\n".encode()
                if endpoint == "/api/profiles/start":
                    current.update(profile_id=query["id"], machine="C64", uptime_secs=1)
                    return b'{"ok":true}'
                if endpoint == "/api/fs/upload" and failure == "upload":
                    raise RuntimeError("simulated upload failure")
                return b""

        class FakePico:
            def __init__(self, *args):
                pass

            def command(self, command):
                commands.append(command)
                if command == "KEY 0 44" and failure != "input":
                    state[0] = 1
                return "OK protocol=1 mounted=1 suspended=0 pending=0"

        class FakeCapture:
            def __init__(self):
                self.width, self.height = WIDTH, HEIGHT
                self.frames = queue.Queue()
                self.audio = queue.Queue()
                self.count = 0
                self.latest_frame = None

            def check(self):
                self.count += 1
                timestamp = tick()
                counter = 0 if failure == "frozen" else self.count % 2
                self.latest_frame = (timestamp, frames[counter, state[0]])
                self.frames.put(self.latest_frame)
                sound = tone if state[0] == 0 or failure == "silence" else quiet
                self.audio.put((timestamp, sound))

        class FakeSuiteCapture:
            current = None
            width, height = WIDTH, HEIGHT
            latest_frame = None

            def open(self, output, size, health, timeout, label, video_size=None):
                self.current = FakeCapture()

            def check(self):
                if self.current:
                    self.current.check()
                    self.latest_frame = self.current.latest_frame

        with tempfile.TemporaryDirectory() as directory:
            args = parser().parse_args([
                "--execute", "--url", "http://localhost", "--pico-uart", "/unused",
                "--timeout", "12",
            ])
            report = {
                "run_id": f"{RUN_ID:08x}", "checks": [], "cleanup_errors": [],
                "status": "FAIL",
            }
            rig = Rig(api=FakeApi(), pico=FakePico(), capture=FakeSuiteCapture())
            profile = {"machine": "C64", "profile": "c64-test-de56af", "id": "c64-test-de56af",
                       "name": "c64_test", "volume": "SD:"}
            with patch("tests.basic.test_c64_default.time.monotonic", tick), \
                    patch("tests.basic.test_c64_default.time.sleep"):
                execute_case(DiagnosticCase(), TestContext(
                    args, Path(directory), report, rig, profile, rig.capture.check))
        self.assertEqual(commands[-1], "RELEASE")
        return report, requests, commands

    def test_complete_run_requires_all_assertions(self):
        report, requests, commands = self.simulate()
        self.assertEqual(report["status"], "PASS")
        self.assertEqual(report["checks"], [
            "profile_and_core", "current_run_id", "guest_progress", "sid_tone",
            "space_round_trip", "sid_silence",
        ])
        self.assertIn("/api/fs/autostart", requests)
        self.assertIn("KEY 0 44", commands)

    def test_switches_to_the_c64_test_profile_only_when_needed(self):
        report, requests, _ = self.simulate()
        self.assertNotIn("/api/profiles/start", requests)
        self.assertNotIn("/api/reboot", requests)
        report, requests, _ = self.simulate(active=("pet-test-de56af", "Pet"))
        self.assertEqual(requests.count("/api/profiles/start"), 1)
        self.assertLess(requests.index("/api/profiles/start"), requests.index("/api/fs/upload"))
        self.assertEqual(report["status"], "PASS")

    def test_upload_failure_never_sends_input(self):
        report, requests, commands = self.simulate("upload")
        self.assertNotIn("/api/fs/autostart", requests)
        self.assertNotIn("KEY 0 44", commands)
        self.assertEqual(report["stage"], "upload_autostart")
        self.assertNotEqual(report["status"], "PASS")

    def test_frozen_counter_never_sends_input(self):
        report, _, commands = self.simulate("frozen")
        self.assertNotIn("KEY 0 44", commands)
        self.assertEqual(report["stage"], "guest_ready")
        self.assertNotEqual(report["status"], "PASS")

    def test_usb_ack_without_guest_response_fails(self):
        report, _, commands = self.simulate("input")
        self.assertIn("KEY 0 44", commands)
        self.assertEqual(report["stage"], "input_round_trip")
        self.assertNotEqual(report["status"], "PASS")

    def test_tone_must_stop_after_visible_response(self):
        report, _, _ = self.simulate("silence")
        self.assertEqual(report["stage"], "audio_silence")
        self.assertNotEqual(report["status"], "PASS")


if __name__ == "__main__":
    unittest.main()
