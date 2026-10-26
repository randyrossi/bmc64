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

"""Run a standalone test with the diagnostic's established evidence format."""

import argparse
import getpass
import secrets
import signal
from datetime import datetime, timezone

from framework.case import BmcTest
from framework.context import TestContext
from framework.reporting import save_json
from framework.runner import execute_case


def run(args: argparse.Namespace, test_type: type[BmcTest]) -> dict:
    run_id = secrets.token_hex(4)
    output = args.results / (
        datetime.now(timezone.utc).strftime("%Y%m%dT%H%M%SZ") + "-" + run_id)
    output.mkdir(parents=True, exist_ok=False)
    report = {
        "schema": 1, "run_id": run_id, "name": test_type.name, "status": "FAIL",
        "stage": "starting", "setup": args.setup, "url": args.url, "video": args.video,
        "audio": args.audio, "pico_uart": args.pico_uart, "bmc_uart": args.bmc_uart,
        "checks": [], "cleanup_errors": [],
        "started_utc": datetime.now(timezone.utc).isoformat(),
    }
    print(f"Evidence: {output}", flush=True)
    context = TestContext(args, output, report)

    def interrupted(signum, frame):
        raise KeyboardInterrupt

    previous = signal.signal(signal.SIGTERM, interrupted)
    try:
        if args.execute and args.ask_pin:
            context.pin = getpass.getpass("BMC64 PIN: ")
        try:
            execute_case(test_type(), context)
        except KeyboardInterrupt:
            report.update(status="FAIL", error="interrupted", category="interrupted")
    finally:
        signal.signal(signal.SIGTERM, previous)
        report["finished_utc"] = datetime.now(timezone.utc).isoformat()
        report["evidence"] = str(output / "result.json")
        save_json(output / "result.json", report)
    print(f"{report['status']}: {output / 'result.json'}")
    return report
