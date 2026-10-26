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

import argparse
import getpass
import secrets
import signal
import sys
import time
from contextlib import ExitStack
from datetime import datetime, timezone
from pathlib import Path
from typing import Callable
from framework.context import Rig, TestContext
from framework.case import BmcTest
from framework.errors import ERRORS
from framework.reporting import progress, save_json
from hardware.api import Api
from hardware.capture import SuiteCapture
from hardware.pico import Pico, SerialPort
from hardware.uart import UartLog
from support.constants import DEFAULT_CAPTURE_SIZE
from support.evidence import failure_evidence
from support.keyboard import wait_pico
from support.network import validate_network
from support.profiles import status, resolve_profiles, recover_api


def rotate(api: Api, pico: Pico, capture: SuiteCapture, uart: UartLog | None,
           cases: list[dict[str, str]], timeout: float, output: Path, report: dict,
           test_types: dict[str, type[BmcTest]] | None = None) -> None:
    from framework.discovery import discover
    available = test_types or {test.name: test for test in discover() if test.suite == "boot"}
    scheduled = [(case, test) for case in cases for test in available.values()
                 if test.machine == case["machine"]]
    blocked = None
    for index, (case, test_type) in enumerate(scheduled, 1):
        result = {**case, "name": test_type.name, "status": "NOT_RUN",
                  "checks": [], "cleanup_errors": []}
        report["cases"].append(result)
        if blocked:
            result["error"] = blocked
            progress(f"[{index}/{len(scheduled)}] {case['machine']}: NOT_RUN; {blocked}")
            continue
        progress(f"[{index}/{len(scheduled)}] {case['machine']}: starting profile {case['id']}")
        directory = output / f"{index:02d}-{case['machine'].lower()}"
        directory.mkdir()
        result.update(status="FAIL", evidence=directory.name,
                      started_monotonic=time.monotonic())
        uart_start = len(uart.findings) if uart else 0

        def health() -> None:
            capture.check()
            if uart:
                uart.check(uart_start)

        context = TestContext(
            args=argparse.Namespace(timeout=timeout, execute=True), output=directory, report=result,
            rig=Rig(api=api, pico=pico, capture=capture, uart=uart),
            profile=case, health=health, uart_start=uart_start)
        from framework.runner import execute_case
        execute_case(test_type(), context)
        print(f"{result['status']}: {case['machine']}", flush=True)
        if result["status"] != "PASS":
            progress(f"{case['machine']}: checking HTTP control before continuing")
            try:
                recover_api(api, timeout)
            except ERRORS as error:
                blocked = f"HTTP control unavailable after {case['machine']}: {error}"
                report["control_error"] = blocked
                print(blocked, file=sys.stderr)


