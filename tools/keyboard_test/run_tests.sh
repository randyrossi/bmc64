#!/bin/bash
#
# run_tests.sh
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
# Builds the keyboard tests with the PC's C compiler and runs them.
# Used by make_all.sh (before the long build) and by the Makefile.
#

cd "$(dirname "$0")/../.." || exit 1

CC="${KEYBOARD_TEST_CC:-cc}"
BIN=build/.keyboard_router_test

if ! command -v "$CC" >/dev/null 2>&1
then
       echo "A host C compiler is required for the keyboard tests" >&2
       exit 1
fi

mkdir -p build
"$CC" -std=c11 -Wall -Wextra -Werror -Wno-unused-parameter \
       -ffunction-sections -fdata-sections -Ithird_party/common \
       -Ithird_party/circle-stdlib/include \
       -Ithird_party/circle-stdlib/libs/circle/include \
       -Wl,--gc-sections tools/keyboard_test/keyboard_router_test.c \
       src/keyboard/keyboard_router.c third_party/common/kbd.c \
       src/keyboard/keyboard_layout.c src/keyboard/layout_*.c -o "$BIN" || exit 1
"./$BIN"
