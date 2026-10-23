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
# Builds the profiles tests with the PC's C compiler and runs them.
# Used by make_all.sh (before the long build) and by the Makefile.
# The Pi's SD card layer (src/sdcard/sd_fs_fatfs.c) is swapped for one that
# uses a temporary folder on the PC (tools/sdcard/sd_fs_posix.c).
#

cd "$(dirname "$0")/../.." || exit 1

CC="${PROFILES_TEST_CC:-cc}"
BIN=build/.profiles_test

if ! command -v "$CC" >/dev/null 2>&1
then
       echo "A host C compiler is required for the profiles tests" >&2
       exit 1
fi

mkdir -p build
"$CC" -std=gnu11 -Wall -Wextra -Werror -Isrc/profiles \
       tools/profiles_test/profiles_test.c tools/sdcard/sd_fs_posix.c \
       src/sdcard/sd_fs_file.c src/profiles/*.c -o "$BIN" || exit 1
"./$BIN"
