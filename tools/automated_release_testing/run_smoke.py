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

"""First real-hardware C64 smoke test. Hardware access requires --execute."""

import argparse
import base64
import fcntl
import getpass
import hashlib
import json
import math
import os
import queue
import secrets
import select
import shutil
import signal
import subprocess
import sys
import termios
import threading
import time
from contextlib import ExitStack
from datetime import datetime, timezone
from pathlib import Path
from typing import BinaryIO, Callable
from urllib.error import URLError
from urllib.parse import urlencode, urlsplit
from urllib.request import Request, build_opener, HTTPRedirectHandler, ProxyHandler

from diagnostic import (
    HEIGHT, SAMPLE_RATE, WIDTH, Marker, audio_metrics, build_guest,
    decode_frame, is_silence, is_tone,
)
from harness import (
    TestFailure, WaitProgress, classify_uart, frame_health, print_summary, progress, save_frame,
)


HERE = Path(__file__).resolve().parent


def save_json(path: Path, value: object) -> None:
    path.write_text(json.dumps(value, indent=2) + "\n", encoding="utf-8")


class NoRedirect(HTTPRedirectHandler):
    def redirect_request(self, req, fp, code, msg, headers, newurl):
        raise RuntimeError("unexpected HTTP redirect; check the BMC64 base URL")


class Api:
    def __init__(self, url: str, pin: str | None = None, timeout: float = 15):
        parsed = urlsplit(url)
        if (parsed.scheme not in ("http", "https") or not parsed.hostname
                or parsed.username or parsed.password or parsed.query
                or parsed.fragment or parsed.path not in ("", "/")):
            raise ValueError("use a base HTTP(S) URL without credentials or a path")
        self.url = url.rstrip("/")
        self.timeout = timeout
        self.headers = {"X-BMC64-Web": "1"}
        if pin is not None:
            encoded = base64.b64encode(f"bmc64:{pin}".encode()).decode("ascii")
            self.headers["Authorization"] = f"Basic {encoded}"
        self.opener = build_opener(ProxyHandler({}), NoRedirect())

    def request(self, endpoint: str, data: bytes | None = None,
                **query: str) -> bytes:
        suffix = "?" + urlencode(query) if query else ""
        request = Request(self.url + endpoint + suffix, data=data,
                          headers=self.headers)
        with self.opener.open(request, timeout=self.timeout) as response:
            return response.read()

    def get_json(self, endpoint: str, **query: str) -> dict[str, object]:
        result = json.loads(self.request(endpoint, **query))
        if not isinstance(result, dict):
            raise RuntimeError(f"{endpoint} did not return a JSON object")
        return result


class SerialPort:
    def __init__(self, device: str):
        self.fd = os.open(device, os.O_RDWR | os.O_NOCTTY | os.O_NONBLOCK)
        self.original = None
        try:
            try:
                fcntl.flock(self.fd, fcntl.LOCK_EX | fcntl.LOCK_NB)
            except BlockingIOError as error:
                raise RuntimeError(
                    f"{device} is locked by another process; close its serial terminal"
                ) from error
            fcntl.ioctl(self.fd, termios.TIOCEXCL)
            self.original = termios.tcgetattr(self.fd)
            settings = termios.tcgetattr(self.fd)
            settings[:6] = [0, 0, termios.CS8 | termios.CLOCAL | termios.CREAD,
                            0, termios.B115200, termios.B115200]
            settings[6][termios.VMIN] = 0
            settings[6][termios.VTIME] = 0
            termios.tcsetattr(self.fd, termios.TCSANOW, settings)
        except (OSError, termios.error, RuntimeError):
            os.close(self.fd)
            raise

    def write(self, data: bytes) -> None:
        deadline = time.monotonic() + 4
        while data:
            remaining = deadline - time.monotonic()
            if remaining <= 0 or not select.select([], [self.fd], [], remaining)[1]:
                raise RuntimeError("UART write timeout; outcome unknown")
            try:
                count = os.write(self.fd, data)
            except BlockingIOError:
                continue
            if count == 0:
                raise RuntimeError("UART write returned zero")
            data = data[count:]

    def read(self, timeout: float) -> bytes | None:
        if not select.select([self.fd], [], [], timeout)[0]:
            return None
        data = os.read(self.fd, 4096)
        if not data:
            raise RuntimeError("UART disconnected")
        return data

    def close(self) -> None:
        try:
            if self.original is not None:
                termios.tcsetattr(self.fd, termios.TCSANOW, self.original)
            fcntl.ioctl(self.fd, termios.TIOCNXCL)
        finally:
            os.close(self.fd)


