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

"""Hardware-free discovery, hook lifecycle and sequential dispatch checks."""

import argparse
import io
import json
import tempfile
import unittest
from pathlib import Path
from types import ModuleType
from unittest.mock import Mock, patch

from framework.case import BmcTest
from framework.context import TestContext
from framework.discovery import discover, select_tests
from framework.errors import TestFailure
from framework.runner import execute_case, run_selected


class FrameworkTests(unittest.TestCase):
    def context(self, directory, execute=True):
        return TestContext(
            argparse.Namespace(execute=execute),
            Path(directory),
            {"status": "FAIL", "stage": "starting", "checks": [], "cleanup_errors": []})

    def lifecycle(self, failure=None):
        events = []

        class Case(BmcTest):
            name, suite, machine = "example", "example", "C64"

            def before_test(self):
                self.context.report["stage"] = "setup"
                self.context.register("first resource", lambda: events.append("close-first"))
                self.context.register("second resource", lambda: events.append("close-second"))
                events.append("before")
                if failure in ("setup", "both"):
                    raise TestFailure("setup_failed", "original setup failure")

            def run_test(self):
                self.context.report["stage"] = "assertion"
                events.append("run")
                if failure == "run":
                    raise TestFailure("assertion", "original assertion failure")
                if failure == "interrupt":
                    raise KeyboardInterrupt

            def after_test(self):
                events.append("after")
                if failure in ("after", "both"):
                    raise RuntimeError("after hook failure")

        with tempfile.TemporaryDirectory() as directory, \
                patch("sys.stdout", new_callable=io.StringIO), \
                patch("sys.stderr", new_callable=io.StringIO):
            context = self.context(directory)
            if failure == "interrupt":
                with self.assertRaises(KeyboardInterrupt):
                    execute_case(Case(), context)
            else:
                execute_case(Case(), context)
            saved = json.loads((Path(directory) / "result.json").read_text())
        return events, saved

    def test_success_runs_hooks_then_registered_cleanup_in_reverse_order(self):
        events, report = self.lifecycle()
        self.assertEqual(events, ["before", "run", "after", "close-second", "close-first"])
        self.assertEqual(report["status"], "PASS")

    def test_partial_setup_failure_still_runs_after_and_closes_resources(self):
        events, report = self.lifecycle("setup")
        self.assertEqual(events, ["before", "after", "close-second", "close-first"])
        self.assertEqual(report["status"], "FAIL")
        self.assertEqual(report["category"], "setup_failed")

    def test_run_failure_is_saved_and_cleanup_still_runs(self):
        events, report = self.lifecycle("run")
        self.assertEqual(events[-3:], ["after", "close-second", "close-first"])
        self.assertEqual(report["error"], "original assertion failure")
        self.assertEqual(report["stage"], "assertion")

    def test_after_failure_prevents_pass(self):
        events, report = self.lifecycle("after")
        self.assertEqual(events[-2:], ["close-second", "close-first"])
        self.assertEqual(report["status"], "FAIL")
        self.assertEqual(report["stage"], "after_test")
        self.assertEqual(report["cleanup_errors"], ["after_test: after hook failure"])

    def test_cleanup_error_does_not_replace_original_failure(self):
        _, report = self.lifecycle("both")
        self.assertEqual(report["error"], "original setup failure")
        self.assertEqual(report["category"], "setup_failed")
        self.assertEqual(report["cleanup_errors"], ["after_test: after hook failure"])

    def test_interruption_closes_resources_and_saves_result_before_propagating(self):
        events, report = self.lifecycle("interrupt")
        self.assertEqual(events[-3:], ["after", "close-second", "close-first"])
        self.assertEqual(report["category"], "interrupted")
        self.assertEqual(report["status"], "FAIL")

    def test_offline_mode_never_invokes_hardware_hooks(self):
        events = []

        class Case(BmcTest):
            def prepare_test(self):
                events.append("prepare")

            def before_test(self):
                raise AssertionError("hardware setup invoked")

            def run_test(self):
                raise AssertionError("hardware test invoked")

            def after_test(self):
                raise AssertionError("hardware teardown invoked")

        with tempfile.TemporaryDirectory() as directory:
            context = self.context(directory, execute=False)
            Case().invoke(context)
            self.assertEqual(context.report["status"], "PREPARED")
        self.assertEqual(events, ["prepare"])

    def test_cleanup_continues_after_registered_resource_failure(self):
        events = []
        with tempfile.TemporaryDirectory() as directory, \
                patch("sys.stdout", new_callable=io.StringIO), \
                patch("sys.stderr", new_callable=io.StringIO):
            context = self.context(directory)
            context.register("remaining", lambda: events.append("remaining"))

            def broken():
                raise RuntimeError("close failed")

            context.register("broken", broken)
            context.close()
        self.assertEqual(events, ["remaining"])
        self.assertEqual(context.report["cleanup_errors"], ["broken: close failed"])

    def test_discovery_finds_only_seven_executable_test_modules(self):
        tests = discover()
        self.assertEqual([test.name for test in tests], [
            "c64-boot-typing", "c128-boot-typing", "vic20-boot-typing",
            "plus4-boot-typing", "plus4emu-boot-typing", "pet-boot-typing", "c64-default"])
        self.assertTrue(all(test.__module__.split(".")[-1].startswith("test_") for test in tests))
        self.assertTrue(all("unit_tests" not in test.__module__ for test in tests))
        self.assertEqual([test.suite for test in tests], ["boot"] * 6 + ["basic"])

    def test_boot_files_own_their_layouts_and_explicit_test_flow(self):
        from tests.boot.base import BootTypingTest
        steps = ["prepare_screen_reader", "start_profile", "open_native_capture",
                 "assert_boot_screen", "wait_for_keyboard", "type_usb_token",
                 "assert_typed_token", "assert_profile_unchanged"]
        for test_type in discover():
            if not issubclass(test_type, BootTypingTest):
                continue
            with self.subTest(test=test_type.name):
                self.assertIn("layout", test_type.__dict__)
                self.assertIn("run_test", test_type.__dict__)
                test = test_type()
                calls = Mock(spec=steps)
                with patch.object(BootTypingTest, "before_test") as setup, \
                        patch.multiple(test, **{step: getattr(calls, step) for step in steps}):
                    test.before_test()
                    test.run_test()
                setup.assert_called_once_with()
                self.assertEqual([call[0] for call in calls.mock_calls], steps)
                calls.prepare_screen_reader.assert_called_once_with(
                    test_type.layout, test_type.capture_size, token_length=8)

    def test_suite_and_individual_selection_deduplicate_in_existing_order(self):
        tests = discover()
        self.assertEqual(select_tests(tests, ["boot"]), tests[:6])
        self.assertEqual(select_tests(tests, ["basic"]), tests[6:])
        self.assertEqual(select_tests(tests, ["basic", "boot", "c64-boot-typing"]), tests)
        self.assertEqual(select_tests(tests, ["pet-boot-typing"]), [tests[5]])
        with self.assertRaisesRegex(ValueError, "unknown test"):
            select_tests(tests, ["typo"])

    def test_malformed_and_duplicate_definitions_fail_before_execution(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            (root / "boot").mkdir()
            for filename in ("test_one.py", "test_two.py", "base.py"):
                (root / "boot" / filename).touch()

            def definition(name):
                module = ModuleType(name)
                module.Test = type("Test", (BmcTest,), {
                    "__module__": name, "name": "duplicate", "description": "example",
                    "run_test": lambda self: None})
                return module

            with patch("framework.discovery.TEST_ROOT", root), \
                    patch("framework.discovery.importlib.import_module", side_effect=definition):
                with self.assertRaisesRegex(ValueError, "duplicate test name"):
                    discover()
            with patch("framework.discovery.TEST_ROOT", root), \
                    patch("framework.discovery.importlib.import_module", return_value=ModuleType("empty")):
                with self.assertRaisesRegex(ValueError, "must define a concrete Test"):
                    discover()

    def test_shared_suite_session_dispatch_and_blocking_are_generic(self):
        calls = []

        class First(BmcTest):
            name, suite = "first", "first-suite"

            def run_test(self):
                pass

            @classmethod
            def run_suite(cls, args, tests, manifest):
                calls.append([test.name for test in tests])
                return [{"name": cls.name, "status": "FAIL"}], ["first-result"], "rig unavailable"

        class Second(BmcTest):
            name, suite = "second", "second-suite"

            def run_test(self):
                raise AssertionError("blocked test ran")

        results, evidence = run_selected(argparse.Namespace(), [First, Second], [])
        self.assertEqual(calls, [["first"]])
        self.assertEqual(evidence, ["first-result"])
        self.assertEqual(results[1], {
            "name": "second", "status": "NOT_RUN", "error": "rig unavailable"})

    def test_multiple_boot_files_for_one_machine_are_not_collapsed(self):
        from tests.boot.base import BootTypingTest
        from tests.boot.session import rotate
        events = []

        class First(BmcTest):
            name, machine, suite = "first-c64", "C64", "boot"

            def run_test(self):
                events.append(self.name)

        class Second(First):
            name = "second-c64"

        with tempfile.TemporaryDirectory() as directory, \
                patch("framework.runner.failure_evidence"), \
                patch("sys.stdout", new_callable=io.StringIO):
            output = Path(directory)

            def session(args, cases, test_types):
                report = {"cases": [], "evidence": "result.json"}
                rotate(Mock(), Mock(), Mock(), None, cases, args.timeout, output,
                       report, test_types)
                return report

            with patch("tests.boot.session.main", side_effect=session):
                results, _, _ = BootTypingTest.run_suite(
                    argparse.Namespace(timeout=1), [First, Second],
                    [{"machine": "C64", "id": "c64-test"}])
            self.assertEqual([result["name"] for result in results], ["first-c64", "second-c64"])
            self.assertEqual([result["status"] for result in results], ["PASS", "PASS"])
            self.assertTrue((output / "01-c64" / "result.json").is_file())
            self.assertTrue((output / "02-c64" / "result.json").is_file())
        self.assertEqual(events, ["first-c64", "second-c64"])

    def test_nested_suite_selector_includes_descendants_only(self):
        class First(BmcTest):
            name, suite = "nested", "boot/c64"

            def run_test(self):
                pass

        class Second(First):
            name, suite = "unrelated", "basic"

        self.assertEqual(select_tests([First, Second], ["boot"]), [First])
        self.assertEqual(select_tests([First, Second], ["boot/c64"]), [First])

    def test_default_suite_stops_after_interruption(self):
        class First(BmcTest):
            name, suite = "first", "example"

            def run_test(self):
                pass

        class Second(First):
            name = "second"

        with patch("framework.session.run", return_value={
                "name": "first", "status": "FAIL", "category": "interrupted",
                "error": "interrupted", "evidence": "first.json"}) as standalone:
            results, evidence, blocked = First.run_suite(
                argparse.Namespace(), [First, Second], [])
        standalone.assert_called_once()
        self.assertEqual(results[1]["status"], "NOT_RUN")
        self.assertEqual(evidence, ["first.json"])
        self.assertEqual(blocked, "interrupted")


if __name__ == "__main__":
    unittest.main()
