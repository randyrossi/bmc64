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

import fcntl
import os
import secrets
import select
import termios
import time
from typing import BinaryIO


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
        # A Pico restart can leave UART noise in its unfinished input line.
        # Terminate that line before sending this command exactly once.
        self.port.write(f"\n{prefix}{command}\n".encode("ascii"))
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