class Pico:
    def __init__(self, port: SerialPort, log: BinaryIO):
        self.port, self.log = port, log
        self.request_id = secrets.randbelow(1_000_000_000) + 1
        self.buffer = b""

    def command(self, command: str) -> str:
        self.request_id += 1
        prefix = f"{self.request_id} "
        self.log.write(f"{time.monotonic():.6f} TX {prefix}{command}\n".encode())
        self.log.flush()
        self.port.write(f"{prefix}{command}\n".encode("ascii"))
        deadline = time.monotonic() + 4
        while time.monotonic() < deadline:
            while b"\n" in self.buffer:
                raw, self.buffer = self.buffer.split(b"\n", 1)
                line = raw.rstrip(b"\r").decode("ascii", errors="replace")
                self.log.write(f"{time.monotonic():.6f} RX {line}\n".encode())
                self.log.flush()
                if line.startswith(prefix):
                    reply = line[len(prefix):]
                    if reply != "OK" and not reply.startswith("OK "):
                        raise RuntimeError(f"Pico rejected {command}: {reply}")
                    return reply
            data = self.port.read(max(0, deadline - time.monotonic()))
            if data:
                self.buffer += data
                if len(self.buffer) > 16384:
                    raise RuntimeError("oversized Pico UART input")
        raise RuntimeError(f"Pico timeout for {command}; outcome unknown, not retried")


class UartLog:
    def __init__(self, port: SerialPort, output: Path):
        self.port = port
        self.stop = threading.Event()
        self.errors: list[str] = []
        self.findings: list[dict[str, object]] = []
        self.file = output.open("wb")
        self.thread = threading.Thread(target=self.collect, daemon=True)
        self.thread.start()

    def collect(self) -> None:
        line_start = True
        pending = b""
        try:
            while not self.stop.is_set():
                data = self.port.read(0.2)
                if data:
                    pending += data
                    while b"\n" in pending:
                        line, pending = pending.split(b"\n", 1)
                        finding = classify_uart(line.decode("utf-8", errors="replace").strip())
                        if finding:
                            self.findings.append({"monotonic": time.monotonic(), **finding})
                    if len(pending) > 16384:
                        self.errors.append("UART line exceeded 16384 bytes")
                        return
                    while data:
                        if line_start:
                            self.file.write(f"[{time.monotonic():.6f}] ".encode())
                        text, newline, data = data.partition(b"\n")
                        self.file.write(text + newline)
                        line_start = bool(newline)
                    self.file.flush()
        except (OSError, RuntimeError) as error:
            self.errors.append(str(error))
        finally:
            if pending:
                finding = classify_uart(pending.decode("utf-8", errors="replace").strip())
                if finding:
                    self.findings.append({"monotonic": time.monotonic(), **finding})

    def check(self, since: int = 0) -> None:
        if self.errors:
            raise TestFailure("uart_transport", "BMC64 UART collector: " + "; ".join(self.errors))
        critical = [item for item in self.findings[since:]
                    if item["severity"] == "critical"]
        if critical:
            raise TestFailure("uart_critical", f"critical UART message: {critical[0]['line']}")

    def close(self) -> None:
        self.stop.set()
        self.thread.join(2)
        self.file.close()
        if self.errors:
            raise RuntimeError("BMC64 UART collector: " + "; ".join(self.errors))
        if self.thread.is_alive():
            raise RuntimeError("BMC64 UART collector did not stop")


