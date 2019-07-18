//
// viceoptions.cpp
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

#include "viceoptions.h"
#include <circle/logger.h>
#include <circle/sysconfig.h>
#include <circle/util.h>
#include <stdlib.h>
#include <string.h>

extern "C" {
#include "third_party/vice-3.3/src/arch/raspi/circle.h"
}

#define INVALID_VALUE ((unsigned)-1)

ViceOptions *ViceOptions::s_pThis = 0;

ViceOptions::ViceOptions(void)
    : m_nFB1Width(DEFAULT_FB1_WIDTH),
      m_nFB1Height(DEFAULT_FB1_HEIGHT),
      m_nMachineTiming(MACHINE_TIMING_PAL_HDMI), m_bHideConsole(true),
      m_bDemoMode(false), m_bSerialEnabled(false), m_nCyclesPerSecond(0),
      m_audioOut(VCHIQSoundDestinationAuto) {
  s_pThis = this;

  CBcmPropertyTags Tags;
  if (!Tags.GetTag(PROPTAG_GET_COMMAND_LINE, &m_TagCommandLine,
                   sizeof m_TagCommandLine)) {
    return;
  }

  if (m_TagCommandLine.Tag.nValueLength >= sizeof m_TagCommandLine.String) {
    return;
  }
  m_TagCommandLine.String[m_TagCommandLine.Tag.nValueLength] = '\0';

  m_pOptions = (char *)m_TagCommandLine.String;

  // Set the default volume we mount for fatfs
  m_disk_partition = 0; // this tells fatfs 'auto'
  strcpy(m_disk_volume, "SD");

  char *pOption;
  while ((pOption = GetToken()) != 0) {
    char *pValue = GetOptionValue(pOption);

    if (strcmp(pOption, "canvas_width") == 0) {
      unsigned nValue;
      if ((nValue = GetDecimal(pValue)) != INVALID_VALUE && MIN_FB1_WIDTH <= nValue &&
          nValue <= 1980) {
        m_nFB1Width = nValue;
      }
    } else if (strcmp(pOption, "canvas_height") == 0) {
      unsigned nValue;
      if ((nValue = GetDecimal(pValue)) != INVALID_VALUE && MIN_FB1_HEIGHT <= nValue &&
          nValue <= 1080) {
        m_nFB1Height = nValue;
      }
    } else if (strcmp(pOption, "machine_timing") == 0) {
      if (strcmp(pValue, "ntsc") == 0 || strcmp(pValue, "ntsc-hdmi") == 0) {
        m_nMachineTiming = MACHINE_TIMING_NTSC_HDMI;
      } else if (strcmp(pValue, "ntsc-composite") == 0) {
        m_nMachineTiming = MACHINE_TIMING_NTSC_COMPOSITE;
      } else if (strcmp(pValue, "ntsc-custom") == 0) {
        m_nMachineTiming = MACHINE_TIMING_NTSC_CUSTOM;
      } else if (strcmp(pValue, "pal") == 0 ||
                 strcmp(pValue, "pal-hdmi") == 0) {
        m_nMachineTiming = MACHINE_TIMING_PAL_HDMI;
      } else if (strcmp(pValue, "pal-composite") == 0) {
        m_nMachineTiming = MACHINE_TIMING_PAL_COMPOSITE;
      } else if (strcmp(pValue, "pal-custom") == 0) {
        m_nMachineTiming = MACHINE_TIMING_PAL_CUSTOM;
      }
    } else if (strcmp(pOption, "hide_console") == 0) {
      if (strcmp(pValue, "1") == 0) {
        m_bHideConsole = true;
      } else {
        m_bHideConsole = false;
      }
    } else if (strcmp(pOption, "demo") == 0) {
      if (strcmp(pValue, "1") == 0) {
        m_bDemoMode = true;
      } else {
        m_bDemoMode = false;
      }
    } else if (strcmp(pOption, "serial") == 0) {
      if (strcmp(pValue, "1") == 0) {
        m_bSerialEnabled = true;
      } else {
        m_bSerialEnabled = false;
      }
    } else if (strcmp(pOption, "disk_volume") == 0) {
      if (strlen(pValue) > 0) {
        strncpy(m_disk_volume, pValue, VOLUME_NAME_LEN - 1);
      }
    } else if (strcmp(pOption, "disk_partition") == 0) {
      m_disk_partition = atoi(pValue);
      if (m_disk_partition < 0)
        m_disk_partition = 0;
    } else if (strcmp(pOption, "cycles_per_refresh") == 0 || strcmp(pOption, "cycles_per_second")) {
      // This was named incorrectly in earlier versions. Keeping the old bad name working.
      m_nCyclesPerSecond = atol(pValue);
    } else if (strcmp(pOption, "audio_out") == 0) {
      if (strcmp(pValue, "hdmi") == 0 || strcmp(pValue, "ntsc-hdmi") == 0) {
        m_audioOut = VCHIQSoundDestinationHDMI;
      } else if (strcmp(pValue, "analog") == 0) {
        m_audioOut = VCHIQSoundDestinationHeadphones;
      } else if (strcmp(pValue, "auto") == 0) {
        m_audioOut = VCHIQSoundDestinationAuto;
      }
    }
  }

  if (m_nMachineTiming == MACHINE_TIMING_PAL_CUSTOM &&
      m_nCyclesPerSecond == 0) {
    m_nMachineTiming = MACHINE_TIMING_PAL_HDMI;
  } else if (m_nMachineTiming == MACHINE_TIMING_NTSC_CUSTOM &&
             m_nCyclesPerSecond == 0) {
    m_nMachineTiming = MACHINE_TIMING_NTSC_HDMI;
  }
}

