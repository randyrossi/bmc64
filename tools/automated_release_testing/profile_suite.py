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

"""Profile boot and real USB typing suite, invoked by run_smoke.py --suite."""

import argparse
import getpass
import hashlib
import json
import re
import secrets
import signal
import subprocess
import sys
import termios
import time
from contextlib import ExitStack
from datetime import datetime, timezone
from pathlib import Path
from typing import Callable
from urllib.error import HTTPError, URLError

from harness import TestFailure, WaitProgress, frame_health, progress, save_frame
from run_smoke import (
    Api, Capture, Pico, SerialPort, UartLog, capture_command, failure_evidence,
    save_json,
)
from screen_text import LAYOUTS, Font, Reading, ScreenText


MACHINES = tuple(LAYOUTS)
ERRORS = (OSError, ValueError, RuntimeError, URLError, subprocess.SubprocessError,
          termios.error)
# Capture at the core's own HDMI resolution so the capture device never
# rescales it: 80-column PET text does not survive 1080p -> 720p.
CAPTURE_SIZES = {"Pet": (1920, 1080)}
DEFAULT_CAPTURE_SIZE = (1280, 720)


def read_manifest(path: Path) -> list[dict[str, str]]:
    value = json.loads(path.read_text(encoding="utf-8"))
    if not isinstance(value, list) or not value:
        raise ValueError("suite manifest must be a nonempty JSON array")
    seen = set()
    for item in value:
        if (not isinstance(item, dict) or set(item) != {"machine", "profile"}
                or not isinstance(item["machine"], str) or item["machine"] not in MACHINES
                or not isinstance(item["profile"], str) or not item["profile"].strip()
                or any(char in item["profile"] for char in "\r\n\0")):
            raise ValueError("each suite case needs a supported machine and a profile name/ID")
        if item["machine"] in seen:
            raise ValueError(f"duplicate machine: {item['machine']}")
        seen.add(item["machine"])
    return value


def profile_values(data: bytes) -> dict[str, str]:
    result = {}
    for line in data.decode("utf-8-sig").splitlines():
        line = line.strip()
        if not line or line.startswith(("#", ";")):
            continue
        key, separator, value = line.partition("=")
        if not separator:
            raise ValueError("invalid profile.txt line: " + line)
        key = key.strip()
        if key in result:
            raise ValueError("duplicate profile.txt key: " + key)
        result[key] = value.strip()
    return result


def resolve_profiles(api: Api, manifest: list[dict[str, str]]) -> list[dict[str, str]]:
    listing = api.get_json("/api/fs/list", path="/profiles")
    entries, volume = listing.get("entries"), listing.get("vol")
    if (not isinstance(entries, list) or not isinstance(volume, str) or not volume
            or listing.get("truncated") is not False):
        raise TestFailure("profile_preflight", "invalid or truncated /profiles listing")
    profiles = []
    for entry in entries:
        if not isinstance(entry, dict):
            raise TestFailure("profile_preflight", "malformed /profiles directory entry")
        identity = entry.get("name")
        if entry.get("dir") is not True or identity == "main":
            continue
        if not isinstance(identity, str) or not re.fullmatch(r"[a-z0-9-]{1,32}", identity):
            raise TestFailure("profile_preflight", f"invalid profile directory ID: {identity!r}")
        values = profile_values(api.request(
            "/api/fs/download", vol=volume, path=f"/profiles/{identity}/profile.txt"))
        profiles.append({**values, "id": identity})
    resolved = []
    for case in manifest:
        matches = [item for item in profiles
                   if case["profile"] in (item["id"], item.get("name"))]
        if len(matches) != 1:
            raise TestFailure("profile_preflight",
                              f"{case['profile']}: expected one name/ID match; found {len(matches)}")
        profile = matches[0]
        machine = profile.get("machine", "").split("/", 1)[0]
        if machine != case["machine"]:
            raise TestFailure("profile_preflight",
                              f"{profile['id']}: expected {case['machine']}, configured {machine}")
        attached = [key for key, value in profile.items()
                    if value and (key == "autostart" or key.startswith("disk_"))]
        if attached:
            raise TestFailure("profile_preflight",
                              f"{profile['id']}: remove test-profile media: {', '.join(attached)}")
        resolved.append({**case, "id": profile["id"], "name": profile.get("name", ""),
                         "volume": volume})
    if len({case["id"] for case in resolved}) != len(resolved):
        raise TestFailure("profile_preflight", "multiple cases resolve to the same profile")
    return resolved