class Capture:
    def __init__(self, command: list[str], output: Path,
                 width: int = WIDTH, height: int = HEIGHT):
        self.width, self.height = width, height
        self.frames: queue.Queue[tuple[float, bytes]] = queue.Queue(maxsize=4)
        self.audio: queue.Queue[tuple[float, bytes]] = queue.Queue(maxsize=4)
        self.errors: list[str] = []
        self.latest_frame: tuple[float, bytes] | None = None
        self.latest_audio: tuple[float, bytes] | None = None
        self.log = (output / "ffmpeg.log").open("wb")
        audio_read, audio_write = os.pipe()
        self.audio_pipe = os.fdopen(audio_read, "rb")
        command = [part.replace("{audio_fd}", str(audio_write)) for part in command]
        save_json(output / "capture-command.json", command)
        try:
            self.process = subprocess.Popen(
                command, stdin=subprocess.PIPE, stdout=subprocess.PIPE,
                stderr=self.log, pass_fds=(audio_write,),
            )
        except OSError:
            self.audio_pipe.close()
            self.log.close()
            raise
        finally:
            os.close(audio_write)
        assert self.process.stdout is not None
        self.threads = [
            threading.Thread(target=self.collect, args=(
                self.process.stdout, width * height * 3, self.frames), daemon=True),
            threading.Thread(target=self.collect, args=(
                self.audio_pipe, SAMPLE_RATE * 2, self.audio), daemon=True),
        ]
        for thread in self.threads:
            thread.start()

    def collect(self, pipe: BinaryIO, size: int,
                target: queue.Queue[tuple[float, bytes]]) -> None:
        try:
            while True:
                data = pipe.read(size)
                if not data:
                    return
                if len(data) != size:
                    self.errors.append("capture ended with an incomplete frame/audio block")
                    return
                item = (time.monotonic(), data)
                if target is self.frames:
                    self.latest_frame = item
                elif target is self.audio:
                    self.latest_audio = item
                try:
                    target.put_nowait(item)
                except queue.Full:
                    try:
                        target.get_nowait()
                    except queue.Empty:
                        pass
                    target.put_nowait(item)
        except OSError as error:
            self.errors.append(str(error))

    def check(self) -> None:
        if self.errors:
            raise TestFailure("capture_reader", "capture reader: " + "; ".join(self.errors))
        if self.process.poll() is not None:
            raise TestFailure("capture_process",
                              f"FFmpeg exited ({self.process.returncode}); see ffmpeg.log")

    def close(self) -> None:
        forced = False
        try:
            if self.process.poll() is None:
                assert self.process.stdin is not None
                try:
                    self.process.stdin.write(b"q\n")
                    self.process.stdin.flush()
                except BrokenPipeError:
                    self.process.wait(timeout=2)
                try:
                    self.process.wait(timeout=8)
                except subprocess.TimeoutExpired:
                    forced = True
                    self.process.terminate()
                    try:
                        self.process.wait(timeout=2)
                    except subprocess.TimeoutExpired:
                        self.process.kill()
                        self.process.wait(timeout=2)
            for thread in self.threads:
                thread.join(2)
            if forced or any(thread.is_alive() for thread in self.threads):
                raise RuntimeError("FFmpeg required forced shutdown; recording may be incomplete")
            if self.process.returncode != 0:
                raise RuntimeError(f"FFmpeg failed ({self.process.returncode}); see ffmpeg.log")
        finally:
            if self.process.stdin:
                self.process.stdin.close()
            if self.process.stdout:
                self.process.stdout.close()
            self.audio_pipe.close()
            self.log.close()


