#
# test_license_assets.py
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
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <https://www.gnu.org/licenses/>.
#

import contextlib
import io
import json
from pathlib import Path
import re
import subprocess
import sys
import tempfile
import unittest
from unittest import mock

sys.path.insert(0, str(Path(__file__).resolve().parents[1]))
import gen_license_assets as generator


class LicenseAssetsTest(unittest.TestCase):
    def test_full_license_is_preserved_without_truncation(self):
        root = generator.REPO_ROOT
        assets = generator.generated_assets(root)
        content = assets[root / "third_party/common/license_text.inc"].decode()
        lines = [json.loads(line.strip().removesuffix(","))
                 for line in content.splitlines()[1:]]
        original = (root / "LICENSE").read_text(encoding="ascii")
        self.assertEqual(re.sub(r"\s", "", "".join(lines)),
                         re.sub(r"\s", "", original))
        self.assertGreater(len(lines), 510)
        self.assertTrue(all(len(line) <= 35 for line in lines))
        self.assertIn("Version 3, 29 June 2007", lines)
        self.assertTrue("".join(lines).endswith(
            "<http://www.gnu.org/philosophy/why-not-lgpl.html>."))

    def test_license_copies_are_byte_identical(self):
        root = generator.REPO_ROOT
        assets = generator.generated_assets(root)
        for name, source in generator.LICENSE_COPIES.items():
            with self.subTest(name=name):
                self.assertEqual(assets[root / "sdcard/licenses" / name],
                                 (root / source).read_bytes())

    def test_wraps_blank_lines_tabs_and_long_words(self):
        text = '  "quoted" \\\\ text\twords\n\n' + "a" * 80 + "\n"
        lines = generator.osd_lines(text, 35)
        self.assertIn("", lines)
        self.assertTrue(all(len(line) <= 35 for line in lines))
        self.assertEqual(re.sub(r"\s", "", "".join(lines)),
                         re.sub(r"\s", "", text))

    def test_invalid_input_is_explicit(self):
        for text, width in [("", 35), (" \n", 35), ("text", 0),
                            ("copyright \u00a9", 35), ("bad\x00text", 35)]:
            with self.subTest(text=text, width=width):
                with self.assertRaises(ValueError):
                    generator.osd_lines(text, width)

    def test_check_mode_is_read_only_and_detects_stale_files(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            folder = root / "sdcard/licenses"
            folder.mkdir(parents=True)
            for name in ("README.txt", "Apache-2.0.txt", "crt-pi.txt",
                         "GCC-Runtime.txt", "RaspberryPi-Firmware.txt"):
                (folder / name).write_text("notice\n")
            target = folder / "GPL-3.0.txt"
            with mock.patch.object(generator, "REPO_ROOT", root), \
                 mock.patch.object(generator, "generated_assets",
                                   return_value={target: b"license\n"}), \
                 mock.patch.object(sys, "argv", ["generator", "--check"]), \
                 contextlib.redirect_stderr(io.StringIO()):
                self.assertEqual(generator.main(), 1)
                self.assertFalse(target.exists())
                target.write_bytes(b"stale\n")
                self.assertEqual(generator.main(), 1)
                self.assertEqual(target.read_bytes(), b"stale\n")
                target.write_bytes(b"license\n")
                self.assertEqual(generator.main(), 0)

    def test_generator_does_not_rewrite_unchanged_files(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            folder = root / "sdcard/licenses"
            folder.mkdir(parents=True)
            for name in ("README.txt", "Apache-2.0.txt", "crt-pi.txt",
                         "GCC-Runtime.txt", "RaspberryPi-Firmware.txt"):
                (folder / name).write_text("notice\n")
            target = folder / "GPL-3.0.txt"
            with mock.patch.object(generator, "REPO_ROOT", root), \
                 mock.patch.object(generator, "generated_assets",
                                   return_value={target: b"license\n"}), \
                 mock.patch.object(sys, "argv", ["generator"]):
                self.assertEqual(generator.main(), 0)
                modified = target.stat().st_mtime_ns
                self.assertEqual(generator.main(), 0)
                self.assertEqual(target.stat().st_mtime_ns, modified)

    def test_missing_source_and_menu_overflow_fail(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            common = root / "third_party/common"
            common.mkdir(parents=True)
            header = common / "text.h"
            header.write_text("#define LICENSE_LINE_WIDTH 36\n")
            (common / "ui.h").write_text("#define MAX_MENU_STR 36\n")
            with self.assertRaisesRegex(ValueError, "truncated"):
                generator.generated_assets(root)
            header.write_text("#define LICENSE_LINE_WIDTH 35\n")
            with self.assertRaises(FileNotFoundError):
                generator.generated_assets(root)

    def test_release_check_rejects_missing_or_empty_license(self):
        root = generator.REPO_ROOT
        script = root / "check_release_files.sh"
        text = script.read_text()
        files_section = text.split("REQUIRED_FILES=(", 1)[1].split("\n)", 1)[0]
        dirs_section = text.split("REQUIRED_DIRECTORIES=(", 1)[1].split("\n)", 1)[0]
        required_files = re.findall(r'^\s+"([^"]+)"', files_section, re.M)
        required_dirs = re.findall(r'^\s+"([^"]+)"', dirs_section, re.M)
        pi3_section = text.split("PI3_KERNEL_FILES=(", 1)[1].split("\n)", 1)[0]
        required_files += re.findall(r'^\s+"([^"]+)"', pi3_section, re.M)
        required_files.append("bmc64-manifest.txt")
        with tempfile.TemporaryDirectory() as directory:
            stage = Path(directory)
            for name in required_dirs:
                (stage / name).mkdir(parents=True, exist_ok=True)
            for name in required_files:
                destination = stage / name
                destination.parent.mkdir(parents=True, exist_ok=True)
                destination.write_bytes(b"fixture\n")

            def check():
                return subprocess.run(["bash", str(script), str(stage), "pi3"],
                                      capture_output=True, text=True)

            self.assertEqual(check().returncode, 0)
            notice = stage / "licenses/Apache-2.0.txt"
            notice.unlink()
            missing = check()
            self.assertNotEqual(missing.returncode, 0)
            self.assertIn("licenses/Apache-2.0.txt", missing.stdout)
            notice.write_bytes(b"")
            empty = check()
            self.assertNotEqual(empty.returncode, 0)
            self.assertIn("empty license notice", empty.stdout)

    def test_every_bundled_notice_is_required_by_release_check(self):
        root = generator.REPO_ROOT
        script = (root / "check_release_files.sh").read_text()
        section = script.split("REQUIRED_FILES=(", 1)[1].split("\n)", 1)[0]
        required = set(re.findall(r'^\s+"([^"]+)"', section, re.M))
        for notice in (root / "sdcard/licenses").glob("*.txt"):
            with self.subTest(notice=notice.name):
                self.assertIn(f"licenses/{notice.name}", required)


if __name__ == "__main__":
    unittest.main()
