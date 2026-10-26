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

"""The rig every test in a run shares: HTTP control, Pico, BMC64 UART and capture."""

import argparse
from dataclasses import dataclass
from pathlib import Path
from typing import Callable

from framework.reporting import progress, save_json
from hardware.api import Api
from hardware.capture import SuiteCapture
from hardware.pico import Pico, SerialPort
from hardware.uart import UartLog
from support.constants import DEFAULT_CAPTURE_SIZE
from support.keyboard import wait_pico


@dataclass
class Rig:
    api: Api
    pico: Pico
    capture: SuiteCapture
    uart: UartLog | None = None


def open_rig(args: argparse.Namespace, api: Api, output: Path, report: dict,
             register: Callable[[str, Callable[[], object]], None]) -> Rig:
    """Open the Pico, UART logger and capture once; register their cleanup."""
    report["stage"] = "fixture"
    progress("Opening Pico UART and checking USB fixture")
    port = SerialPort(args.pico_uart)
    register("close Pico UART", port.close)
    log = (output / "pico.log").open("wb")
    register("close Pico log", log.close)
    pico = Pico(port, log)
    # A newline discards any incomplete/noisy line left by manual bring-up.
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
    # Record before any profile restart: a Pi that reboots while the capture
    # device is closed has come up with no HDMI picture. Each test reopens it.
    progress("Starting HDMI video/audio recording; each test reopens it after its core boots")
    capture = SuiteCapture(args.video, args.audio)

    def finish_capture() -> None:
        capture.close()
        if capture.errors:
            raise RuntimeError("; ".join(capture.errors))

    register("finish capture", finish_capture)

    def startup_health() -> None:
        capture.check()
        if uart:
            uart.check()

    capture.open(output, DEFAULT_CAPTURE_SIZE, startup_health, min(15, args.timeout), "Startup")
    return Rig(api=api, pico=pico, capture=capture, uart=uart)