def capture_command(video: str, audio: str, output: Path,
                    width: int = WIDTH, height: int = HEIGHT,
                    video_size: str = "1280x720") -> list[str]:
    return [
        "ffmpeg", "-hide_banner", "-loglevel", "warning", "-n",
        "-thread_queue_size", "512", "-f", "v4l2", "-input_format", "mjpeg",
        "-video_size", video_size, "-framerate", "30", "-i", video,
        "-thread_queue_size", "512", "-f", "alsa", "-i", audio,
        "-map", "0:v:0", "-map", "1:a:0", "-c:v", "copy", "-c:a", "pcm_s16le",
        "-af", "asetpts=N/SR/TB",
        str(output / "capture.mkv"),
        "-map", "0:v:0", "-vf", f"fps=5,scale={width}:{height}",
        "-pix_fmt", "rgb24", "-f", "rawvideo", "pipe:1",
        "-map", "1:a:0", "-ac", "1", "-ar", str(SAMPLE_RATE),
        "-af", "asetpts=N/SR/TB",
        "-c:a", "pcm_s16le", "-f", "s16le", "pipe:{audio_fd}",
    ]


def wait_for(capture: Capture, uart: UartLog | None, timeout: float,
             predicate: Callable[[], bool], description: str) -> None:
    deadline = time.monotonic() + timeout
    updates = WaitProgress(description, timeout)
    while time.monotonic() < deadline:
        capture.check()
        if uart:
            uart.check()
        if predicate():
            updates.update("verified", force=True)
            return
        updates.update("still waiting for matching capture evidence")
        time.sleep(0.02)
    latest = getattr(capture, "latest_frame", None)
    if hasattr(capture, "latest_frame") and latest is None:
        raise TestFailure("capture_no_frames", f"timeout: {description}; no video frames received")
    if latest and frame_health(latest[1], capture.width, capture.height)["black"]:
        raise TestFailure("capture_black",
                          f"timeout: {description}; latest HDMI frame is black")
    if latest and time.monotonic() - latest[0] > 5:
        raise TestFailure("capture_stalled", f"timeout: {description}; video frames stopped")
    if hasattr(capture, "latest_audio") and capture.latest_audio is None:
        raise TestFailure("capture_no_audio", f"timeout: {description}; no audio blocks received")
    raise TestFailure("assertion_timeout", f"timeout: {description}")


def failure_evidence(capture: Capture, output: Path, report: dict) -> None:
    latest = capture.latest_frame
    if latest:
        failed = report["status"] != "PASS" or report.get("cleanup_errors")
        save_frame(output / ("failure.ppm" if failed else "last-frame.ppm"),
                   latest[1], capture.width, capture.height)
        save_json(output / "capture-health.json", {
            "frame_age_seconds": time.monotonic() - latest[0],
            **frame_health(latest[1], capture.width, capture.height),
        })
    else:
        save_json(output / "capture-health.json", {"video": "no frames received"})


class Observations:
    def __init__(self, capture: Capture, run_id: int, output: Path):
        self.capture, self.run_id, self.output = capture, run_id, output
        self.markers: list[tuple[float, Marker]] = []
        self.metrics: list[tuple[float, dict[str, float]]] = []
        self.events = (output / "observations.jsonl").open("w", encoding="utf-8")

    def poll(self) -> None:
        for kind, source in (("video", self.capture.frames), ("audio", self.capture.audio)):
            while True:
                try:
                    timestamp, data = source.get_nowait()
                except queue.Empty:
                    break
                if kind == "video":
                    marker = decode_frame(data, self.run_id)
                    if marker is None:
                        continue
                    self.markers.append((timestamp, marker))
                    details = {"counter": marker.counter, "state": marker.state,
                               "bounds": marker.bounds}
                    name = "response.ppm" if marker.state else "ready.ppm"
                    if not (self.output / name).exists():
                        (self.output / name).write_bytes(
                            f"P6\n{WIDTH} {HEIGHT}\n255\n".encode() + data
                        )
                else:
                    metrics = audio_metrics(data)
                    self.metrics.append((timestamp, metrics))
                    details = metrics
                self.events.write(json.dumps({
                    "monotonic": timestamp, "kind": kind, **details,
                }) + "\n")
                self.events.flush()