def status(api: Api) -> dict[str, object]:
    value = api.get_json("/api/status")
    uptime = value.get("uptime_secs")
    if (not isinstance(value.get("profile_id"), str)
            or not re.fullmatch(r"[a-z0-9-]{1,32}", value["profile_id"])
            or not isinstance(value.get("machine"), str) or value["machine"] not in MACHINES
            or isinstance(uptime, bool) or not isinstance(uptime, int) or uptime < 0
            or not isinstance(value.get("profile_error"), str)):
        raise TestFailure("api_status", "status lacks valid profile, machine, uptime or profile_error")
    return value


def transient(error: OSError) -> bool:
    return not isinstance(error, HTTPError) or error.code >= 500


def recover_api(api: Api, timeout: float, label: str = "HTTP control") -> dict[str, object]:
    deadline = time.monotonic() + timeout
    last_error = "no response"
    updates = WaitProgress(label, timeout)
    while time.monotonic() < deadline:
        try:
            current = status(api)
        except OSError as error:
            if not transient(error):
                raise
            last_error = str(error)
        else:
            updates.update("available", force=True)
            return current
        updates.update("still waiting for the status endpoint")
        time.sleep(0.3)
    raise TestFailure("http_unavailable", f"{label} did not return: {last_error}")


def switch_profile(api: Api, case: dict[str, str], timeout: float,
                   health: Callable[[], None], output: Path) -> dict[str, object]:
    before = status(api)
    save_json(output / "status-before.json", before)
    restarting = before["profile_id"] == case["id"]
    endpoint = "/api/reboot" if restarting else "/api/profiles/start"
    query = {} if restarting else {"id": case["id"], "once": "1"}
    action = "reboot" if restarting else "profile start"
    progress(f"{case['machine']}: {action} request via {endpoint}")
    requested = time.monotonic()
    updates = WaitProgress(f"{case['machine']} profile switch to {case['id']}", timeout)
    try:
        reply = json.loads(api.request(endpoint, b"", **query))
    except HTTPError:
        raise
    except OSError as error:
        # A restart can drop the HTTP reply after the start request was applied.
        save_json(output / "switch-request.json", {
            "endpoint": endpoint, "outcome": "unknown", "error": str(error), "retried": False,
        })
        print(f"{case['machine']}: {action} reply lost ({error}); "
              "verifying profile boot without retrying the request", file=sys.stderr, flush=True)
    else:
        if not isinstance(reply, dict) or reply.get("ok") is not True:
            raise TestFailure("profile_switch", f"{action} was not accepted")
        save_json(output / "switch-request.json", {"endpoint": endpoint, "outcome": "accepted"})
    deadline = requested + timeout
    last = before
    detail = "waiting for restart and verified profile/core"
    with (output / "status-poll.jsonl").open("w", encoding="utf-8") as log:
        while time.monotonic() < deadline:
            updates.update(detail)
            health()
            try:
                current = status(api)
            except OSError as error:
                if not transient(error):
                    raise
                log.write(json.dumps({"monotonic": time.monotonic(), "error": str(error)}) + "\n")
                detail = "HTTP temporarily unavailable during restart"
            else:
                last = current
                detail = (f"reported {current['profile_id']} / {current['machine']}, "
                          f"uptime {current['uptime_secs']}s; waiting for verified transition")
                log.write(json.dumps({"monotonic": time.monotonic(), **current}) + "\n")
                transitioned = (before["profile_id"] != current["profile_id"]
                                or current["uptime_secs"] < before["uptime_secs"]
                                or current["uptime_secs"] < time.monotonic() - requested - 1)
                # Reboot does not clear the previous start error until the new boot.
                stale_error = (restarting and not transitioned
                               and current["profile_error"] == before["profile_error"])
                if current["profile_error"] and not stale_error:
                    raise TestFailure("profile_switch", f"profile start failed: {current['profile_error']}")
                if (current["profile_id"] == case["id"]
                        and current["machine"] == case["machine"] and transitioned):
                    save_json(output / "status-booted.json", current)
                    updates.update("profile and core verified", force=True)
                    return current
            log.flush()
            time.sleep(0.3)
    raise TestFailure("profile_switch_timeout",
                      f"no verified switch to {case['id']} / {case['machine']}; last status: {last}")


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


