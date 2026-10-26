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
import json
import queue
import subprocess
import time
from pathlib import Path
from typing import Callable
from framework.case import BmcTest
from framework.reporting import progress, save_json
from hardware.api import Api
from hardware.capture import Capture, capture_command, wait_for
from hardware.pico import Pico, SerialPort
from hardware.uart import UartLog
from support.evidence import failure_evidence, frame_health
from support.profiles import read_manifest, resolve_profiles, switch_profile
from support.network import validate_network
from .diagnostic import (
    HEIGHT, WIDTH, Marker, audio_metrics, decode_frame, is_silence, is_tone, payload,
)


def listing(run_id: int) -> str:
    values = ",".join(str(value) for value in payload(run_id, 0, 0))
    return f"""10 rem bmc64 smoke run {run_id:08x}
20 poke 53280,0:poke 53281,0:print chr$(147)
30 dim d(9):for i=0 to 9:read d(i):next
40 for i=0 to 999:poke 1024+i,160:poke 55296+i,0:next
50 for y=3 to 20:for x=4 to 35:poke 55296+y*40+x,2:next:next
60 poke 54296,8:poke 54272,132:poke 54273,66
70 poke 54277,0:poke 54278,240:poke 54276,33
80 c=0:s=0:l=0:h=9:gosub 500
90 get a$:if a$=" " then s=1:poke 54276,32
100 c=c+1:if c>255 then c=0
110 d(6)=c:d(7)=s:v=0:for i=0 to 7:v=v+d(i):next
120 d(8)=v-256*int(v/256):l=6:h=8:gosub 500:goto 90
500 for b=l to h:v=d(b)
510 for k=0 to 7:p=int(v/2^(7-k)) and 1
520 n=b*8+k:y=int(n/10):x=n-y*10
530 a=55296+(4+y*2)*40+5+x*3
540 for j=0 to 1:for i=0 to 2:poke a+j*40+i,p:next:next
550 next:next:return
600 data {values}
"""


def build_guest(run_id: int, output: Path) -> Path:
    source = output / "diagnostic.bas"
    program = output / "diagnostic.prg"
    source.write_text(listing(run_id), encoding="ascii")
    completed = subprocess.run(
        ["petcat", "-w2", "-o", str(program), "--", str(source)],
        capture_output=True, text=True, timeout=10, check=False,
    )
    (output / "petcat.log").write_text(
        completed.stdout + completed.stderr, encoding="utf-8"
    )
    if completed.returncode:
        raise RuntimeError(f"petcat failed; see {output / 'petcat.log'}")
    if not program.read_bytes().startswith(b"\x01\x08"):
        raise RuntimeError("petcat did not produce a C64 BASIC PRG at $0801")
    return program


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
                          report: dict, health: Callable[[], None],
                          network: str | None = None) -> None:
    """Start the suite's C64 test profile unless it is already running."""
    report["stage"] = "profile_switch"
    case = next((item for item in read_manifest(manifest) if item["machine"] == "C64"), None)
    if case is None:
        raise RuntimeError(f"{manifest} has no C64 profile for the diagnostic")
    profile = resolve_profiles(api, [case])[0]
    validate_network(api, [profile], network)
    report["profile"] = profile["id"]
    current = api.get_json("/api/status")
    if current.get("profile_id") == profile["id"] and current.get("machine") == "C64":
        progress(f"C64 test profile {profile['id']} is already running")
    else:
        progress(f"Switching from {current.get('profile_id')} / {current.get('machine')} "
                 f"to the C64 test profile {profile['id']}")
        switch_profile(api, profile, timeout, health, output)
    report["stage"] = "fixture"


class Test(BmcTest):
    name = "c64-default"
    description = "C64: PRG video counter, SID tone, USB Space response and silence"
    machine = "C64"
    suite = "basic"
    order = 70
    requirements = ("petcat",)

    def prepare_test(self) -> None:
        context = self.context
        output, report = context.output, context.report
        report["stage"] = "guest_generation"
        progress("Generating the C64 diagnostic PRG")
        program = build_guest(int(report["run_id"], 16), output)
        report["guest_sha256"] = hashlib.sha256(program.read_bytes()).hexdigest()
        self.program = program

    def before_test(self) -> None:
        self.prepare_test()
        context = self.context
        args, output, report = context.args, context.output, context.report
        report["stage"] = "fixture"
        progress("Checking C64 HTTP status, storage and Pico USB fixture")
        api = Api(args.url, context.pin)
        listing_result = api.get_json("/api/fs/list", path="/")
        save_json(output / "volume.json", listing_result)
        volume = listing_result.get("vol")
        if not isinstance(volume, str) or not volume:
            raise RuntimeError("root directory listing did not return a volume")
        pico_port = SerialPort(args.pico_uart)
        context.register("close Pico UART", pico_port.close)
        pico_log = context.stack.enter_context((output / "pico.log").open("wb"))
        pico = Pico(pico_port, pico_log)
        # A newline discards any incomplete/noisy line left by manual bring-up.
        pico_port.write(b"\n")
        context.register("release inputs", lambda: pico.command("RELEASE"))
        reply = pico.command("STATUS")
        fields = dict(part.split("=", 1) for part in reply.split()[1:] if "=" in part)
        if any(fields.get(key) != value for key, value in {
                "protocol": "1", "mounted": "1", "suspended": "0", "pending": "0"}.items()):
            raise RuntimeError(f"Pico is not ready: {reply}")
        pico.command("RELEASE")
        uart = None
        if args.bmc_uart:
            port = SerialPort(args.bmc_uart)
            context.register("close BMC64 UART", port.close)
            uart = UartLog(port, output / "bmc64-uart.log")
            context.register("save UART classification", lambda: save_json(
                output / "uart-findings.json", uart.findings))
            context.register("check final UART messages", uart.check)
            context.register("stop BMC64 UART logger", uart.close)
        report["uart_collected"] = uart is not None
        report["stage"] = "capture"
        # Record before any profile restart, as the suite does: a Pi that reboots
        # while the capture device is closed has come up with no HDMI picture.
        progress("Starting HDMI video/audio recording")
        capture = Capture(capture_command(args.video, args.audio, output), output)
        context.register("save capture evidence", lambda: failure_evidence(capture, output, report))
        context.register("finish capture", capture.close)

        def health() -> None:
            capture.check()
            if uart:
                uart.check()

        # UART logging is running, so a profile restart's boot messages are kept.
        switch_to_c64_profile(api, args.manifest, args.timeout, output, report, health, args.network)
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
        context.stack.callback(observed.events.close)

        self.api, self.volume = api, volume
        self.pico, self.capture, self.uart, self.observed = pico, capture, uart, observed

    def run_test(self) -> None:
        context = self.context
        args, output, report = context.args, context.output, context.report
        if not args.execute:
            return
        program, api, volume = self.program, self.api, self.volume
        pico, capture, uart, observed = self.pico, self.capture, self.uart, self.observed
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


    def after_test(self) -> None:
        pass
