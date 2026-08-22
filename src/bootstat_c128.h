#ifndef BOOTSTAT_C128
#define BOOTSTAT_C128

int dflt_bootStatNum = 20;

int dflt_bootStatWhat[] = {
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
    BOOTSTAT_WHAT_FAIL,
    BOOTSTAT_WHAT_FAIL,
    BOOTSTAT_WHAT_FAIL,
};

const char *dflt_bootStatFile[] = {
    "kernal",
    "kernal64",
    "basic64",
    "basichi",
    "basiclo",
    "chargen",
    "fliplist-C128.vfl",
    "mps803",
    "mps803.vpl",
    "nl10-cbm",
    "1520.vpl",
    "dos1540",
    "dos1570",
    "dos2000",
    "dos4000",
    "dos2031",
    "dos2040",
    "dos3040",
    "dos4040",
    "dos1001",
};
int dflt_bootStatSize[] = {
    16384,
    8192,
    8192,
    16384,
    16384,
    8192,
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
    0,
    0,
    0
};

#endif
