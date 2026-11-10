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

"""The guest assembler, and optionally the guest itself running in host VICE."""

import os
import re
import shutil
import socket
import subprocess
import tempfile
import time
import unittest
from pathlib import Path

from support.constants import HEIGHT, WIDTH
from tests.basic.assembler import AssemblyError, assemble
from tests.basic.diagnostic import decode_frame
from tests.basic.guest import build_guest

VICE_DIR = os.environ.get("BMC64_VICE_DIR")
RUN_ID = 0x1234ABCD


class AssemblerTests(unittest.TestCase):
    def test_addressing_modes_encode_as_6502(self):
        code, listing = assemble("""
value = $12
start:  lda #lo(table)
        lda value
        sta $d020
        lda table,x
        ldy table,x
        sta (value),y
        cpx value
        bne start
        jsr later
later:  rts
table:  .byte 1, $02, value
""", 0x1000)
        self.assertEqual(code.hex(" "), " ".join([
            "a9 17", "a5 12", "8d 20 d0", "bd 17 10", "bc 17 10", "91 12", "e4 12",
            "d0 ed", "20 16 10", "60", "01 02 12"]))
        self.assertTrue(listing[0].startswith("$1000  a9 17"))

    def test_errors_are_explicit(self):
        for source, message in (("        ror", "unsupported instruction"),
                                ("        jmp (vector)", "unknown name|does not support"),
                                ("        lda missing", "unknown name missing"),
                                ("        ldx $1234,x", "does not support absx"),
                                ("        lda #$100", "does not fit")):
            with self.subTest(source=source), self.assertRaisesRegex(AssemblyError, message):
                assemble(source, 0x1000)
        far = "start:\n        .byte " + ", ".join(["0"] * 200) + "\n        bne start\n"
        with self.assertRaisesRegex(AssemblyError, "branch out of range"):
            assemble(far, 0x1000)


@unittest.skipUnless(VICE_DIR and shutil.which("x64"),
                     "set BMC64_VICE_DIR to a VICE data directory with C64 ROMs")
class GuestInViceTests(unittest.TestCase):
    """Runs the PRG in host VICE x64 and reads its screen through the remote monitor."""

    PORT = 6510
    PALETTE = {0: (0, 0, 0), 1: (255, 255, 255), 2: (136, 57, 50)}

    def monitor(self, *commands: str) -> str:
        output = b""
        with socket.create_connection(("127.0.0.1", self.PORT), timeout=5) as sock:
            sock.settimeout(0.5)
            for command in (*commands, "x"):
                sock.sendall(command.encode() + b"\n")
                time.sleep(0.3)
                try:
                    while chunk := sock.recv(65536):
                        output += chunk
                except socket.timeout:
                    pass
        return output.decode("latin-1")

    @staticmethod
    def memory(text: str, start: int, end: int) -> bytes:
        values = {}
        for address, data in re.findall(r">C:([0-9a-f]{4})\s+((?:[0-9a-f]{2}\s+)+)", text):
            for index, byte in enumerate(data.split()):
                values[int(address, 16) + index] = int(byte, 16)
        return bytes(values[address] for address in range(start, end + 1))

    def screen(self):
        """Render colour RAM inside a black border at the analysis size, then decode it."""
        colour = self.memory(self.monitor("m d800 dbe7"), 0xD800, 0xDBE7)
        rgb = bytearray(WIDTH * HEIGHT * 3)
        for y in range(HEIGHT):
            for x in range(WIDTH):
                px, py = x * 384 // WIDTH - 32, y * 272 // HEIGHT - 36
                if 0 <= px < 320 and 0 <= py < 200:
                    offset = (y * WIDTH + x) * 3
                    rgb[offset:offset + 3] = bytes(self.PALETTE[colour[py // 8 * 40 + px // 8] & 15])
        return decode_frame(bytes(rgb), RUN_ID)

    def test_marker_counter_and_space_response(self):
        with tempfile.TemporaryDirectory() as directory:
            program = build_guest(RUN_ID, Path(directory))
            vice = subprocess.Popen(
                ["x64", "-default", "-directory", VICE_DIR, "-console", "-silent",
                 "-sounddev", "dummy", "-remotemonitor",
                 "-remotemonitoraddress", f"ip4://127.0.0.1:{self.PORT}",
                 "-autostart", str(program)],
                stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
            try:
                time.sleep(6)
                first = self.screen()
                time.sleep(1)
                second = self.screen()
                self.monitor('keybuf " "')
                time.sleep(1)
                third = self.screen()
            finally:
                vice.terminate()
                vice.wait(5)
        self.assertIsNotNone(first, "no valid marker on the guest screen")
        self.assertEqual((first.state, second.state, third.state), (0, 0, 1))
        self.assertNotEqual(first.counter, second.counter)


if __name__ == "__main__":
    unittest.main()
