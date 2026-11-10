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

"""Run selected tests in one session: one preflight, one rig, one evidence folder."""

import argparse
import getpass
import json
import secrets
import signal
import sys
import time
from datetime import datetime, timezone

from framework.case import BmcTest
from framework.context import Cleanups, TestContext
from framework.errors import ERRORS, TestFailure
from framework.reporting import progress, save_json
from framework.rig import Rig, ensure_picture, open_rig, open_uart, start_capture
from hardware.api import Api
from hardware.uart import UartLog
from support.evidence import failure_evidence
from support.network import validate_network
from support.profiles import recover_api, resolve_profiles, status, transient


def execute_case(test: BmcTest, context: TestContext) -> None:
    """Run one test's hooks, then the teardown every test gets, and save its result."""
    report = context.report
    report.setdefault("started_monotonic", time.monotonic())
    rig = context.rig
    try:
        test.invoke(context)
        if report["cleanup_errors"]:
            report["status"] = "FAIL"
        elif report["status"] == "PASS":
            report["stage"] = "complete"
    except ERRORS as error:
        report.update(status="FAIL", error=str(error),
                      category=getattr(error, "category", "fixture_or_api"))
        print(f"FAIL {test.name} [{report.get('stage')}]: {error}",
              file=sys.stderr, flush=True)
    except KeyboardInterrupt:
        report.update(status="FAIL", error="interrupted", category="interrupted")
        raise
    finally:
        report["finished_monotonic"] = time.monotonic()
        if rig:
            progress(f"{test.name}: releasing USB input and saving test evidence")
            context.cleanup("release inputs", lambda: rig.pico.command("RELEASE"))
            if rig.uart:
                context.cleanup("check test UART", lambda: rig.uart.check(context.uart_start))
                report["uart_findings"] = rig.uart.findings[context.uart_start:]
            context.cleanup("save test capture evidence",
                            lambda: failure_evidence(rig.capture, context.output, report))
        save_json(context.output / "result.json", report)


def locate_target(api: Api, uart: UartLog | None, timeout: float) -> None:
    """Reach BMC64, waiting for the UART to report its address if the saved one fails."""
    try:
        status(api)
    except OSError as error:
        if not transient(error):
            raise
        progress(f"No reply from {api.url}: reboot the Pi now; "
                 "its address will be read from the BMC64 UART")
        try:
            recover_api(api, timeout, "BMC64 address from the UART")
        except TestFailure as failure:
            received = uart.received if uart else 0
            hint = ("check the UART wiring and that enable_logging=uart is set" if not received
                    else "no IP address line in it; see bmc64-uart.log")
            raise TestFailure("uart_address", f"{failure}; BMC64 UART received "
                              f"{received} bytes: {hint}") from None


def save_address(args: argparse.Namespace, url: str) -> None:
    """Keep the address BMC64 reported, so the next run starts from it."""
    config = getattr(args, "config", None)
    if not config or url == args.url.rstrip("/"):
        return
    settings = json.loads(config.read_text(encoding="utf-8"))
    settings["url"] = url + "/"
    config.write_text(json.dumps(settings, indent=2) + "\n", encoding="utf-8")
    progress(f"Saved {url} to {config.name}")


def preflight(api: Api, tests: list[type[BmcTest]], manifest: list[dict[str, str]],
              args: argparse.Namespace, output, report: dict) -> dict[str, dict[str, str]]:
    """Resolve and check every selected test's profile before touching the fixture."""
    report["stage"] = "profile_preflight"
    progress("Preflight: checking HTTP status and resolving selected SD-card profiles")
    save_json(output / "status-before.json", status(api))
    machines = {test.machine for test in tests}
    cases = resolve_profiles(api, [case for case in manifest if case["machine"] in machines])
    validate_network(api, cases, args.network)
    save_json(output / "resolved-profiles.json", cases)
    progress(f"Preflight complete: {len(cases)} profile(s): " +
             ", ".join(case["machine"] for case in cases))
    return {case["machine"]: case for case in cases}