def switch_to_c64_profile(api: Api, manifest: Path, timeout: float, output: Path,
                          report: dict, health: Callable[[], None]) -> None:
    """Start the suite's C64 test profile unless it is already running."""
    from profile_suite import read_manifest, resolve_profiles, switch_profile
    report["stage"] = "profile_switch"
    case = next((item for item in read_manifest(manifest) if item["machine"] == "C64"), None)
    if case is None:
        raise RuntimeError(f"{manifest} has no C64 profile for the diagnostic")
    profile = resolve_profiles(api, [case])[0]
    report["profile"] = profile["id"]
    current = api.get_json("/api/status")
    if current.get("profile_id") == profile["id"] and current.get("machine") == "C64":
        progress(f"C64 test profile {profile['id']} is already running")
    else:
        progress(f"Switching from {current.get('profile_id')} / {current.get('machine')} "
                 f"to the C64 test profile {profile['id']}")
        switch_profile(api, profile, timeout, health, output)
    report["stage"] = "fixture"


def execute(args: argparse.Namespace, output: Path, report: dict,
            stack: ExitStack) -> None:
    def register(label: str, function: Callable[[], object]) -> None:
        def cleanup() -> None:
            progress(f"Cleanup: {label}")
            try:
                function()
            except (OSError, termios.error, RuntimeError, subprocess.SubprocessError) as error:
                message = f"{label}: {error}"
                report["cleanup_errors"].append(message)
                print(message, file=sys.stderr)
        stack.callback(cleanup)

    report["stage"] = "guest_generation"
    progress("Generating the C64 diagnostic PRG")
    program = build_guest(int(report["run_id"], 16), output)
    report["guest_sha256"] = hashlib.sha256(program.read_bytes()).hexdigest()
    if not args.execute:
        report.update(status="PREPARED", stage="offline", detail="No hardware accessed")
        return
    report["stage"] = "fixture"
    progress("Checking C64 HTTP status, storage and Pico USB fixture")
    api = Api(args.url, getpass.getpass("BMC64 PIN: ") if args.ask_pin else None)
    listing_result = api.get_json("/api/fs/list", path="/")
    save_json(output / "volume.json", listing_result)
    volume = listing_result.get("vol")
    if not isinstance(volume, str) or not volume:
        raise RuntimeError("root directory listing did not return a volume")
    pico_port = SerialPort(args.pico_uart)
    register("close Pico UART", pico_port.close)
    pico_log = stack.enter_context((output / "pico.log").open("wb"))
    pico = Pico(pico_port, pico_log)
    # A newline discards any incomplete/noisy line left by manual bring-up.
    pico_port.write(b"\n")
    register("release inputs", lambda: pico.command("RELEASE"))
    reply = pico.command("STATUS")
    fields = dict(part.split("=", 1) for part in reply.split()[1:] if "=" in part)
    if any(fields.get(key) != value for key, value in {
            "protocol": "1", "mounted": "1", "suspended": "0", "pending": "0"}.items()):
        raise RuntimeError(f"Pico is not ready: {reply}")
    pico.command("RELEASE")
    uart = None
    if args.bmc_uart:
        port = SerialPort(args.bmc_uart)
        register("close BMC64 UART", port.close)
        uart = UartLog(port, output / "bmc64-uart.log")
        register("save UART classification", lambda: save_json(
            output / "uart-findings.json", uart.findings))
        register("check final UART messages", uart.check)
        register("stop BMC64 UART logger", uart.close)
    report["uart_collected"] = uart is not None
    report["stage"] = "capture"
    # Record before any profile restart, as the suite does: a Pi that reboots
    # while the capture device is closed has come up with no HDMI picture.
    progress("Starting HDMI video/audio recording")
    capture = Capture(capture_command(args.video, args.audio, output), output)
    register("save capture evidence", lambda: failure_evidence(capture, output, report))
    register("finish capture", capture.close)

    def health() -> None:
        capture.check()
        if uart:
            uart.check()

    # UART logging is running, so a profile restart's boot messages are kept.
    switch_to_c64_profile(api, args.manifest, args.timeout, output, report, health)
    status = api.get_json("/api/status")
    save_json(output / "status-before.json", status)
    if str(status.get("machine", "")).upper() != "C64":
        raise RuntimeError("this diagnostic supports C64 only; target reports another machine")
    report["stage"] = "capture"
    wait_for(capture, uart, args.timeout, lambda: (
        not capture.frames.empty() and not capture.audio.empty()
        and (capture.latest_frame is None or not frame_health(capture.latest_frame[1])["black"])
    ), "HDMI video and audio capture startup")
    observed = Observations(capture, int(report["run_id"], 16), output)
    stack.callback(observed.events.close)
    report["stage"] = "upload_autostart"
    progress("Uploading and autostarting the diagnostic PRG")
    remote_path = f"/bmc64-smoke-{report['run_id']}.prg"
    report["guest_path"] = remote_path
    api.request("/api/fs/upload", program.read_bytes(), vol=volume, path=remote_path)
    api.request("/api/fs/autostart", b"", vol=volume, path=remote_path)
    report["stage"] = "guest_ready"

    def ready() -> bool:
        observed.poll()
        counters = {marker.counter for _, marker in observed.markers if marker.state == 0}
        first_marker = next((timestamp for timestamp, marker in observed.markers
                             if marker.state == 0), float("inf"))
        tones = [metrics for timestamp, metrics in observed.metrics
                 if timestamp > first_marker + 1 and is_tone(metrics)]
        return len(counters) >= 2 and len(tones) >= 2

    wait_for(capture, uart, args.timeout, ready,
             "current run ID, advancing counter and two seconds of SID tone")
    report["checks"].extend(["current_run_id", "guest_progress", "sid_tone"])
    tone_rms = max(metrics["rms"] for _, metrics in observed.metrics if is_tone(metrics))
    report["stage"] = "input_round_trip"
    progress("Sending USB Space and checking the guest response")
    sent = time.monotonic()
    pico.command("KEY 0 44")
    time.sleep(0.1)
    pico.command("KEY 0")

    def responded() -> bool:
        observed.poll()
        markers = [marker for timestamp, marker in observed.markers
                   if timestamp > sent and marker.state == 1]
        return len(markers) >= 2

    wait_for(capture, uart, args.timeout, responded, "Space response in current guest")
    report["checks"].append("space_round_trip")
    # Start fresh after the visual transition; do not accept pre-input silence.
    quiet_after = time.monotonic() + 1
    report["stage"] = "audio_silence"

    def quiet() -> bool:
        observed.poll()
        recent = [metrics for timestamp, metrics in observed.metrics
                  if timestamp > quiet_after]
        return len(recent) >= 2 and all(
            is_silence(metrics, tone_rms) for metrics in recent[-2:]
        )

    wait_for(capture, uart, args.timeout, quiet, "tone stops after guest receives Space")
    report["checks"].append("sid_silence")
    report["stage"] = "final_status"
    progress("Checking final HTTP status")
    save_json(output / "status-after.json", api.get_json("/api/status"))
    report.update(status="PASS", stage="complete")