def execute(args: argparse.Namespace, output: Path, report: dict, stack: ExitStack,
            test_types: dict[str, type[BmcTest]] | None = None) -> None:
    def register(label: str, action: Callable[[], object]) -> None:
        def cleanup() -> None:
            progress(f"Cleanup: {label}")
            try:
                action()
            except ERRORS as error:
                report["cleanup_errors"].append(f"{label}: {error}")
                print(f"{label}: {error}", file=sys.stderr)
        stack.callback(cleanup)

    api = Api(args.url, getpass.getpass("BMC64 PIN: ") if args.ask_pin else None, timeout=3)
    report["stage"] = "profile_preflight"
    progress("Preflight: checking HTTP status and resolving selected SD-card profiles")
    save_json(output / "status-before.json", status(api))
    cases = resolve_profiles(api, report["manifest"])
    validate_network(api, cases, args.network)
    save_json(output / "resolved-profiles.json", cases)
    progress(f"Preflight complete: {len(cases)} case(s): " +
             ", ".join(case["machine"] for case in cases))
    report["stage"] = "fixture"
    progress("Opening Pico UART and checking USB fixture")
    port = SerialPort(args.pico_uart)
    register("close Pico UART", port.close)
    log = stack.enter_context((output / "pico.log").open("wb"))
    pico = Pico(port, log)
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
    progress("Starting HDMI video/audio recording; each case reopens it after its core boots")
    capture = SuiteCapture(args.video, args.audio)

    def finish_capture() -> None:
        capture.close()
        if capture.errors:
            raise RuntimeError("; ".join(capture.errors))

    register("save capture evidence", lambda: failure_evidence(capture, output, report))
    register("finish capture", finish_capture)

    def startup_health() -> None:
        capture.check()
        if uart:
            uart.check()

    capture.open(output, DEFAULT_CAPTURE_SIZE, startup_health, min(15, args.timeout), "Startup")
    report["stage"] = "profile_cases"
    rotate(api, pico, capture, uart, cases, args.timeout, output, report, test_types)
    if report.get("control_error"):
        return
    progress("All selected cases finished; checking final HTTP status")
    save_json(output / "status-after.json", recover_api(api, args.timeout))
    expected_count = len(test_types) if test_types is not None else len(cases)
    if len(report["cases"]) == expected_count and all(case["status"] == "PASS" for case in report["cases"]):
        report.update(status="PASS", stage="complete")


def case_name(machine: str) -> str:
    return f"{machine.lower()}-boot-typing"


def main(args: argparse.Namespace, manifest: list[dict[str, str]],
         test_types: dict[str, type[BmcTest]] | None = None) -> dict:
    """Run the given manifest cases; returns the saved report."""
    run_id = secrets.token_hex(4)
    output = args.results / (datetime.now(timezone.utc).strftime("%Y%m%dT%H%M%SZ")
                             + "-suite-" + run_id)
    output.mkdir(parents=True, exist_ok=False)
    report = {
        "schema": 1, "run_id": run_id, "kind": "profile_suite", "status": "FAIL",
        "stage": "starting", "manifest": manifest, "cases": [], "cleanup_errors": [],
        "setup": args.setup, "url": args.url, "video": args.video, "audio": args.audio,
        "pico_uart": args.pico_uart, "bmc_uart": args.bmc_uart,
        "started_utc": datetime.now(timezone.utc).isoformat(),
    }
    print(f"Evidence: {output}", flush=True)

    def interrupted(signum, frame):
        raise KeyboardInterrupt

    previous = signal.signal(signal.SIGTERM, interrupted)
    try:
        if not args.execute:
            report.update(status="PREPARED", stage="offline", detail="Manifest valid; no hardware accessed")
        else:
            with ExitStack() as stack:
                try:
                    execute(args, output, report, stack, test_types)
                except ERRORS as error:
                    report.update(error=str(error), category=getattr(error, "category", "fixture_or_api"))
                    print(f"FAIL [{report['stage']}]: {error}", file=sys.stderr)
                except KeyboardInterrupt:
                    report.update(error="interrupted", category="interrupted")
            if report["cleanup_errors"]:
                report["status"] = "FAIL"
    finally:
        signal.signal(signal.SIGTERM, previous)
        expected = ([{**case, "name": test.name} for case in manifest
                     for test in test_types.values() if test.machine == case["machine"]]
                    if test_types is not None else
                    [{**case, "name": case_name(case["machine"])} for case in manifest])
        completed = {case["name"] for case in report["cases"]}
        report["cases"].extend({
            **case,
            **({"status": "PREPARED", "detail": "offline preparation; hardware not accessed"}
               if report["status"] == "PREPARED" else
               {"status": "NOT_RUN", "error": report.get("error", "suite stopped before this case")}),
        } for case in expected if case["name"] not in completed)
        report["finished_utc"] = datetime.now(timezone.utc).isoformat()
        save_json(output / "result.json", report)
    report["evidence"] = str(output / "result.json")
    print(f"{report['status']}: {output / 'result.json'}")
    return report
