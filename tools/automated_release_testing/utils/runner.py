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

"""Run discovered BMC64 hardware tests by name or suite folder."""

import argparse
import json
import math
import os
import shutil
import sys
from pathlib import Path
from hardware.api import Api
from framework.reporting import print_summary
from framework.discovery import discover, select_tests
from framework.runner import run_selected
from support.profiles import read_manifest

HERE = Path(__file__).resolve().parents[1]


def parser() -> argparse.ArgumentParser:
    result = argparse.ArgumentParser(description=__doc__)
    result.add_argument("--config", type=Path,
                        help="saved setup JSON; explicit command-line options override it")
    result.add_argument("--execute", action="store_true",
                        help="allow PRG upload/autostart, capture and Pico input")
    result.add_argument("--list", action="store_true", help="list the test names and exit")
    result.add_argument("tests", nargs="*", metavar="TEST",
                        help="suite folders or test names (see --list); default: every test")
    result.add_argument("--manifest", type=Path, default=HERE / "suite_profiles.json",
                        help="profile manifest for the boot/typing tests")
    result.add_argument("--url", help="BMC64 base URL, including port if needed")
    result.add_argument("--network", choices=("wifi", "ethernet"),
                        help="require this network setting in the selected test profiles")
    result.add_argument("--pico-uart", help="exclusive Pico serial adapter")
    result.add_argument("--bmc-uart", help="optional exclusive diagnostic UART")
    result.add_argument("--ask-pin", action="store_true",
                        help="prompt without storing the Web UI PIN")
    result.add_argument("--video", default="/dev/v4l/by-id/usb-MACROSILICON_2109-video-index0")
    result.add_argument("--audio", default="plughw:CARD=MS2109,DEV=0")
    result.add_argument("--setup", default="", help="board/profile/ROM/filter notes")
    result.add_argument("--timeout", type=float, default=60,
                        help="deadline in seconds for each guest assertion")
    result.add_argument("--results", type=Path, default=HERE / "results")
    return result


def load_setup(path: Path) -> dict[str, object]:
    settings = json.loads(path.read_text(encoding="utf-8"))
    string_fields = {"url", "pico_uart", "bmc_uart", "video", "audio", "setup"}
    allowed = string_fields | {"ask_pin", "timeout", "network"}
    if not isinstance(settings, dict) or settings.keys() - allowed:
        raise ValueError("setup must be a JSON object containing only supported setup options")
    for name in string_fields & settings.keys():
        value = settings[name]
        if name == "bmc_uart" and value is None:
            continue
        if not isinstance(value, str) or any(char in value for char in "\r\n\0"):
            raise ValueError(f"setup option {name} must be a single-line string")
    for name in ("url", "pico_uart"):
        if not settings.get(name):
            raise ValueError(f"setup requires {name}")
    # Constructing the client validates the URL without making a request.
    Api(settings["url"])
    for name in ("pico_uart", "bmc_uart", "video"):
        if settings.get(name) and not Path(settings[name]).is_absolute():
            raise ValueError(f"setup option {name} must be an absolute device path")
    for name in ("video", "audio"):
        if name in settings and not settings[name]:
            raise ValueError(f"setup option {name} must not be empty")
    if "ask_pin" in settings and not isinstance(settings["ask_pin"], bool):
        raise ValueError("setup option ask_pin must be true or false")
    if "network" in settings and settings["network"] not in ("wifi", "ethernet"):
        raise ValueError("setup option network must be wifi or ethernet")
    if "timeout" in settings:
        timeout = settings["timeout"]
        if (isinstance(timeout, bool) or not isinstance(timeout, (int, float))
                or not math.isfinite(timeout) or timeout <= 0):
            raise ValueError("setup option timeout must be a finite positive number")
    return settings


def parse_arguments(argv: list[str] | None = None) -> argparse.Namespace:
    argument_parser = parser()
    selector = argparse.ArgumentParser(add_help=False)
    selector.add_argument("--config", type=Path)
    selected, _ = selector.parse_known_args(argv)
    if selected.config:
        try:
            argument_parser.set_defaults(**load_setup(selected.config))
        except (OSError, ValueError) as error:
            argument_parser.error(f"cannot load setup {selected.config}: {error}")
    return argument_parser.parse_args(argv)


def main() -> int:
    argument_parser = parser()
    args = parse_arguments()
    try:
        manifest = read_manifest(args.manifest)
        machines = {case["machine"] for case in manifest}
        available = [test for test in discover()
                     if not test.profile_required or test.machine in machines]
        selected = select_tests(available, args.tests)
    except (OSError, ValueError) as error:
        argument_parser.error(str(error))
    if args.list:
        width = max(len(test.name) for test in available)
        for test in available:
            print(f"{test.name:<{width}}  {test.description} [suite: {test.suite}]")
        return 0
    if not math.isfinite(args.timeout) or args.timeout <= 0:
        argument_parser.error("--timeout must be finite and positive")
    if args.execute and (not args.url or not args.pico_uart):
        argument_parser.error("--execute requires --url and --pico-uart")
    if args.bmc_uart and args.pico_uart and (
            os.path.realpath(args.bmc_uart) == os.path.realpath(args.pico_uart)):
        argument_parser.error("Pico and BMC64 UART paths must be different")
    required = {tool for test in selected for tool in test.requirements}
    if args.execute:
        required.add("ffmpeg")
    for tool in sorted(required):
        if shutil.which(tool) is None:
            argument_parser.error(f"required tool is missing: {tool}; see docs/prerequisites.md")
    results, evidence = run_selected(args, selected, manifest)
    print_summary(results)
    for path in evidence:
        print(f"  Evidence: {path}", flush=True)
    return 0 if all(result["status"] in ("PASS", "PREPARED") for result in results) else 1


if __name__ == "__main__":
    sys.exit(main())
