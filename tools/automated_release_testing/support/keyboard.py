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

import re
import time
from typing import Callable
from framework.errors import TestFailure
from framework.reporting import WaitProgress
from hardware.pico import Pico


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