class SuiteCapture:
    """The current HDMI recording segment.

    A capture left open across a profile switch has recorded corrupted, tiled
    frames after the HDMI mode changed. Each case therefore reopens the
    capture device once its core is running, so it locks to the new signal.
    """

    def __init__(self, video: str, audio: str):
        self.video, self.audio = video, audio
        self.current: Capture | None = None
        self.errors: list[str] = []

    @property
    def latest_frame(self) -> tuple[float, bytes] | None:
        return self.current.latest_frame if self.current else None

    @property
    def width(self) -> int:
        return self.current.width if self.current else DEFAULT_CAPTURE_SIZE[0]

    @property
    def height(self) -> int:
        return self.current.height if self.current else DEFAULT_CAPTURE_SIZE[1]

    def check(self) -> None:
        if self.current:
            self.current.check()

    def close(self) -> None:
        current, self.current = self.current, None
        if current:
            try:
                current.close()
            except ERRORS as error:
                self.errors.append(str(error))

    def open(self, output: Path, size: tuple[int, int], health: Callable[[], None],
             timeout: float, label: str) -> None:
        self.close()
        output.mkdir(exist_ok=True)
        width, height = size
        progress(f"{label}: opening HDMI capture at {width}x{height} in {output.name}/")
        self.current = Capture(capture_command(self.video, self.audio, output, width, height,
                                               f"{width}x{height}"),
                               output, width, height)
        deadline = time.monotonic() + timeout
        updates = WaitProgress(f"{label} HDMI capture startup", timeout)
        while time.monotonic() < deadline:
            health()
            latest = self.current.latest_frame
            if latest and self.current.latest_audio is not None:
                metrics = frame_health(latest[1], width, height)
                if not metrics["black"] and not metrics["flat"]:
                    updates.update("nonblack video and audio received", force=True)
                    return
            updates.update("waiting for nonblack video and audio")
            time.sleep(0.1)
        raise TestFailure("capture_startup", f"{label}: no nonblack HDMI video and audio "
                          f"within {timeout:g}s of opening the capture")


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


def wait_pico(pico: Pico, health: Callable[[], None], timeout: float) -> None:
    deadline = time.monotonic() + timeout
    last = ""
    updates = WaitProgress("Pico USB keyboard ready/remounted", timeout)
    while time.monotonic() < deadline:
        health()
        last = pico.command("STATUS")
        fields = dict(part.split("=", 1) for part in last.split()[1:] if "=" in part)
        if fields.get("protocol") != "1":
            raise TestFailure("pico_protocol", f"unsupported Pico protocol: {last}")
        if all(fields.get(key) == value for key, value in {
                "mounted": "1", "suspended": "0", "pending": "0"}.items()):
            updates.update("ready", force=True)
            return
        updates.update(last)
        time.sleep(0.25)
    raise TestFailure("pico_unavailable", f"Pico did not remount after profile boot: {last}")


def type_token(pico: Pico, token: str, health: Callable[[], None]) -> None:
    if not re.fullmatch(r"[A-Z]{8}", token):
        raise ValueError("typing token must be eight uppercase ASCII letters")
    for letter in token:
        health()
        pico.command(f"KEY 0 {ord(letter) - ord('A') + 4}")
        time.sleep(0.12)
        pico.command("KEY 0")
        time.sleep(0.12)


def run_case(api: Api, pico: Pico, capture: SuiteCapture, health: Callable[[], None],
             case: dict[str, str], timeout: float, output: Path, report: dict) -> None:
    machine = case["machine"]
    layout = LAYOUTS[machine]
    report["stage"] = "font_rom"
    progress(f"{machine}: reading character ROM {layout.font_path} from the SD card")
    data = api.request("/api/fs/download", vol=case["volume"], path=layout.font_path)
    report["font"] = {"path": layout.font_path, "offset": layout.font_offset,
                      "bytes": len(data), "sha256": hashlib.sha256(data).hexdigest()}
    size = CAPTURE_SIZES.get(machine, DEFAULT_CAPTURE_SIZE)
    reader = ScreenText(machine, Font(data, layout.font_offset), *size)
    token = "".join(secrets.choice(reader.alphabet) for _ in range(8))
    report.update(token=token, typing_method="rom_glyphs", typing_alphabet=reader.alphabet)
    report["stage"] = "profile_switch"
    switch_profile(api, case, timeout, health, output)
    report["checks"].append("profile_and_core")
    report["stage"] = "capture_reopen"
    capture.open(output / "capture-1", size, health, timeout, machine)
    segments = [1]

    def reopen() -> None:
        segments.append(len(segments) + 1)
        capture.open(output / f"capture-{segments[-1]}", size, health, timeout, machine)

    def bounds(rgb: bytes) -> tuple[int, int, int, int]:
        return screen_bounds(rgb, *size)

    report["stage"] = "basic_boot"
    progress(f"{machine}: checking the BASIC boot screen cell by cell against the ROM font")
    reading = wait_screen(capture, health, timeout, time.monotonic(), output, "boot",
                          lambda rgb: reader.read(rgb, bounds(rgb)), machine, reopen)
    report["screen_grid"] = reading.grid
    report["checks"].append("basic_ready")
    report["stage"] = "pico_remount"
    wait_pico(pico, health, timeout)
    pico.command("RELEASE")
    report["stage"] = "typing"
    progress(f"{machine}: typing token {token!r} through USB (no Return); "
             f"alphabet {reader.alphabet}")
    type_token(pico, token, health)
    reading = wait_screen(capture, health, timeout, time.monotonic(), output, "typing",
                          lambda rgb: reader.read(rgb, bounds(rgb), token), machine,
                          reopen if len(segments) == 1 else None)
    report["typing_glyph_distances"] = reading.distances
    report["checks"].append("usb_typing_visible")
    report["stage"] = "final_status"
    progress(f"{machine}: typing verified (bit distances {reading.distances}); "
             "checking final profile/core")
    final = recover_api(api, timeout, f"{machine} final profile/core status")
    save_json(output / "status-after.json", final)
    if (final["profile_id"] != case["id"] or final["machine"] != machine
            or final["profile_error"]):
        raise TestFailure("profile_changed", "profile/core changed during the typing assertion")
    health()
    report.update(status="PASS", stage="complete")