def run_tests(args: argparse.Namespace, rig: Rig, tests: list[type[BmcTest]],
              results: list[dict], profiles: dict[str, dict[str, str]], output,
              report: dict) -> None:
    blocked = None
    for index, (test, result) in enumerate(zip(tests, results), 1):
        label = f"[{index}/{len(tests)}] {test.name}"
        if blocked:
            result["error"] = blocked
            progress(f"{label}: NOT_RUN; {blocked}")
            continue
        profile = profiles[test.machine]
        progress(f"{label}: starting ({test.machine}, profile {profile['id']})")
        marker = f"===== {label} ({test.machine}) ====="
        rig.pico.mark(marker)
        if rig.uart:
            rig.uart.mark(marker)
        directory = output / f"{index:02d}-{test.name}"
        directory.mkdir()
        result.update(id=profile["id"], status="FAIL", evidence=directory.name)
        uart_start = len(rig.uart.findings) if rig.uart else 0

        def health() -> None:
            rig.capture.check()
            if rig.uart:
                rig.uart.check(uart_start)

        execute_case(test(), TestContext(args, directory, result, rig, profile,
                                         health, uart_start))
        print(f"{result['status']}: {test.name}", flush=True)
        if result["status"] != "PASS":
            progress(f"{test.name}: checking HTTP control before continuing")
            try:
                recover_api(rig.api, args.timeout)
            except ERRORS as error:
                blocked = f"HTTP control unavailable after {test.name}: {error}"
                report["control_error"] = blocked
                print(blocked, file=sys.stderr, flush=True)


def prepare_tests(args: argparse.Namespace, tests: list[type[BmcTest]],
                  results: list[dict], manifest: list[dict[str, str]], output) -> None:
    """Offline: run each test's prepare_test() only; nothing touches the rig."""
    cases = {case["machine"]: case for case in manifest}
    for index, (test, result) in enumerate(zip(tests, results), 1):
        directory = output / f"{index:02d}-{test.name}"
        directory.mkdir()
        result.update(evidence=directory.name)
        execute_case(test(), TestContext(args, directory, result,
                                         profile=cases.get(test.machine, {})))


def run(args: argparse.Namespace, tests: list[type[BmcTest]],
        manifest: list[dict[str, str]]) -> dict:
    """Run tests in order; returns the saved run report with one result per test."""
    started = time.monotonic()
    run_id = secrets.token_hex(4)
    output = args.results / (datetime.now(timezone.utc).strftime("%Y%m%dT%H%M%SZ")
                             + "-" + run_id)
    output.mkdir(parents=True, exist_ok=False)
    cases = {case["machine"]: case for case in manifest}
    results = [{**cases.get(test.machine, {}), "name": test.name, "suite": test.suite,
                "machine": test.machine, "run_id": run_id, "status": "NOT_RUN",
                "checks": [], "cleanup_errors": []} for test in tests]
    report = {
        "schema": 2, "run_id": run_id, "status": "FAIL", "stage": "starting",
        "tests": results, "cleanup_errors": [],
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
            report["stage"] = "offline"
            prepare_tests(args, tests, results, manifest, output)
        else:
            cleanups = Cleanups(report)
            capture = api = None
            try:
                pin = getpass.getpass("BMC64 PIN: ") if args.ask_pin else None
                api = Api(args.url, pin, timeout=3)
                report["stage"] = "target_address"
                uart = open_uart(args, api, output, report, cleanups.register)
                capture = start_capture(args, output, report, cleanups.register)
                report["stage"] = "target_address"
                locate_target(api, uart, args.timeout)
                profiles = preflight(api, tests, manifest, args, output, report)
                ensure_picture(capture, api, uart, args.timeout, report)
                rig = open_rig(args, api, uart, capture, output, report, cleanups.register)
                report["stage"] = "tests"
                run_tests(args, rig, tests, results, profiles, output, report)
                if not report.get("control_error"):
                    progress("All selected tests finished; checking final HTTP status")
                    save_json(output / "status-after.json", recover_api(api, args.timeout))
                    report["stage"] = "complete"
            except ERRORS as error:
                report.update(error=str(error),
                              category=getattr(error, "category", "fixture_or_api"))
                print(f"FAIL [{report['stage']}]: {error}", file=sys.stderr, flush=True)
                if capture:
                    cleanups.run("save capture evidence",
                                 lambda: failure_evidence(capture, output, report))
            except KeyboardInterrupt:
                report.update(error="interrupted", category="interrupted")
                print("interrupted by user or termination signal", file=sys.stderr, flush=True)
            finally:
                cleanups.close()
                if api:
                    report["url"] = api.url
                    cleanups.run("save BMC64 address", lambda: save_address(args, api.url))
    finally:
        signal.signal(signal.SIGTERM, previous)
        for result in results:
            if result["status"] == "NOT_RUN" and not result.get("error"):
                result["error"] = report.get("error", "run stopped before this test")
        statuses = {result["status"] for result in results}
        if report["cleanup_errors"] or report.get("error") or report.get("control_error"):
            report["status"] = "FAIL"
        elif statuses <= {"PREPARED"}:
            report["status"] = "PREPARED"
        elif statuses == {"PASS"}:
            report["status"] = "PASS"
        report["finished_utc"] = datetime.now(timezone.utc).isoformat()
        report["duration_seconds"] = round(time.monotonic() - started, 1)
        save_json(output / "result.json", report)
    report["evidence"] = str(output / "result.json")
    print(f"{report['status']}: {output / 'result.json'}", flush=True)
    return report