def parser() -> argparse.ArgumentParser:
    result = argparse.ArgumentParser(description=__doc__)
    result.add_argument("--config", type=Path,
                        help="saved setup JSON; explicit command-line options override it")
    result.add_argument("--execute", action="store_true",
                        help="allow PRG upload/autostart, capture and Pico input")
    result.add_argument("--list", action="store_true", help="list the test names and exit")
    result.add_argument("tests", nargs="*", metavar="TEST",
                        help="test names to run (see --list); default: every test")
    result.add_argument("--manifest", type=Path, default=HERE / "suite_profiles.json",
                        help="profile manifest for the boot/typing tests")
    result.add_argument("--url", help="BMC64 base URL, including port if needed")
    result.add_argument("--pico-uart", help="exclusive Pico serial adapter")
    result.add_argument("--bmc-uart", help="optional exclusive diagnostic UART")
    result.add_argument("--ask-pin", action="store_true",
                        help="prompt without storing the Web UI PIN")
    result.add_argument("--video", default="/dev/v4l/by-id/usb-MACROSILICON_2109-video-index0")
    result.add_argument("--audio", default="plughw:CARD=MS2109,DEV=0")
    result.add_argument("--setup", default="", help="board/profile/ROM/filter notes")
    result.add_argument("--timeout", type=float, default=60,
                        help="deadline in seconds for each guest assertion")
    result.add_argument("--results", type=Path, default=HERE / "results")
    return result


