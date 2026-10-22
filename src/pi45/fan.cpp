//
// fan.cpp
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
// See fan.h.
//
// Sources:
//  - Circle, include/circle/pwmoutput-rp1.h and lib/pwmoutput-rp1.cpp (RP1
//    PWM, PWM_CHANNEL1 to 4), lib/gpioclock-rp1.cpp (PWM1 clocked from the
//    50 MHz crystal), lib/gpiopin2712.cpp (pins 34 to 53 are RP1 bank 2)
//    and include/circle/bcmpropertytags.h (PROPTAG_GET_TEMPERATURE, in
//    thousandths of a degree).
//  - Raspberry Pi 5 device tree (facts only): the fan is on RP1 PWM1's
//    fourth channel, GPIO45 in function 0, inverted, with a 41566 ns
//    period; its cooling levels are duties 75, 125, 175 and 250 of 255 from
//    50, 60, 67.5 and 75 degrees, each held until 5 degrees below.
//

#include "fan.h"

#if RASPPI >= 5

#include <circle/bcmpropertytags.h>
#include <circle/gpiopin.h>
#include <circle/logger.h>
#include <circle/pwmoutput.h>
#include <circle/timer.h>

static const char FromFan[] = "fan";

// 41566 ns at 50 MHz, about 24 kHz.
static const unsigned ClockHz = 50000000;
static const unsigned Range = 2078;

static const unsigned Levels = 5;
static const unsigned Duty[Levels] = {0, 75, 125, 175, 250};
static const unsigned OnAt[Levels] = {0, 50000, 60000, 67500, 75000};
static const unsigned Hysteresis = 5000;

static bool s_tried;
static CPWMOutput *s_pwm;
static unsigned s_level = Levels;
static unsigned s_last_check;

// The SoC's temperature in thousandths of a degree, 0 if it can't be read.
static unsigned Temperature(void) {
  CBcmPropertyTags tags;
  TPropertyTagTemperature tag;
  tag.nTemperatureId = TEMPERATURE_ID;
  if (!tags.GetTag(PROPTAG_GET_TEMPERATURE, &tag, sizeof tag, sizeof(u32))) {
    return 0;
  }
  return tag.nValue;
}

static void SetLevel(unsigned level, unsigned mc) {
  if (level == s_level) {
    return;
  }
  s_level = level;
  s_pwm->Write(PWM_CHANNEL4, Duty[level] * Range / 255);
  CLogger::Get()->Write(FromFan, LogNotice, "%u.%u C, speed %u of %u",
                        mc / 1000, mc % 1000 / 100, level, Levels - 1);
}

// Up to the highest level reached; down one level once below its own
// level's start less the hysteresis.
static unsigned NextLevel(unsigned level, unsigned mc) {
  unsigned up = 0;
  for (unsigned i = 1; i < Levels; i++) {
    if (mc >= OnAt[i]) {
      up = i;
    }
  }
  if (up > level) {
    return up;
  }
  if (level > 0 && mc + Hysteresis < OnAt[level]) {
    return level - 1;
  }
  return level;
}

void Pi45FanPoll(void) {
  if (!s_tried) {
    s_tried = true;
    new CGPIOPin(45, GPIOModeAlternateFunction0);
    CPWMOutput *pwm = new CPWMOutput(ClockHz, Range, TRUE, TRUE, 1);
    if (!pwm->Start()) {
      CLogger::Get()->Write(FromFan, LogWarning, "The PWM did not start");
      delete pwm;
      return;
    }
    s_pwm = pwm;
    s_last_check = CTimer::GetClockTicks();
    // Full speed until the first reading.
    SetLevel(Levels - 1, Temperature());
    return;
  }
  if (s_pwm == 0) {
    return;
  }
  unsigned now = CTimer::GetClockTicks();
  if (now - s_last_check < CLOCKHZ) {
    return;
  }
  s_last_check = now;
  unsigned mc = Temperature();
  if (mc == 0) {
    SetLevel(Levels - 1, 0);
    return;
  }
  SetLevel(NextLevel(s_level, mc), mc);
}

#endif
