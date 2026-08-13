#ifndef BMC64_CIRCLE_GLUE_WRAPPER_H
#define BMC64_CIRCLE_GLUE_WRAPPER_H

#include <circle/serial.h>
#include "third_party/circle-stdlib/include/circle_glue.h"

#define MAX_BOOTSTAT_LINES 32
#define MAX_BOOTSTAT_FLEN 64

#define BOOTSTAT_WHAT_STAT 0
#define BOOTSTAT_WHAT_FAIL 1

void CGlueStdioInit(CSerialDevice *serial);
void CGlueStdioInitBootStat(int num,
	int *mBootStatWhat,
	const char **mBootStateFile,
	int *mBootStatSize);
void CGlueStdioSetPartitionForVolume(const char* volume, int p, unsigned int ss);

#endif