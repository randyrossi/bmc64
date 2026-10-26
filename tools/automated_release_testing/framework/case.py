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

"""The lifecycle contract implemented by every discoverable hardware test."""

from abc import ABC, abstractmethod
import argparse

from framework.context import TestContext


class BmcTest(ABC):
    name: str
    description: str
    machine: str
    suite: str
    order = 100
    requirements: tuple[str, ...] = ()
    profile_required = False
    context: TestContext

    def before_test(self) -> None:
        pass

    def prepare_test(self) -> None:
        """Optional offline preparation; must never access the rig."""

    @abstractmethod
    def run_test(self) -> None:
        """Perform assertions; raise on failure rather than returning a status."""

    def after_test(self) -> None:
        pass

    def invoke(self, context: TestContext) -> None:
        self.context = context
        if not context.args.execute:
            try:
                self.prepare_test()
                context.report.update(status="PREPARED", stage="offline", detail="No hardware accessed")
            finally:
                context.close()
            return
        try:
            self.before_test()
            self.run_test()
            context.report.update(status="PASS", stage="complete")
        finally:
            try:
                context.cleanup("after_test", self.after_test)
            finally:
                context.close()

    @classmethod
    def run_suite(cls, args: argparse.Namespace, tests: list[type["BmcTest"]],
                  manifest: list[dict[str, str]]) -> tuple[list[dict], list[str], str | None]:
        from framework.session import run as run_standalone
        results, evidence = [], []
        blocked = None
        for test in tests:
            if blocked:
                results.append({"name": test.name, "status": "NOT_RUN", "error": blocked})
                continue
            report = run_standalone(args, test)
            results.append(report)
            evidence.append(report["evidence"])
            if report.get("category") == "interrupted":
                blocked = report["error"]
        return results, evidence, blocked
