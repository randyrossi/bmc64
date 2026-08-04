//
// viceoptions.h
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

#ifndef _vice_options_h
#define _vice_options_h

#include "vicesoundbasedevice.h"
#include <circle/bcmpropertytags.h>
#include <circle/cputhrottle.h>

#define VOLUME_NAME_LEN 16

class ViceOptions {
public:
  ViceOptions(void);
  ~ViceOptions(void);

  unsigned GetMachineTiming(void) const;
  bool DemoEnabled(void) const;
  bool SerialEnabled(void) const;
  bool GPIOOutputsEnabled(void) const;
  int GetDiskPartition(void) const;
  const char *GetDiskVolume(void) const;
  unsigned long GetCyclesPerSecond(void) const;
  TVCHIQSoundDestination GetAudioOut(void) const;
  bool DPIEnabled(void) const;
  void GetScalingParams(int display, int *fbw, int *fbh, int *sx, int *sy) const;
  bool GetRasterSkip(void) const;
  bool GetRasterSkip2(void) const;

  static ViceOptions *Get(void);

private:
  char *
  GetToken(void); // returns next "option=value" pair, 0 if nothing follows

  static char *GetOptionValue(
      char *pOption); // returns value and terminates option with '\0'

  static unsigned
  GetDecimal(char *pString); // returns decimal value, -1 on error

private:
  TPropertyTagCommandLine m_TagCommandLine;
  char *m_pOptions;

  unsigned m_nMachineTiming;
  bool m_bDemoEnabled;
  bool m_bSerialEnabled;
  bool m_bGPIOOutputsEnabled;
  int m_disk_partition;
  char m_disk_volume[VOLUME_NAME_LEN];
  unsigned long m_nCyclesPerSecond;
  TVCHIQSoundDestination m_audioOut;
  bool m_bDPIEnabled;
  int m_scaling_param_fbw[2];
  int m_scaling_param_fbh[2];
  int m_scaling_param_sx[2];
  int m_scaling_param_sy[2];
  bool m_raster_skip;
  bool m_raster_skip2; // for VDC

  static ViceOptions *s_pThis;
};

#endif