def load_setup(path: Path) -> dict[str, object]:
    settings = json.loads(path.read_text(encoding="utf-8"))
    string_fields = {"url", "pico_uart", "bmc_uart", "video", "audio", "setup"}
    allowed = string_fields | {"ask_pin", "timeout"}
    if not isinstance(settings, dict) or settings.keys() - allowed:
        raise ValueError("setup must be a JSON object containing only supported setup options")
    for name in string_fields & settings.keys():
        value = settings[name]
        if name == "bmc_uart" and value is None:
            continue
        if not isinstance(value, str) or any(char in value for char in "\r\n\0"):
            raise ValueError(f"setup option {name} must be a single-line string")
    for name in ("url", "pico_uart"):
        if not settings.get(name):
            raise ValueError(f"setup requires {name}")
    # Constructing the client validates the URL without making a request.
    Api(settings["url"])
    for name in ("pico_uart", "bmc_uart", "video"):
        if settings.get(name) and not Path(settings[name]).is_absolute():
            raise ValueError(f"setup option {name} must be an absolute device path")
    for name in ("video", "audio"):
        if name in settings and not settings[name]:
            raise ValueError(f"setup option {name} must not be empty")
    if "ask_pin" in settings and not isinstance(settings["ask_pin"], bool):
        raise ValueError("setup option ask_pin must be true or false")
    if "timeout" in settings:
        timeout = settings["timeout"]
        if (isinstance(timeout, bool) or not isinstance(timeout, (int, float))
                or not math.isfinite(timeout) or timeout <= 0):
            raise ValueError("setup option timeout must be a finite positive number")
    return settings


def parse_arguments(argv: list[str] | None = None) -> argparse.Namespace:
    argument_parser = parser()
    selector = argparse.ArgumentParser(add_help=False)
    selector.add_argument("--config", type=Path)
    selected, _ = selector.parse_known_args(argv)
    if selected.config:
        try:
            argument_parser.set_defaults(**load_setup(selected.config))
        except (OSError, ValueError) as error:
            argument_parser.error(f"cannot load setup {selected.config}: {error}")
    return argument_parser.parse_args(argv)


DIAGNOSTIC = "c64-default"