ViceOptions::~ViceOptions(void) { s_pThis = 0; }

unsigned ViceOptions::GetFB1Width(void) const {
    return m_nFB1Width;
}

unsigned ViceOptions::GetFB1Height(void) const {
    return m_nFB1Height;
}

unsigned ViceOptions::GetMachineTiming(void) const { return m_nMachineTiming; }

bool ViceOptions::GetHideConsole(void) const { return m_bHideConsole; }

void ViceOptions::SetHideConsole(bool value) { m_bHideConsole = value; }

bool ViceOptions::GetDemoMode(void) const { return m_bDemoMode; }

bool ViceOptions::SerialEnabled(void) const { return m_bSerialEnabled; }

int ViceOptions::GetDiskPartition(void) const { return m_disk_partition; }

const char *ViceOptions::GetDiskVolume(void) const { return m_disk_volume; }

unsigned long ViceOptions::GetCyclesPerSecond(void) const {
  return m_nCyclesPerSecond;
}

TVCHIQSoundDestination ViceOptions::GetAudioOut(void) const {
  return m_audioOut;
}

ViceOptions *ViceOptions::Get(void) { return s_pThis; }

char *ViceOptions::GetToken(void) {
  while (*m_pOptions != '\0') {
    if (*m_pOptions != ' ') {
      break;
    }

    m_pOptions++;
  }

  if (*m_pOptions == '\0') {
    return 0;
  }

  char *pToken = m_pOptions;

  while (*m_pOptions != '\0') {
    if (*m_pOptions == ' ') {
      *m_pOptions++ = '\0';

      break;
    }

    m_pOptions++;
  }

  return pToken;
}

char *ViceOptions::GetOptionValue(char *pOption) {
  while (*pOption != '\0') {
    if (*pOption == '=') {
      break;
    }

    pOption++;
  }

  if (*pOption == '\0') {
    return 0;
  }

  *pOption++ = '\0';

  return pOption;
}

unsigned ViceOptions::GetDecimal(char *pString) {
  if (pString == 0 || *pString == '\0') {
    return INVALID_VALUE;
  }

  unsigned nResult = 0;

  char chChar;
  while ((chChar = *pString++) != '\0') {
    if (!('0' <= chChar && chChar <= '9')) {
      return INVALID_VALUE;
    }

    unsigned nPrevResult = nResult;

    nResult = nResult * 10 + (chChar - '0');
    if (nResult < nPrevResult || nResult == INVALID_VALUE) {
      return INVALID_VALUE;
    }
  }

  return nResult;
}
