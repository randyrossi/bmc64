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

"""A minimal two-pass 6502 assembler for the diagnostic guest.

Only the instructions and addressing modes the guest needs are supported;
anything else is an error rather than a guess. Syntax, one statement per line:

    label:                   ; a comment
    name = expression        ; a constant
            lda #lo(table)   ; immediate
            sta $d020        ; zero page if the value is below $100, else absolute
            lda table,x      ; absolute,X / absolute,Y
            sta (pointer),y  ; (zero page),Y
            bne label        ; relative branch
            .byte 1, $02, name

Expressions are Python arithmetic over numbers ($hex or decimal), names,
lo() and hi().
"""

import re

# mnemonic -> {mode: opcode}
OPCODES = {
    "adc": {"imm": 0x69, "zp": 0x65, "abs": 0x6D, "absx": 0x7D, "absy": 0x79},
    "and": {"imm": 0x29, "abs": 0x2D, "absx": 0x3D, "absy": 0x39},
    "beq": {"rel": 0xF0}, "bne": {"rel": 0xD0}, "bpl": {"rel": 0x10},
    "bcc": {"rel": 0x90}, "bcs": {"rel": 0xB0},
    "clc": {"imp": 0x18}, "cli": {"imp": 0x58}, "sei": {"imp": 0x78},
    "cmp": {"imm": 0xC9, "zp": 0xC5, "abs": 0xCD},
    "cpx": {"imm": 0xE0, "zp": 0xE4}, "cpy": {"imm": 0xC0, "zp": 0xC4},
    "dec": {"zp": 0xC6, "abs": 0xCE}, "inc": {"zp": 0xE6, "abs": 0xEE},
    "dex": {"imp": 0xCA}, "dey": {"imp": 0x88}, "inx": {"imp": 0xE8}, "iny": {"imp": 0xC8},
    "jmp": {"abs": 0x4C}, "jsr": {"abs": 0x20}, "rts": {"imp": 0x60},
    "lda": {"imm": 0xA9, "zp": 0xA5, "abs": 0xAD, "absx": 0xBD, "absy": 0xB9, "indy": 0xB1},
    "ldx": {"imm": 0xA2, "zp": 0xA6, "abs": 0xAE, "absy": 0xBE},
    "ldy": {"imm": 0xA0, "zp": 0xA4, "abs": 0xAC, "absx": 0xBC},
    "sta": {"zp": 0x85, "abs": 0x8D, "absx": 0x9D, "absy": 0x99, "indy": 0x91},
    "stx": {"zp": 0x86, "abs": 0x8E}, "sty": {"zp": 0x84, "abs": 0x8C},
}
SIZES = {"imp": 1, "imm": 2, "zp": 2, "rel": 2, "indy": 2, "abs": 3, "absx": 3, "absy": 3}


class AssemblyError(ValueError):
    pass


def evaluate(text: str, names: dict[str, int], strict: bool) -> int | None:
    """Evaluate an expression; None for a name not defined yet, unless strict (pass 2)."""
    expression = re.sub(r"\$([0-9a-fA-F]+)", r"0x\1", text)
    for name in re.findall(r"\b[A-Za-z_]\w*", expression):
        if name not in names and name not in ("lo", "hi"):
            if strict:
                raise AssemblyError(f"unknown name {name}")
            return None
    scope = {**names, "lo": lambda value: value & 0xFF, "hi": lambda value: (value >> 8) & 0xFF}
    return int(eval(expression, {"__builtins__": {}}, scope))


def parse(operand: str) -> tuple[str, str]:
    """Return (mode, expression) before choosing zero page or absolute."""
    if not operand:
        return "imp", ""
    if operand.startswith("#"):
        return "imm", operand[1:]
    match = re.fullmatch(r"\((.+)\)\s*,\s*y", operand, re.IGNORECASE)
    if match:
        return "indy", match[1]
    match = re.fullmatch(r"(.+?)\s*,\s*([xy])", operand, re.IGNORECASE)
    if match:
        return "abs" + match[2].lower(), match[1]
    return "addr", operand


def assemble(source: str, origin: int) -> tuple[bytes, list[str]]:
    """Assemble source at origin; returns the code and a listing."""
    statements = []
    for number, raw in enumerate(source.splitlines(), 1):
        line = raw.split(";", 1)[0].strip()
        while (match := re.match(r"([A-Za-z_]\w*):\s*", line)):
            statements.append((number, raw, "label", match[1]))
            line = line[match.end():]
        if not line:
            continue
        if (match := re.fullmatch(r"([A-Za-z_]\w*)\s*=\s*(.+)", line)):
            statements.append((number, raw, "constant", (match[1], match[2])))
        elif line.startswith(".byte"):
            statements.append((number, raw, "byte", line[5:].split(",")))
        else:
            mnemonic, _, operand = line.partition(" ")
            statements.append((number, raw, "op", (mnemonic.lower(), operand.strip())))

    names: dict[str, int] = {}
    for strict in (False, True):
        address, code, listing = origin, bytearray(), []
        for number, raw, kind, value in statements:
            try:
                if kind == "label":
                    names[value] = address
                    continue
                if kind == "constant":
                    constant = evaluate(value[1], names, True)
                    names[value[0]] = constant
                    continue
                if kind == "byte":
                    data = bytes((evaluate(item, names, strict) or 0) & 0xFF for item in value)
                else:
                    data = encode(*value, address, names, strict)
            except AssemblyError as error:
                raise AssemblyError(f"line {number}: {error}: {raw.strip()}") from None
            listing.append(f"${address:04X}  {data.hex(' '):<9}  {raw.rstrip()}")
            code += data
            address += len(data)
    return bytes(code), listing


def encode(mnemonic: str, operand: str, address: int, names: dict[str, int],
           strict: bool) -> bytes:
    modes = OPCODES.get(mnemonic)
    if modes is None:
        raise AssemblyError(f"unsupported instruction {mnemonic}")
    mode, expression = parse(operand)
    value = evaluate(expression, names, strict) if expression else 0
    if "rel" in modes:
        mode = "rel"
    elif mode == "addr":
        # A name not defined yet is a label after this line: absolute.
        mode = "zp" if value is not None and value < 0x100 and "zp" in modes else "abs"
    if mode not in modes:
        raise AssemblyError(f"{mnemonic} does not support {mode} addressing")
    opcode = modes[mode]
    if value is None:
        return bytes((opcode,)) + bytes(SIZES[mode] - 1)
    if mode == "imp":
        return bytes((opcode,))
    if mode == "rel":
        offset = value - (address + 2)
        if strict and not -128 <= offset <= 127:
            raise AssemblyError("branch out of range")
        return bytes((opcode, offset & 0xFF))
    if SIZES[mode] == 2:
        if strict and not 0 <= value <= 0xFF:
            raise AssemblyError(f"operand ${value:X} does not fit in a byte")
        return bytes((opcode, value & 0xFF))
    if strict and not 0 <= value <= 0xFFFF:
        raise AssemblyError(f"address ${value:X} out of range")
    return bytes((opcode, value & 0xFF, value >> 8))
