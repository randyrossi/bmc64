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
import time
from pathlib import Path
from framework.case import BmcTest
from framework.reporting import progress, save_json
from hardware.capture import Capture, wait_for
from support.constants import DEFAULT_CAPTURE_SIZE
from .diagnostic import HEIGHT, WIDTH, Marker, audio_metrics, decode_frame, is_silence, is_tone
from .guest import build_guest


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


class Test(BmcTest):
    name = "c64-default"
    description = "C64: PRG video counter, SID tone, USB Space response and silence"
    machine = "C64"
    suite = "basic"
    order = 70

    def prepare_test(self) -> None:
        self.stage("guest_generation")
        progress("Assembling the C64 diagnostic PRG")
        self.program = build_guest(int(self.report["run_id"], 16), self.output)
        self.report["guest_sha256"] = hashlib.sha256(self.program.read_bytes()).hexdigest()

    def before_test(self) -> None:
        self.prepare_test()
        # Keep an already running C64 test profile; the PRG resets the machine anyway.
        self.start_profile(fresh=False)
        # The diagnostic decodes small frames: analyse 720p capture at 480x360.
        self.open_capture((WIDTH, HEIGHT), video_size=DEFAULT_CAPTURE_SIZE)
        self.observed = Observations(self.capture.current, int(self.report["run_id"], 16),
                                     self.output)
        self.context.register("close observations", self.observed.events.close)
        self.wait_for_keyboard()

    def run_test(self) -> None:
        capture, observed, pico = self.capture.current, self.observed, self.pico
        self.stage("upload_autostart")
        progress("Uploading and autostarting the diagnostic PRG")
        remote_path = f"/bmc64-smoke-{self.report['run_id']}.prg"
        self.report["guest_path"] = remote_path
        volume = self.profile["volume"]
        self.api.request("/api/fs/upload", self.program.read_bytes(), timeout=15,
                         vol=volume, path=remote_path)
        self.api.request("/api/fs/autostart", b"", timeout=15, vol=volume, path=remote_path)
        self.stage("guest_ready")

        def ready() -> bool:
            observed.poll()
            counters = {marker.counter for _, marker in observed.markers if marker.state == 0}
            first_marker = next((timestamp for timestamp, marker in observed.markers
                                 if marker.state == 0), float("inf"))
            tones = [metrics for timestamp, metrics in observed.metrics
                     if timestamp > first_marker + 1 and is_tone(metrics)]
            return len(counters) >= 2 and len(tones) >= 2

        wait_for(capture, self.health, self.timeout, ready,
                 "current run ID, advancing counter and two seconds of SID tone")
        for check in ("current_run_id", "guest_progress", "sid_tone"):
            self.passed(check)
        tone_rms = max(metrics["rms"] for _, metrics in observed.metrics if is_tone(metrics))
        self.stage("input_round_trip")
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

        wait_for(capture, self.health, self.timeout, responded, "Space response in current guest")
        self.passed("space_round_trip")
        # Start fresh after the visual transition; do not accept pre-input silence.
        quiet_after = time.monotonic() + 1
        self.stage("audio_silence")

        def quiet() -> bool:
            observed.poll()
            recent = [metrics for timestamp, metrics in observed.metrics
                      if timestamp > quiet_after]
            return len(recent) >= 2 and all(
                is_silence(metrics, tone_rms) for metrics in recent[-2:]
            )

        wait_for(capture, self.health, self.timeout, quiet, "tone stops after guest receives Space")
        self.passed("sid_silence")
        self.stage("final_status")
        progress("Checking final HTTP status")
        save_json(self.output / "status-after.json", self.api.get_json("/api/status"))

    def after_test(self) -> None:
        pass
