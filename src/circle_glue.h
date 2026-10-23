//
// circle_glue.h
//
// BMC64 - A bare metal Commodore emulator for Raspberry Pi
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

#ifndef BMC64_CIRCLE_GLUE_WRAPPER_H
#define BMC64_CIRCLE_GLUE_WRAPPER_H

#include <circle/device.h>
#include "../third_party/circle-stdlib/include/circle_glue.h"

#define MAX_BOOTSTAT_LINES 32
#define MAX_BOOTSTAT_FLEN 64

#define BOOTSTAT_WHAT_STAT 0
#define BOOTSTAT_WHAT_FAIL 1

void CGlueStdioInit(CDevice *output_device);
void CGlueStdioInitBootStat(int num,
	int *mBootStatWhat,
	const char **mBootStateFile,
	int *mBootStatSize);
void CGlueStdioSetPartitionForVolume(const char* volume, int p, unsigned int ss);

#endif