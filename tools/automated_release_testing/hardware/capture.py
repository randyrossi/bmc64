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

import os
import queue
import subprocess
import threading
import time
from pathlib import Path
from typing import BinaryIO, Callable
from framework.errors import ERRORS, TestFailure
from framework.reporting import WaitProgress, progress, save_json
from hardware.uart import UartLog
from support.constants import DEFAULT_CAPTURE_SIZE, HEIGHT, SAMPLE_RATE, WIDTH
from support.evidence import frame_health


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