def rotate(api: Api, pico: Pico, capture: SuiteCapture, uart: UartLog | None,
           cases: list[dict[str, str]], timeout: float, output: Path, report: dict) -> None:
    blocked = None
    for index, case in enumerate(cases, 1):
        result = {**case, "name": case_name(case["machine"]), "status": "NOT_RUN",
                  "checks": [], "cleanup_errors": []}
        report["cases"].append(result)
        if blocked:
            result["error"] = blocked
            progress(f"[{index}/{len(cases)}] {case['machine']}: NOT_RUN; {blocked}")
            continue
        progress(f"[{index}/{len(cases)}] {case['machine']}: starting profile {case['id']}")
        directory = output / f"{index:02d}-{case['machine'].lower()}"
        directory.mkdir()
        result.update(status="FAIL", evidence=directory.name,
                      started_monotonic=time.monotonic())
        uart_start = len(uart.findings) if uart else 0

        def health() -> None:
            capture.check()
            if uart:
                uart.check(uart_start)

        try:
            run_case(api, pico, capture, health, case, timeout, directory, result)
        except ERRORS as error:
            result.update(error=str(error), category=getattr(error, "category", "fixture_or_api"))
            print(f"FAIL {case['machine']} [{result.get('stage')}]: {error}", file=sys.stderr)
        except KeyboardInterrupt:
            result.update(error="interrupted", category="interrupted")
            raise
        finally:
            progress(f"{case['machine']}: releasing USB input and saving case evidence")
            try:
                pico.command("RELEASE")
            except ERRORS as error:
                result["cleanup_errors"].append(f"release inputs: {error}")
                result["status"] = "FAIL"
            if uart:
                try:
                    uart.check(uart_start)
                except ERRORS as error:
                    if result["status"] == "PASS":
                        result.update(error=str(error),
                                      category=getattr(error, "category", "uart_error"))
                    result["status"] = "FAIL"
            result["finished_monotonic"] = time.monotonic()
            if uart:
                result["uart_findings"] = uart.findings[uart_start:]
            failure_evidence(capture, directory, result)
            save_json(directory / "result.json", result)
        print(f"{result['status']}: {case['machine']}", flush=True)
        if result["status"] != "PASS":
            progress(f"{case['machine']}: checking HTTP control before continuing")
            try:
                recover_api(api, timeout)
            except ERRORS as error:
                blocked = f"HTTP control unavailable after {case['machine']}: {error}"
                report["control_error"] = blocked
                print(blocked, file=sys.stderr)


