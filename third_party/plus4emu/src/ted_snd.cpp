
// plus4emu -- portable Commodore Plus/4 emulator
// Copyright (C) 2003-2008 Istvan Varga <istvanv@users.sourceforge.net>
// http://sourceforge.net/projects/plus4emu/
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

#include "plus4emu.hpp"
#include "cpu.hpp"
#include "ted.hpp"

namespace Plus4 {

  const uint8_t TED7360::soundVolumeTable[16] = {
        0,     6,    16,    26,    36,    46,    56,    66,
       75,    75,    75,    75,    75,    75,    75,    75
  };

  const uint8_t TED7360::soundMixTable[128] = {
    0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  1, 1, 1, 1,
    0, 0, 1, 1,  0, 0, 1, 1,  0, 0, 1, 1,  1, 1, 2, 2,
    1, 0, 1, 0,  1, 0, 1, 0,  1, 0, 1, 0,  2, 1, 2, 1,
    0, 0, 1, 1,  0, 0, 1, 1,  0, 0, 1, 1,  1, 1, 2, 2,
    0, 0, 0, 0,  0, 0, 0, 0,  1, 1, 1, 1,  1, 1, 1, 1,
    1, 1, 1, 1,  1, 1, 1, 1,  2, 2, 2, 2,  2, 2, 2, 2,
    0, 0, 0, 0,  0, 0, 0, 0,  1, 1, 1, 1,  1, 1, 1, 1,
    1, 1, 1, 1,  1, 1, 1, 1,  2, 2, 2, 2,  2, 2, 2, 2
  };

  const int16_t TED7360::soundDistortionTable[301] = {
        0,    66,   135,   205,   276,   347,   419,   491,
      563,   635,   708,   781,   854,   927,  1000,  1073,
     1146,  1219,  1293,  1366,  1440,  1513,  1587,  1660,
     1734,  1808,  1882,  1955,  2029,  2103,  2177,  2251,
     2325,  2399,  2473,  2547,  2621,  2695,  2770,  2844,
     2918,  2992,  3067,  3141,  3215,  3290,  3364,  3438,
     3513,  3587,  3662,  3737,  3811,  3886,  3961,  4035,
     4110,  4185,  4260,  4334,  4409,  4484,  4559,  4634,
     4709,  4784,  4860,  4935,  5010,  5085,  5160,  5236,
     5311,  5387,  5462,  5537,  5613,  5689,  5764,  5840,
     5916,  5991,  6067,  6143,  6219,  6295,  6371,  6447,
     6523,  6599,  6675,  6752,  6828,  6904,  6981,  7057,
     7134,  7210,  7287,  7364,  7440,  7517,  7594,  7671,
     7748,  7825,  7902,  7979,  8056,  8134,  8211,  8288,
     8366,  8443,  8521,  8599,  8676,  8754,  8832,  8910,
     8988,  9066,  9144,  9222,  9300,  9379,  9457,  9536,
     9614,  9693,  9771,  9850,  9929, 10008, 10087, 10166,
    10245, 10324, 10403, 10482, 10562, 10641, 10721, 10801,
    10880, 10960, 11040, 11120, 11200, 11280, 11360, 11440,
    11521, 11601, 11682, 11762, 11843, 11924, 12005, 12085,
    12166, 12248, 12329, 12410, 12491, 12573, 12654, 12736,
    12818, 12900, 12981, 13063, 13146, 13228, 13310, 13392,
    13475, 13557, 13640, 13723, 13805, 13888, 13971, 14055,
    14138, 14221, 14304, 14388, 14472, 14555, 14639, 14723,
    14807, 14891, 14975, 15060, 15144, 15228, 15313, 15398,
    15483, 15568, 15653, 15738, 15823, 15908, 15994, 16079,
    16165, 16251, 16337, 16423, 16509, 16595, 16681, 16768,
    16854, 16941, 17028, 17115, 17202, 17289, 17376, 17464,
    17551, 17639, 17726, 17814, 17902, 17990, 18078, 18167,
    18255, 18344, 18432, 18521, 18610, 18699, 18788, 18877,
    18967, 19056, 19146, 19236, 19326, 19416, 19506, 19596,
    19687, 19777, 19868, 19959, 20049, 20141, 20232, 20323,
    20414, 20506, 20598, 20690, 20782, 20874, 20966, 21058,
    21151, 21244, 21336, 21429, 21522, 21615, 21709, 21802,
    21896, 21990, 22084, 22178, 22272, 22366, 22461, 22555,
    22650, 22745, 22840, 22935, 23030, 23126, 23222, 23317,
    23413, 23509, 23606, 23702, 23798, 23895, 23992, 24089,
    24186, 24283, 24381, 24478, 24576
  };

  PLUS4EMU_REGPARM3 void TED7360::write_register_FF0E(
      void *userData, uint16_t addr, uint8_t value)
  {
    (void) addr;
    TED7360&  ted = *(reinterpret_cast<TED7360 *>(userData));
    ted.dataBusState = value;
    ted.tedRegisters[0x0E] = value;
    int     tmp = int(value) | (int(ted.tedRegisters[0x12] & 0x03) << 8);
    ted.soundChannel1Reload = uint16_t((((tmp + 1) ^ 0x03FF) & 0x03FF) + 1);
  }

