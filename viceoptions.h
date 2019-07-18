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

#define DEFAULT_FB1_WIDTH 640
#define DEFAULT_FB1_HEIGHT 480

#define MIN_FB1_WIDTH 320
#define MIN_FB1_HEIGHT 240

#define VOLUME_NAME_LEN 16

class ViceOptions {
public:
  ViceOptions(void);
  ~ViceOptions(void);

  unsigned GetFB1Width(void) const;
  unsigned GetFB1Height(void) const;
  unsigned GetMachineTiming(void) const;
  bool GetHideConsole(void) const;
  void SetHideConsole(bool value);
  bool GetDemoMode(void) const;
  bool SerialEnabled(void) const;
  int GetDiskPartition(void) const;
  const char *GetDiskVolume(void) const;
  unsigned long GetCyclesPerSecond(void) const;
  TVCHIQSoundDestination GetAudioOut(void) const;

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

  unsigned m_nFB1Width;
  unsigned m_nFB1Height;
  unsigned m_nMachineTiming;
  bool m_bHideConsole;
  bool m_bDemoMode;
  bool m_bSerialEnabled;
  int m_disk_partition;
  char m_disk_volume[VOLUME_NAME_LEN];
  unsigned long m_nCyclesPerSecond;
  TVCHIQSoundDestination m_audioOut;

  static ViceOptions *s_pThis;
};

#endif