def execute(args: argparse.Namespace, output: Path, report: dict, stack: ExitStack) -> None:
    def register(label: str, action: Callable[[], object]) -> None:
        def cleanup() -> None:
            progress(f"Cleanup: {label}")
            try:
                action()
            except ERRORS as error:
                report["cleanup_errors"].append(f"{label}: {error}")
                print(f"{label}: {error}", file=sys.stderr)
        stack.callback(cleanup)

    api = Api(args.url, getpass.getpass("BMC64 PIN: ") if args.ask_pin else None, timeout=3)
    report["stage"] = "profile_preflight"
    progress("Preflight: checking HTTP status and resolving selected SD-card profiles")
    save_json(output / "status-before.json", status(api))
    cases = resolve_profiles(api, report["manifest"])
    save_json(output / "resolved-profiles.json", cases)
    progress(f"Preflight complete: {len(cases)} case(s): " +
             ", ".join(case["machine"] for case in cases))
    report["stage"] = "fixture"
    progress("Opening Pico UART and checking USB fixture")
    port = SerialPort(args.pico_uart)
    register("close Pico UART", port.close)
    log = stack.enter_context((output / "pico.log").open("wb"))
    pico = Pico(port, log)
    port.write(b"\n")
    register("release inputs", lambda: pico.command("RELEASE"))
    wait_pico(pico, lambda: None, args.timeout)
    pico.command("RELEASE")
    uart = None
    if args.bmc_uart:
        progress("Starting BMC64 UART logging")
        diagnostic_port = SerialPort(args.bmc_uart)
        register("close BMC64 UART", diagnostic_port.close)
        uart = UartLog(diagnostic_port, output / "bmc64-uart.log")
        register("save UART classification", lambda: save_json(
            output / "uart-findings.json", uart.findings))
        register("check final UART messages", uart.check)
        register("stop UART logger", uart.close)
    report["uart_collected"] = uart is not None
    report["stage"] = "capture_startup"
    progress("Starting HDMI video/audio recording; each case reopens it after its core boots")
    capture = SuiteCapture(args.video, args.audio)

    def finish_capture() -> None:
        capture.close()
        if capture.errors:
            raise RuntimeError("; ".join(capture.errors))

    register("save capture evidence", lambda: failure_evidence(capture, output, report))
    register("finish capture", finish_capture)

    def startup_health() -> None:
        capture.check()
        if uart:
            uart.check()

    capture.open(output, DEFAULT_CAPTURE_SIZE, startup_health, min(15, args.timeout), "Startup")
    report["stage"] = "profile_cases"
    rotate(api, pico, capture, uart, cases, args.timeout, output, report)
    if report.get("control_error"):
        return
    progress("All selected cases finished; checking final HTTP status")
    save_json(output / "status-after.json", recover_api(api, args.timeout))
    if len(report["cases"]) == len(cases) and all(case["status"] == "PASS" for case in report["cases"]):
        report.update(status="PASS", stage="complete")


def case_name(machine: str) -> str:
    return f"{machine.lower()}-boot-typing"


def main(args: argparse.Namespace, manifest: list[dict[str, str]]) -> dict:
    """Run the given manifest cases; returns the saved report."""
    run_id = secrets.token_hex(4)
    output = args.results / (datetime.now(timezone.utc).strftime("%Y%m%dT%H%M%SZ")
                             + "-suite-" + run_id)
    output.mkdir(parents=True, exist_ok=False)
    report = {
        "schema": 1, "run_id": run_id, "kind": "profile_suite", "status": "FAIL",
        "stage": "starting", "manifest": manifest, "cases": [], "cleanup_errors": [],
        "setup": args.setup, "url": args.url, "video": args.video, "audio": args.audio,
        "pico_uart": args.pico_uart, "bmc_uart": args.bmc_uart,
        "started_utc": datetime.now(timezone.utc).isoformat(),
    }
    print(f"Evidence: {output}", flush=True)

    def interrupted(signum, frame):
        raise KeyboardInterrupt

    previous = signal.signal(signal.SIGTERM, interrupted)
    try:
        if not args.execute:
            report.update(status="PREPARED", stage="offline", detail="Manifest valid; no hardware accessed")
        else:
            with ExitStack() as stack:
                try:
                    execute(args, output, report, stack)
                except ERRORS as error:
                    report.update(error=str(error), category=getattr(error, "category", "fixture_or_api"))
                    print(f"FAIL [{report['stage']}]: {error}", file=sys.stderr)
                except KeyboardInterrupt:
                    report.update(error="interrupted", category="interrupted")
            if report["cleanup_errors"]:
                report["status"] = "FAIL"
    finally:
        signal.signal(signal.SIGTERM, previous)
        completed = {case["machine"] for case in report["cases"]}
        report["cases"].extend({
            **case, "name": case_name(case["machine"]),
            **({"status": "PREPARED", "detail": "offline preparation; hardware not accessed"}
               if report["status"] == "PREPARED" else
               {"status": "NOT_RUN", "error": report.get("error", "suite stopped before this case")}),
        } for case in manifest if case["machine"] not in completed)
        report["finished_utc"] = datetime.now(timezone.utc).isoformat()
        save_json(output / "result.json", report)
    report["evidence"] = str(output / "result.json")
    print(f"{report['status']}: {output / 'result.json'}")
    return report
