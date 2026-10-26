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
from typing import BinaryIO, TextIO

from framework.reporting import clock


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


# USB HID usage IDs and modifier bits, for the readable log.
KEY_NAMES = {**{4 + index: chr(ord("A") + index) for index in range(26)},
             **{30 + index: str((index + 1) % 10) for index in range(10)},
             40: "Return", 41: "Esc", 42: "Backspace", 43: "Tab", 44: "Space"}
MODIFIER_NAMES = ("LCtrl", "LShift", "LAlt", "LGUI", "RCtrl", "RShift", "RAlt", "RGUI")


def describe(command: str) -> str:
    """KEY 0 25 -> KEY V, KEY 2 4 -> KEY LShift+A, KEY 0 -> KEY release."""
    parts = command.split()
    if parts[0] != "KEY" or len(parts) < 2 or not all(part.isdigit() for part in parts[1:]):
        return command
    modifiers, usages = int(parts[1]), [int(part) for part in parts[2:]]
    names = [name for bit, name in enumerate(MODIFIER_NAMES) if modifiers & (1 << bit)]
    names += [KEY_NAMES.get(usage, f"usage {usage}") for usage in usages]
    return "KEY " + ("+".join(names) if usages else "release" + (
        f" (holding {'+'.join(names)})" if names else ""))


class Pico:
    """Pico UART client.

    log gets the exact TX/RX transcript with request IDs; the optional
    readable log gets one line per command with its reply and round-trip time.
    """

    def __init__(self, port: SerialPort, log: BinaryIO, readable: TextIO | None = None):
        self.port, self.log, self.readable = port, log, readable
        self.request_id = secrets.randbelow(1_000_000_000) + 1
        self.buffer = b""

    def note(self, text: str, wall: float | None = None) -> None:
        if self.readable:
            self.readable.write(f"{clock(wall)}  {text}\n")
            self.readable.flush()

    def mark(self, text: str) -> None:
        """Write a runner note, such as the start of a test."""
        self.note(text)

    def unsolicited(self, line: str) -> str:
        # Keep the Pico's own message, not restart noise in front of it.
        start = line.find(" EVENT ") if " EVENT " in line else line.find(" ERROR ")
        text = line[start + 1:] if start >= 0 else line
        if text.startswith("EVENT ready"):
            return f"Pico restarted ({text})"
        return f"Pico: {text}"

    def command(self, command: str) -> str:
        self.request_id += 1
        prefix = f"{self.request_id} "
        sent, wall = time.monotonic(), time.time()
        notes: list[str] = []
        outcome: str | None = None

        def finish() -> None:
            elapsed = (time.monotonic() - sent) * 1000
            self.note(f"{describe(command)} -> {outcome or 'interrupted'} ({elapsed:.0f} ms)", wall)
            for text in notes:
                self.note(text)

        self.log.write(f"{sent:.6f} TX {prefix}{command}\n".encode())
        self.log.flush()
        try:
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
                        outcome = reply[3:] if reply.startswith("OK ") else reply
                        if reply != "OK" and not reply.startswith("OK "):
                            raise RuntimeError(f"Pico rejected {command}: {reply}")
                        return reply
                    if line.strip():
                        notes.append(self.unsolicited(line))
                data = self.port.read(max(0, deadline - time.monotonic()))
                if data:
                    self.buffer += data
                    if len(self.buffer) > 16384:
                        outcome = "oversized UART input"
                        raise RuntimeError("oversized Pico UART input")
            outcome = "no reply (timeout)"
            raise RuntimeError(f"Pico timeout for {command}; outcome unknown, not retried")
        except (OSError, RuntimeError) as error:
            outcome = outcome or f"failed: {error}"
            raise
        finally:
            finish()
