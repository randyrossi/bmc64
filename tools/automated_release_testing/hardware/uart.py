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

import threading
import time
from pathlib import Path
from framework.errors import TestFailure
from framework.reporting import clock
from hardware.pico import SerialPort
from support.uart_findings import classify_uart


MAX_LINE = 16384


class UartLog:
    """Logs BMC64 UART output: exact bytes to .raw, readable timestamped lines to output.

    BMC64 ends lines with LF CR, so any run of CR/LF is one line break in the
    readable log. Each line is stamped with the PC wall-clock time its first
    byte arrived; findings also keep the monotonic time used by test results.
    """

    def __init__(self, port: SerialPort, output: Path):
        self.port = port
        self.stop = threading.Event()
        self.lock = threading.Lock()
        self.errors: list[str] = []
        self.findings: list[dict[str, object]] = []
        self.raw = output.with_suffix(".raw").open("wb")
        self.file = output.open("w", encoding="utf-8")
        self.line = bytearray()
        self.started: tuple[float, float] | None = None
        self.thread = threading.Thread(target=self.collect, daemon=True)
        self.thread.start()

    def mark(self, text: str) -> None:
        """Write a runner note, such as the start of a test, between UART lines."""
        with self.lock:
            self.file.write(f"{clock(time.time())}  {text}\n")
            self.file.flush()

    def end_line(self) -> None:
        if self.started is None:
            return
        wall, monotonic = self.started
        text = self.line.decode("utf-8", errors="replace").rstrip()
        self.line.clear()
        self.started = None
        with self.lock:
            self.file.write(f"{clock(wall)}  {text}\n")
        finding = classify_uart(text.strip())
        if finding:
            self.findings.append({"monotonic": monotonic, "time": clock(wall), **finding})

    def feed(self, data: bytes) -> None:
        for value in data:
            if value in b"\r\n":
                self.end_line()
                continue
            if self.started is None:
                self.started = (time.time(), time.monotonic())
            self.line.append(value)
            if len(self.line) > MAX_LINE:
                raise RuntimeError(f"UART line exceeded {MAX_LINE} bytes")

    def collect(self) -> None:
        try:
            while not self.stop.is_set():
                data = self.port.read(0.2)
                if data:
                    self.raw.write(data)
                    self.raw.flush()
                    self.feed(data)
                    with self.lock:
                        self.file.flush()
        except (OSError, RuntimeError) as error:
            self.errors.append(str(error))
        finally:
            self.end_line()

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
        self.raw.close()
        if self.errors:
            raise RuntimeError("BMC64 UART collector: " + "; ".join(self.errors))
        if self.thread.is_alive():
            raise RuntimeError("BMC64 UART collector did not stop")
