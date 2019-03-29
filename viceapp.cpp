//
// viceapp.c
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     https://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "viceapp.h"

int dflt_bootStatNum = 21;

int dflt_bootStatWhat[] = {
    BOOTSTAT_WHAT_STAT, BOOTSTAT_WHAT_STAT, BOOTSTAT_WHAT_STAT,
    BOOTSTAT_WHAT_STAT, BOOTSTAT_WHAT_FAIL, BOOTSTAT_WHAT_FAIL,
    BOOTSTAT_WHAT_FAIL, BOOTSTAT_WHAT_FAIL, BOOTSTAT_WHAT_FAIL,
    BOOTSTAT_WHAT_FAIL, BOOTSTAT_WHAT_FAIL, BOOTSTAT_WHAT_FAIL,
    BOOTSTAT_WHAT_FAIL, BOOTSTAT_WHAT_FAIL, BOOTSTAT_WHAT_FAIL,
    BOOTSTAT_WHAT_FAIL, BOOTSTAT_WHAT_FAIL, BOOTSTAT_WHAT_FAIL,
    BOOTSTAT_WHAT_FAIL, BOOTSTAT_WHAT_FAIL, BOOTSTAT_WHAT_FAIL,
};

const char *dflt_bootStatFile[] = {
    "kernal",           "basic",   "chargen",    "d1541II",  "rpi_pos.vkm",
    "fliplist-C64.vfl", "mps803",  "mps803.vpl", "nl10-cbm", "1520.vpl",
    "dos1540",          "dos1541", "dos1570",    "dos1581",  "dos2000",
    "dos4000",          "dos2031", "dos2040",    "dos3040",  "dos4040",
    "dos1001",
};

int dflt_bootStatSize[] = {8192, 8192, 4096, 16384, 0, 0, 0, 0, 0, 0, 0,
                           0,    0,    0,    0,     0, 0, 0, 0, 0, 0};

void ViceStdioApp::InitBootStat() {
  FILE *fp = fopen("/bootstat.txt", "r");
  if (fp == NULL) {
    printf("Could not find bootstat. Using default list.\n");

    CGlueStdioInitBootStat(dflt_bootStatNum, dflt_bootStatWhat,
                           dflt_bootStatFile, dflt_bootStatSize);

    return;
  }

  char line[80];
  int num = 0;
  while (fgets(line, 79, fp)) {
    if (feof(fp))
      break;
    if (strlen(line) == 0)
      continue;
    if (line[0] == '#')
      continue;
    char *what = strtok(line, ",");
    if (what == NULL)
      continue;
    char *file = strtok(NULL, ",");
    if (file == NULL)
      continue;
    char *size = strtok(NULL, ",");
    if (size == NULL)
      continue;
    if (size[strlen(size) - 1] == '\n') {
      size[strlen(size) - 1] = '\0';
    }

    if (num >= MAX_BOOTSTAT_LINES) {
      printf("Warning: bootstat.txt too long, max %d entries\n",
             MAX_BOOTSTAT_LINES);
      break;
    }

    if (strcmp(what, "stat") == 0) {
      mBootStatWhat[num] = BOOTSTAT_WHAT_STAT;
    } else if (strcmp(what, "fail") == 0) {
      mBootStatWhat[num] = BOOTSTAT_WHAT_FAIL;
    } else {
      printf("Ignoring unknown bootstat.txt '%s'\n", what);
      continue;
    }

    // These never get freed...
    mBootStatFile[num] = (char *)malloc(MAX_BOOTSTAT_FLEN);
    strncpy(mBootStatFile[num], file, MAX_BOOTSTAT_FLEN);
    mBootStatSize[num] = atoi(size);

    num++;
  }

  fclose(fp);

  CGlueStdioInitBootStat(num, mBootStatWhat, (const char **)mBootStatFile,
                         mBootStatSize);
}

void ViceStdioApp::DisableBootStat() {
  CGlueStdioInitBootStat(0, nullptr, nullptr, nullptr);
}
