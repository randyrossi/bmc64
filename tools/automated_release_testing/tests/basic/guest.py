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

"""The C64 diagnostic guest: a machine-code PRG with a BASIC SYS line.

The screen is black with a red 32x18-cell block (rows 3-20, columns 4-35).
Inside it, the 80 bits of payload() are 3x2-cell squares, white for 1 and
black for 0: bit n is at row 4 + 2 * (n // 10), column 5 + 3 * (n % 10).
diagnostic.decode_frame() reads exactly this layout.

A SID sawtooth plays at about 1 kHz. Every 10 frames the counter advances
and the counter, state and checksum bits are redrawn while the raster is in
the bottom border, so a captured frame never shows a half-drawn marker.
Space, read through the KERNAL keyboard buffer like BASIC's GET, sets the
state to 1 and gates the tone off.
"""

from pathlib import Path

from .assembler import assemble
from .diagnostic import payload

ORIGIN = 0x0801
FRAMES_PER_UPDATE = 10


def bit_tables() -> dict[str, list[int]]:
    """Colour RAM address, payload byte and mask for each of the 80 bits."""
    addresses = [0xD800 + (4 + 2 * (bit // 10)) * 40 + 5 + 3 * (bit % 10) for bit in range(80)]
    return {
        "bit_lo": [address & 0xFF for address in addresses],
        "bit_hi": [address >> 8 for address in addresses],
        "bit_byte": [bit // 8 for bit in range(80)],
        "bit_mask": [0x80 >> (bit % 8) for bit in range(80)],
    }


def source(run_id: int) -> str:
    tables = "\n".join(f"{name}:  .byte {', '.join(map(str, values))}"
                       for name, values in bit_tables().items())
    return f"""; BMC64 C64 diagnostic guest, run {run_id:08x}
border      = $d020
background  = $d021
raster      = $d012
sid_freq_lo = $d400
sid_freq_hi = $d401
sid_control = $d404
sid_attack  = $d405
sid_sustain = $d406
sid_volume  = $d418
getin       = $ffe4         ; KERNAL: next key from the buffer, 0 if none
pointer     = $fb           ; and $fc: colour RAM cell being drawn
last_bit    = $fd           ; draw stops before this bit
frames      = $fe           ; frames left until the next update
update      = {FRAMES_PER_UPDATE}

start:  lda #0
        sta border
        sta background
        ldx #0
fill:   lda #160            ; reverse space: every cell solid
        sta $0400,x
        sta $0500,x
        sta $0600,x
        sta $0700,x
        lda #0              ; black
        sta $d800,x
        sta $d900,x
        sta $da00,x
        sta $db00,x
        inx
        bne fill

        lda #lo($d800 + 3 * 40 + 4)   ; red block: rows 3-20, columns 4-35
        sta pointer
        lda #hi($d800 + 3 * 40 + 4)
        sta pointer + 1
        ldx #18
block:  ldy #31
        lda #2
block_cell:
        sta (pointer),y
        dey
        bpl block_cell
        lda pointer
        clc
        adc #40
        sta pointer
        bcc block_next
        inc pointer + 1
block_next:
        dex
        bne block

        lda #8              ; SID: volume 8, sawtooth at about 1 kHz, held
        sta sid_volume
        lda #132
        sta sid_freq_lo
        lda #66
        sta sid_freq_hi
        lda #0
        sta sid_attack
        lda #240
        sta sid_sustain
        lda #33
        sta sid_control

        ldx #0              ; draw all 80 bits
        lda #80
        sta last_bit
        jsr draw

main:   lda #update
        sta frames
frame:  lda raster          ; wait for the next frame's line 251 (bottom border)
        cmp #251
        beq frame
wait:   lda raster
        cmp #251
        bne wait
        jsr getin
        cmp #32
        bne no_space
        lda #1              ; Space: state 1, tone gated off
        sta marker + 7
        lda #32
        sta sid_control
no_space:
        dec frames
        bne frame

        inc marker + 6      ; counter, then checksum of bytes 0-7
        lda #0
        ldx #7
sum:    clc
        adc marker,x
        dex
        bpl sum
        sta marker + 8
        ldx #48             ; redraw bytes 6-8: bits 48-71
        lda #72
        sta last_bit
        jsr draw
        jmp main

draw:   lda bit_lo,x        ; draw bits X .. last_bit - 1
        sta pointer
        lda bit_hi,x
        sta pointer + 1
        ldy bit_byte,x
        lda marker,y
        and bit_mask,x
        beq draw_cell       ; 0: black
        lda #1              ; 1: white
draw_cell:
        ldy #0
        sta (pointer),y
        iny
        sta (pointer),y
        iny
        sta (pointer),y
        ldy #40
        sta (pointer),y
        iny
        sta (pointer),y
        iny
        sta (pointer),y
        inx
        cpx last_bit
        bne draw
        rts

marker: .byte {', '.join(map(str, payload(run_id, 0, 0)))}
{tables}
"""


def basic_stub(run_id: int) -> tuple[bytes, int]:
    """10 SYS <start>:REM BMC64 SMOKE <run id>; returns the stub and start address."""
    remark = f" BMC64 SMOKE {run_id:08X}".encode("ascii")
    length = 2 + 2 + 1 + 4 + 1 + 1 + len(remark) + 1 + 2
    start = ORIGIN + length
    line_end = start - 2
    line = (bytes((0x9E,)) + str(start).encode("ascii") + b":" + bytes((0x8F,)) + remark + b"\0")
    stub = line_end.to_bytes(2, "little") + (10).to_bytes(2, "little") + line + b"\0\0"
    assert len(stub) == length and len(str(start)) == 4
    return stub, start


def build_guest(run_id: int, output: Path) -> Path:
    """Write diagnostic.asm, its listing and diagnostic.prg; returns the PRG."""
    stub, start = basic_stub(run_id)
    text = source(run_id)
    code, listing = assemble(text, start)
    (output / "diagnostic.asm").write_text(text, encoding="ascii")
    (output / "diagnostic.lst").write_text("\n".join(listing) + "\n", encoding="ascii")
    program = output / "diagnostic.prg"
    program.write_bytes(ORIGIN.to_bytes(2, "little") + stub + code)
    return program
