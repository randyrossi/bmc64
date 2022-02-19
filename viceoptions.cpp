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

#include <stdlib.h>
#include <string.h>
#include <algorithm>

#include <circle/logger.h>
#include <circle/sysconfig.h>
#include <circle/util.h>

extern "C" {
#include "third_party/common/circle.h"
}

#define INVALID_VALUE ((unsigned)-1)

ViceOptions *ViceOptions::s_pThis = 0;

ViceOptions::ViceOptions(void)
    : m_nMachineTiming(MACHINE_TIMING_PAL_HDMI),
      m_bDemoEnabled(false), m_bSerialEnabled(false),
      m_bGPIOOutputsEnabled(false), m_nCyclesPerSecond(0),
      m_audioOut(VCHIQSoundDestinationAuto), m_bDPIEnabled(false),
      m_scaling_param_fbw{0,0}, m_scaling_param_fbh{0,0},
      m_scaling_param_sx{0,0}, m_scaling_param_sy{0,0},
      m_raster_skip(false), m_raster_skip2(false) {
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

    if (!pValue) continue;

    if (strcmp(pOption, "machine_timing") == 0) {
      if (strcmp(pValue, "ntsc") == 0 || strcmp(pValue, "ntsc-hdmi") == 0) {
        m_nMachineTiming = MACHINE_TIMING_NTSC_HDMI;
      } else if (strcmp(pValue, "ntsc-dpi") == 0) {
        m_nMachineTiming = MACHINE_TIMING_NTSC_DPI;
      } else if (strcmp(pValue, "ntsc-composite") == 0) {
        m_nMachineTiming = MACHINE_TIMING_NTSC_COMPOSITE;
      } else if (strcmp(pValue, "ntsc-custom") == 0) {
        m_nMachineTiming = MACHINE_TIMING_NTSC_CUSTOM_HDMI;
      } else if (strcmp(pValue, "pal") == 0 || strcmp(pValue, "pal-hdmi") == 0) {
        m_nMachineTiming = MACHINE_TIMING_PAL_HDMI;
      } else if (strcmp(pValue, "pal-dpi") == 0) {
        m_nMachineTiming = MACHINE_TIMING_PAL_DPI;
      } else if (strcmp(pValue, "pal-composite") == 0) {
        m_nMachineTiming = MACHINE_TIMING_PAL_COMPOSITE;
      } else if (strcmp(pValue, "pal-custom") == 0) {
        m_nMachineTiming = MACHINE_TIMING_PAL_CUSTOM_HDMI;
      }
    } else if (strcmp(pOption, "enable_demo") == 0) {
      if (strcmp(pValue,"true") == 0 || strcmp(pValue, "1") == 0) {
        m_bDemoEnabled = true;
      } else {
        m_bDemoEnabled = false;
      }
    } else if (strcmp(pOption, "enable_serial") == 0) {
      if (strcmp(pValue,"true") == 0 || strcmp(pValue, "1") == 0) {
        m_bSerialEnabled = true;
      } else {
        m_bSerialEnabled = false;
      }
    } else if (strcmp(pOption, "enable_gpio_outputs") == 0) {
      // Unless this is true, OUTPUT HIGH should not be allowed on any pin.
      if (strcmp(pValue,"true") == 0 || strcmp(pValue, "1") == 0) {
        m_bGPIOOutputsEnabled = true;
      } else {
        m_bGPIOOutputsEnabled = false;
      }
    } else if (strcmp(pOption, "disk_partition") == 0) {
      m_disk_partition = atoi(pValue);
      if (m_disk_partition < 0)
        m_disk_partition = 0;
    } else if (strcmp(pOption, "cycles_per_refresh") == 0 || strcmp(pOption, "cycles_per_second") == 0) {
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
    } else if (strcmp(pOption, "enable_dpi") == 0) {
      if (strcmp(pValue, "true") == 0 || strcmp(pValue, "1") == 0) {
        m_bDPIEnabled = true;
      } else {
        m_bDPIEnabled = false;
      }
    } else if (strcmp(pOption, "scaling_params") == 0 ||
               strcmp(pOption, "scaling_params2") == 0) {
      int num = 0;
      if (strcmp(pOption, "scaling_params2") == 0) {
         num = 1;
      }
      char* fbw_s = strtok(pValue, ",");
      if (!fbw_s) continue;
      char* fbh_s = strtok(NULL, ",");
      if (!fbh_s) continue;
      char* sx_s = strtok(NULL, ",");
      if (!sx_s) continue;
      char* sy_s = strtok(NULL, ",");
      if (!sy_s) continue;

      m_scaling_param_fbw[num] = atoi(fbw_s);
      m_scaling_param_fbh[num] = atoi(fbh_s);
      m_scaling_param_sx[num] = atoi(sx_s);
      m_scaling_param_sy[num] = atoi(sy_s);
    } else if (strcmp(pOption, "raster_skip") == 0) {
      if (strcmp(pValue, "true") == 0 || strcmp(pValue, "1") == 0) {
        m_raster_skip = true;
      } else {
        m_raster_skip = false;
      }
    } else if (strcmp(pOption, "raster_skip2") == 0) {
      if (strcmp(pValue, "true") == 0 || strcmp(pValue, "1") == 0) {
        m_raster_skip2 = true;
      } else {
        m_raster_skip2 = false;
      }
    }
  }

  // When DPI is enabled, use the DPI versions of constants. Behavior
  // is identical. It's just used for display purposes.
  if (m_nMachineTiming == MACHINE_TIMING_PAL_CUSTOM_HDMI &&
      m_bDPIEnabled) {
     m_nMachineTiming = MACHINE_TIMING_PAL_CUSTOM_DPI;
  } else if (m_nMachineTiming == MACHINE_TIMING_NTSC_CUSTOM_HDMI &&
      m_bDPIEnabled) {
     m_nMachineTiming = MACHINE_TIMING_NTSC_CUSTOM_DPI;
  }

  if (m_nMachineTiming == MACHINE_TIMING_PAL_CUSTOM_HDMI &&
      m_nCyclesPerSecond == 0) {
    m_nMachineTiming = MACHINE_TIMING_PAL_HDMI;
  } else if (m_nMachineTiming == MACHINE_TIMING_NTSC_CUSTOM_HDMI &&
             m_nCyclesPerSecond == 0) {
    m_nMachineTiming = MACHINE_TIMING_NTSC_HDMI;
  } else if (m_nMachineTiming == MACHINE_TIMING_NTSC_CUSTOM_DPI &&
             m_nCyclesPerSecond == 0) {
    m_nMachineTiming = MACHINE_TIMING_NTSC_DPI;
  } else if (m_nMachineTiming == MACHINE_TIMING_NTSC_CUSTOM_DPI &&
             m_nCyclesPerSecond == 0) {
    m_nMachineTiming = MACHINE_TIMING_NTSC_DPI;
  }

  if (m_bDPIEnabled) {
     m_bSerialEnabled = false;
  }
}

ViceOptions::~ViceOptions(void) { s_pThis = 0; }

unsigned ViceOptions::GetMachineTiming(void) const { return m_nMachineTiming; }

bool ViceOptions::DemoEnabled(void) const { return m_bDemoEnabled; }

bool ViceOptions::SerialEnabled(void) const { return m_bSerialEnabled; }

bool ViceOptions::GPIOOutputsEnabled(void) const { return m_bGPIOOutputsEnabled; }

bool ViceOptions::DPIEnabled(void) const { return m_bDPIEnabled; }

int ViceOptions::GetDiskPartition(void) const { return m_disk_partition; }

void ViceOptions::GetScalingParams(int display, int *fbw, int *fbh, int *sx, int *sy) const {
  if (display >=0 && display < 2) {
     *fbw = m_scaling_param_fbw[display];
     *fbh = m_scaling_param_fbh[display];
     *sx = m_scaling_param_sx[display];
     *sy = m_scaling_param_sy[display];
  }
}

bool ViceOptions::GetRasterSkip(void) const { return m_raster_skip; }
bool ViceOptions::GetRasterSkip2(void) const { return m_raster_skip2; }

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
