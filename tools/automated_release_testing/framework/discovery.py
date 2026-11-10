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

"""Discover test_*.py classes without connecting to hardware."""

import importlib
import inspect
import re
from pathlib import Path

from framework.case import BmcTest


TEST_ROOT = Path(__file__).resolve().parents[1] / "tests"


def discover() -> list[type[BmcTest]]:
    tests = []
    names = set()
    for path in sorted(TEST_ROOT.rglob("test_*.py")):
        relative = path.relative_to(TEST_ROOT)
        if len(relative.parts) < 2:
            raise ValueError(f"test must belong to a suite folder: {relative}")
        module_name = ".".join(("tests", *relative.with_suffix("").parts))
        module = importlib.import_module(module_name)
        test = getattr(module, "Test", None)
        if (not inspect.isclass(test) or not issubclass(test, BmcTest)
                or inspect.isabstract(test) or test.__module__ != module_name):
            raise ValueError(f"{relative} must define a concrete Test(BmcTest) class")
        suite = relative.parent.as_posix()
        if test.__dict__.get("suite", suite) != suite:
            raise ValueError(f"{relative}: suite must match its folder {suite}")
        test.suite = suite
        if (not isinstance(getattr(test, "name", None), str)
                or not re.fullmatch(r"[a-z0-9][a-z0-9-]*", test.name)
                or not isinstance(getattr(test, "description", None), str)
                or not test.description.strip()):
            raise ValueError(f"{relative}: test needs a name and description")
        if test.name in names:
            raise ValueError(f"duplicate test name: {test.name}")
        names.add(test.name)
        tests.append(test)
    suites = {test.suite for test in tests}
    if names & suites:
        raise ValueError("test names must not conflict with suite folders")
    return sorted(tests, key=lambda test: (test.order, test.suite, test.name))


def select_tests(tests: list[type[BmcTest]], selectors: list[str]) -> list[type[BmcTest]]:
    if not selectors:
        return tests
    selected = set()
    for selector in selectors:
        matches = [test for test in tests if test.name == selector or test.suite == selector
                   or test.suite.startswith(selector + "/")]
        if not matches:
            raise ValueError(f"unknown test {selector} or suite; see --list")
        selected.update(matches)
    return [test for test in tests if test in selected]