def run_diagnostic(args: argparse.Namespace) -> dict:
    """The C64 PRG video/audio/input diagnostic; returns the saved report."""
    run_id = secrets.token_hex(4)
    output = args.results / (
        datetime.now(timezone.utc).strftime("%Y%m%dT%H%M%SZ") + "-" + run_id
    )
    output.mkdir(parents=True, exist_ok=False)
    report = {
        "schema": 1, "run_id": run_id, "name": DIAGNOSTIC, "status": "FAIL",
        "stage": "starting", "setup": args.setup, "url": args.url, "video": args.video,
        "audio": args.audio, "pico_uart": args.pico_uart, "bmc_uart": args.bmc_uart,
        "checks": [], "cleanup_errors": [],
        "started_utc": datetime.now(timezone.utc).isoformat(),
    }
    print(f"Evidence: {output}", flush=True)

    def interrupted(signum, frame):
        raise KeyboardInterrupt

    previous = signal.signal(signal.SIGTERM, interrupted)
    try:
        with ExitStack() as stack:
            try:
                execute(args, output, report, stack)
            except (OSError, termios.error, ValueError, RuntimeError, URLError,
                    subprocess.SubprocessError) as error:
                report["error"] = str(error)
                report["category"] = getattr(error, "category", "fixture_or_api")
                print(f"FAIL [{report['stage']}]: {error}", file=sys.stderr)
            except KeyboardInterrupt:
                report["error"] = "interrupted by user or termination signal"
                print(report["error"], file=sys.stderr)
        if report["cleanup_errors"]:
            report["status"] = "FAIL"
    finally:
        signal.signal(signal.SIGTERM, previous)
        report["finished_utc"] = datetime.now(timezone.utc).isoformat()
        report["evidence"] = str(output / "result.json")
        save_json(output / "result.json", report)
    print(f"{report['status']}: {output / 'result.json'}")
    return report


def available_tests(manifest: list[dict[str, str]]) -> dict[str, str]:
    from profile_suite import case_name
    tests = {case_name(case["machine"]):
             f"{case['machine']}: start profile {case['profile']}, check the BASIC boot "
             "screen, type a token over USB" for case in manifest}
    tests[DIAGNOSTIC] = ("C64: upload and autostart a PRG, check its video counter, SID tone, "
                         "USB Space response and silence")
    return tests


def main() -> int:
    from profile_suite import case_name, main as suite_main, read_manifest
    argument_parser = parser()
    args = parse_arguments()
    try:
        manifest = read_manifest(args.manifest)
    except (OSError, ValueError) as error:
        argument_parser.error(f"cannot read manifest {args.manifest}: {error}")
    tests = available_tests(manifest)
    if args.list:
        width = max(len(name) for name in tests)
        for name, description in tests.items():
            print(f"{name:<{width}}  {description}")
        return 0
    unknown = [name for name in args.tests if name not in tests]
    if unknown:
        argument_parser.error(f"unknown test {', '.join(unknown)}; see --list")
    selected = args.tests or list(tests)
    if not math.isfinite(args.timeout) or args.timeout <= 0:
        argument_parser.error("--timeout must be finite and positive")
    if args.execute and (not args.url or not args.pico_uart):
        argument_parser.error("--execute requires --url and --pico-uart")
    if args.bmc_uart and args.pico_uart and (
            os.path.realpath(args.bmc_uart) == os.path.realpath(args.pico_uart)):
        argument_parser.error("Pico and BMC64 UART paths must be different")
    required = (["petcat"] if DIAGNOSTIC in selected else []) + (["ffmpeg"] if args.execute else [])
    for tool in required:
        if shutil.which(tool) is None:
            argument_parser.error(f"required tool is missing: {tool}; see docs/prerequisites.md")
    results, evidence = [], []
    cases = [case for case in manifest if case_name(case["machine"]) in selected]
    if cases:
        report = suite_main(args, cases)
        results.extend(report["cases"])
        evidence.append(report["evidence"])
    if DIAGNOSTIC in selected:
        report = run_diagnostic(args)
        results.append(report)
        evidence.append(report["evidence"])
    print_summary(results)
    for path in evidence:
        print(f"  Evidence: {path}", flush=True)
    return 0 if all(result["status"] in ("PASS", "PREPARED") for result in results) else 1


if __name__ == "__main__":
    sys.exit(main())
