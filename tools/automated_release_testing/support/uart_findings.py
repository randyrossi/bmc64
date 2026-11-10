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

import re


def classify_uart(line: str) -> dict[str, str] | None:
    # VICE probes PRGs as disk/tape/cartridge images before accepting them.
    probes = (
        r"Filesystem Image: Unknown disk image ",
        r"Tape: Error - Cannot open file ",
        r"Error - CRT header invalid\.",
        r"CART: .* is not a valid CRT file\.",
        r"Error - archdep_mkdir unimpl:",
    )
    if any(re.search(pattern, line) for pattern in probes):
        return {"severity": "info", "category": "autostart_probe", "line": line}
    # Every boot reports the printer, plotter and drive ROMs BMC64 does not ship.
    startup = (
        r"^MPS-803: Error - (?:Could not load MPS-803 charset|Cannot load palette file)",
        r"^NL10: Error - Could not load NL-10 ROM file",
        r"^Palette: Error - Palette not found: `(?:mps803|1520)\.vpl'",
        r"^plot1520: Error - Cannot load palette file",
        r"^DriveROM: Error - \S+ ROM image not found\. "
        r"Hardware-level \S+ emulation is not available\.",
        r"^Warning - ROM `[^']*/dosCMDHD': long file, discarding end\.",
    )
    if any(re.search(pattern, line) for pattern in startup):
        return {"severity": "info", "category": "boot_startup", "line": line}
    critical = (
        ("panic", r"\bpanic\b"),
        ("exception", r"\b(?:data abort|prefetch abort|unhandled exception)\b"),
        ("assertion", r"\bassert(?:ion)?\s+(?:failed|failure)\b"),
        ("memory", r"\b(?:KASAN|out of memory|failed to allocate)\b"),
        ("profile", r"\bprofile\b.*\b(?:failed|error|fallback)\b"),
    )
    for category, pattern in critical:
        if re.search(pattern, line, re.IGNORECASE):
            return {"severity": "critical", "category": category, "line": line}
    if re.search(r"\b(?:error|failed|warning)\b", line, re.IGNORECASE):
        return {"severity": "warning", "category": "unclassified", "line": line}
    return None
