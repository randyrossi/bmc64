//
// bootstat_plus4.h
//
// BMC64 - A bare metal Commodore emulator for Raspberry Pi
// Copyright (C) 2019 Randy Rossi <randy.rossi@gmail.com>
// Copyright (C) 2026 aminch <https://github.com/aminch>
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.
//

#ifndef BOOTSTAT_PLUS4
#define BOOTSTAT_PLUS4

int dflt_bootStatNum = 20;

int dflt_bootStatWhat[] = {
    BOOTSTAT_WHAT_STAT,
    BOOTSTAT_WHAT_STAT,
    BOOTSTAT_WHAT_STAT,
    BOOTSTAT_WHAT_STAT,
    BOOTSTAT_WHAT_STAT,
    BOOTSTAT_WHAT_STAT,
    BOOTSTAT_WHAT_STAT,
    BOOTSTAT_WHAT_STAT,
    BOOTSTAT_WHAT_FAIL,
    BOOTSTAT_WHAT_FAIL,
    BOOTSTAT_WHAT_FAIL,
    BOOTSTAT_WHAT_FAIL,
    BOOTSTAT_WHAT_FAIL,
    BOOTSTAT_WHAT_FAIL,
    BOOTSTAT_WHAT_FAIL,
    BOOTSTAT_WHAT_FAIL,
    BOOTSTAT_WHAT_FAIL,
    BOOTSTAT_WHAT_FAIL,
    BOOTSTAT_WHAT_FAIL,
};

const char *dflt_bootStatFile[] = {
    "kernal",
    "kernal.005",
    "kernal.232",
    "kernal.364",
    "basic",
    "3plus1hi",
    "3plus1lo", 
    "c2lo.364",
    "mps803",
    "mps803.vpl",
    "nl10-cbm",
    "1520.vpl",
    "dos1540",
    "dos1570",
    "dos2031",
    "dos2040",
    "dos3040",
    "dos4040",
    "dos1001",
};
int dflt_bootStatSize[] = {
    16384,
    16384,
    16384,
    16384,
    16384,
    16384,
    16384,
    16384,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
};

#endif
