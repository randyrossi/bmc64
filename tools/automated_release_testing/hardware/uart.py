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
from hardware.pico import SerialPort
from support.uart_findings import classify_uart


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