  PLUS4EMU_REGPARM3 void TED7360::write_register_FF0F(
      void *userData, uint16_t addr, uint8_t value)
  {
    (void) addr;
    TED7360&  ted = *(reinterpret_cast<TED7360 *>(userData));
    ted.dataBusState = value;
    ted.tedRegisters[0x0F] = value;
    int     tmp = int(value) | (int(ted.tedRegisters[0x10] & 0x03) << 8);
    ted.soundChannel2Reload = uint16_t((((tmp + 1) ^ 0x03FF) & 0x03FF) + 1);
  }

  PLUS4EMU_REGPARM3 void TED7360::write_register_FF10(
      void *userData, uint16_t addr, uint8_t value)
  {
    (void) addr;
    TED7360&  ted = *(reinterpret_cast<TED7360 *>(userData));
    ted.dataBusState = value;
    ted.tedRegisters[0x10] = value;
    int     tmp = int(ted.tedRegisters[0x0F]) | (int(value & 0x03) << 8);
    ted.soundChannel2Reload = uint16_t((((tmp + 1) ^ 0x03FF) & 0x03FF) + 1);
  }

  PLUS4EMU_REGPARM3 void TED7360::write_register_FF11(
      void *userData, uint16_t addr, uint8_t value)
  {
    (void) addr;
    TED7360&  ted = *(reinterpret_cast<TED7360 *>(userData));
    ted.dataBusState = value;
    ted.soundFlags = (value >> 1) & 0x78;
    ted.soundVolume = soundVolumeTable[value & 0x0F];
    if ((value ^ ted.tedRegisters[0x11]) & 0x80) {
      if (value & 0x80) {
        ted.soundChannel1State = uint8_t(1);
        ted.soundChannel2State = uint8_t(1);
        ted.prvCycleCount = ted.cycle_count;
      }
      else {
        uint8_t nBits = (ted.prvCycleCount - ted.cycle_count) & 3;
        if (nBits > 0) {
          // shift in '1' bits at single clock frequency
          ted.soundChannel2NoiseState = (ted.soundChannel2NoiseState << nBits)
                                        | uint8_t(0xFF >> (8 - nBits));
        }
      }
    }
    ted.tedRegisters[0x11] = value;
    ted.updateSoundOutput();
  }

  void TED7360::calculateSoundOutput()
  {
    if (tedRegisters[0x11] & uint8_t(0x80)) {
      // DAC mode
      soundChannel1Cnt = soundChannel1Reload;
      soundChannel2Cnt = soundChannel2Reload;
      soundChannel1State = uint8_t(1);
      soundChannel2State = uint8_t(1);
      // shift in '1' bits at single clock frequency
      soundChannel2NoiseState = (soundChannel2NoiseState << (prvCycleCount + 1))
                                | uint8_t(0xFF >> (7 - prvCycleCount));
      prvCycleCount = 3;
      if (soundChannel1Cnt == 1) {
        if (soundChannel1Decay > 0U && soundChannel1Decay <= soundDecayCycles) {
          soundChannel1Decay--;
          soundChannel1State = uint8_t(0);
        }
        prvSoundChannel1Overflow = true;
      }
      else {
        soundChannel1Decay = soundDecayCycles;
        prvSoundChannel1Overflow = false;
      }
      if (soundChannel2Cnt == 1) {
        if (soundChannel2Decay > 0U && soundChannel2Decay <= soundDecayCycles) {
          soundChannel2Decay--;
          soundChannel2State = uint8_t(0);
        }
        prvSoundChannel2Overflow = true;
      }
      else {
        soundChannel2Decay = soundDecayCycles;
        prvSoundChannel2Overflow = false;
      }
    }
    else {
      // channel 1
      soundChannel1Cnt--;
      bool    soundChannel1Overflow = !(soundChannel1Cnt);
      if (soundChannel1Overflow) {
        soundChannel1Cnt = soundChannel1Reload;
        if (!prvSoundChannel1Overflow) {
          soundChannel1Decay = soundDecayCycles;
          soundChannel1State = (~soundChannel1State) & uint8_t(1);
          updateSoundOutput();
        }
      }
      prvSoundChannel1Overflow = soundChannel1Overflow;
      soundChannel1Decay--;
      if (!soundChannel1Decay) {
        soundChannel1State = uint8_t(1);
        updateSoundOutput();
      }
      // channel 2
      soundChannel2Cnt--;
      bool    soundChannel2Overflow = !(soundChannel2Cnt);
      if (soundChannel2Overflow) {
        soundChannel2Cnt = soundChannel2Reload;
        if (!prvSoundChannel2Overflow) {
          soundChannel2Decay = soundDecayCycles;
          soundChannel2State = (~soundChannel2State) & uint8_t(1);
          // channel 2 noise, 8 bit polycnt (10110010)
          // (note: this produces identical output to the older code, which
          // used a constant of 0xB3 and an additional XOR, but is simpler)
          uint8_t tmp = soundChannel2NoiseState & uint8_t(0xB2);
          tmp = tmp ^ (tmp >> 1);
          tmp = tmp ^ (tmp >> 2);
          tmp = tmp ^ (tmp >> 4);
          soundChannel2NoiseState <<= 1;
          soundChannel2NoiseState |= (tmp & uint8_t(1));
          updateSoundOutput();
        }
      }
      prvSoundChannel2Overflow = soundChannel2Overflow;
      soundChannel2Decay--;
      if (!soundChannel2Decay) {
        soundChannel2State = uint8_t(1);
        updateSoundOutput();
      }
    }
    // mix sound outputs
    int16_t tmp =
        soundDistortionTable[size_t(prvSoundOutput) + size_t(soundOutput)];
    prvSoundOutput = soundOutput;
    // send sound output signal (sample rate = 221 kHz)
    playSample(tmp);
  }

}       // namespace Plus4

