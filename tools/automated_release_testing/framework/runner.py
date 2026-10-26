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

"""Common case execution and sequential suite dispatch."""

import argparse
import sys
import time

from framework.case import BmcTest
from framework.context import TestContext
from framework.errors import ERRORS
from framework.reporting import progress, save_json
from support.evidence import failure_evidence


def execute_case(test: BmcTest, context: TestContext) -> None:
    report = context.report
    report.setdefault("started_monotonic", time.monotonic())
    try:
        test.invoke(context)
        if report["cleanup_errors"]:
            report["status"] = "FAIL"
        elif report["status"] != "PREPARED":
            report.update(status="PASS", stage="complete")
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
        if context.rig:
            progress(f"{test.machine}: releasing USB input and saving case evidence")
            if context.rig.uart:
                report["uart_findings"] = context.rig.uart.findings[context.uart_start:]
            context.cleanup("save case capture evidence",
                            lambda: failure_evidence(context.rig.capture, context.output, report))
        save_json(context.output / "result.json", report)


def run_selected(args: argparse.Namespace, tests: list[type[BmcTest]],
                 manifest: list[dict[str, str]]) -> tuple[list[dict], list[str]]:
    results, evidence = [], []
    suites: dict[str, list[type[BmcTest]]] = {}
    for test in tests:
        suites.setdefault(test.suite, []).append(test)
    blocked = None
    for suite_tests in suites.values():
        if blocked:
            results.extend({"name": test.name, "status": "NOT_RUN", "error": blocked}
                           for test in suite_tests)
            continue
        suite_results, suite_evidence, blocked = suite_tests[0].run_suite(args, suite_tests, manifest)
        results.extend(suite_results)
        evidence.extend(suite_evidence)
    return results, evidence
