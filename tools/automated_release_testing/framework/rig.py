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

from framework.errors import TestFailure
from framework.reporting import progress, save_json
from hardware.api import Api
from hardware.capture import SuiteCapture
from hardware.pico import Pico, SerialPort
from hardware.uart import UartLog
from support.constants import DEFAULT_CAPTURE_SIZE
from support.keyboard import wait_pico
from support.profiles import recover_api, transient


@dataclass
class Rig:
    api: Api
    pico: Pico
    capture: SuiteCapture
    uart: UartLog | None = None


def open_uart(args: argparse.Namespace, api: Api, output: Path, report: dict,
              register: Callable[[str, Callable[[], object]], None]) -> UartLog | None:
    """Start the BMC64 UART log first: it is what tells the run the Pi's address."""
    report["uart_collected"] = bool(args.bmc_uart)
    if not args.bmc_uart:
        return None

    def follow_address(address: str) -> None:
        previous = api.url
        api.use_address(address)
        if api.url != previous:
            progress(f"BMC64 UART reports a new address: now using {api.url}")

    progress("Starting BMC64 UART logging")
    port = SerialPort(args.bmc_uart)
    register("close BMC64 UART", port.close)
    uart = UartLog(port, output / "bmc64-uart.log", follow_address)
    register("save UART classification", lambda: save_json(
        output / "uart-findings.json", uart.findings))
    register("check final UART messages", uart.check)
    register("stop UART logger", uart.close)
    return uart


def start_capture(args: argparse.Namespace, output: Path, report: dict,
                  register: Callable[[str, Callable[[], object]], None]) -> SuiteCapture:
    """Start recording before anything can reboot the Pi.

    A Pi that reboots while the capture device is closed comes up with no HDMI
    picture, so this comes before the wait for a manual reboot. Each test
    reopens the capture after its core boots.
    """
    report["stage"] = "capture_startup"
    progress("Starting HDMI video/audio recording; each test reopens it after its core boots")
    capture = SuiteCapture(args.video, args.audio)

    def finish_capture() -> None:
        capture.close()
        if capture.errors:
            raise RuntimeError("; ".join(capture.errors))

    register("finish capture", finish_capture)
    capture.start(output, DEFAULT_CAPTURE_SIZE, "Startup")
    return capture


def ensure_picture(capture: SuiteCapture, api: Api, uart: UartLog | None, timeout: float,
                   report: dict) -> None:
    """Wait for HDMI; if the Pi booted while the capture was closed, restart it."""

    def health() -> None:
        capture.check()
        if uart:
            uart.check()

    report["stage"] = "capture_startup"
    try:
        capture.wait_ready(health, min(15, timeout), "Startup")
        return
    except TestFailure as failure:
        if failure.category != "capture_startup":
            raise
    progress("No HDMI picture: restarting BMC64 now that the capture is open")
    report["startup_reboot"] = True
    try:
        api.request("/api/reboot", b"")
    except OSError as error:
        # A restart can drop the reply; the picture below shows if it worked.
        if not transient(error):
            raise
    capture.wait_ready(health, timeout, "Startup after restart")
    recover_api(api, timeout)


def open_rig(args: argparse.Namespace, api: Api, uart: UartLog | None, capture: SuiteCapture,
             output: Path, report: dict,
             register: Callable[[str, Callable[[], object]], None]) -> Rig:
    """Open the Pico, the last part of the rig; register its cleanup."""
    report["stage"] = "fixture"
    progress("Opening Pico UART and checking USB fixture")
    port = SerialPort(args.pico_uart)
    register("close Pico UART", port.close)
    raw = (output / "pico.raw").open("wb")
    register("close Pico transcript", raw.close)
    log = (output / "pico.log").open("w", encoding="utf-8")
    register("close Pico log", log.close)
    pico = Pico(port, raw, log)
    # A newline discards any incomplete/noisy line left by manual bring-up.
    port.write(b"\n")
    register("release inputs", lambda: pico.command("RELEASE"))
    wait_pico(pico, lambda: None, args.timeout)
    pico.command("RELEASE")
    return Rig(api=api, pico=pico, capture=capture, uart=uart)
