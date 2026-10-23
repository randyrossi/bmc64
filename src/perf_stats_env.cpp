//
// perf_stats_env.cpp
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
// Kernel-side glue for the opt-in performance instrumentation
// (third_party/common/perf_stats.c). Compiled to an empty object unless
// BMC64_PERF_STATS is defined (./make_all.sh <board> --perf-stats).
//
// Reports the conditions that decide whether a run is comparable with another:
// SoC temperature, the firmware throttle bitmask and the live ARM clock.
//
// Licensed under the Apache License, Version 2.0.
//

extern "C" {
#include "../third_party/common/circle.h"
}

#ifdef BMC64_PERF_STATS

#include <circle/bcmpropertytags.h>
#include <circle/machineinfo.h>
#include <circle/types.h>

extern "C" {

// Mailbox property calls: call from the reporting context (core 0 / the Pi 0
// scheduler task) only, never from the emulator-core frame hooks. The web
// UI's /api/status does the same queries.
void circle_perf_get_env(int *temp_millic, unsigned *throttled,
                         unsigned *arm_hz) {
  CBcmPropertyTags tags;

  *temp_millic = 0;
  TPropertyTagTemperature temp;
  temp.nTemperatureId = TEMPERATURE_ID;
  if (tags.GetTag(PROPTAG_GET_TEMPERATURE, &temp, sizeof(temp), sizeof(u32))) {
    *temp_millic = (int)temp.nValue;
  }

  // Bits 0-3 are "now", bits 16-19 "since boot": under-voltage, ARM clock
  // capped, throttled, soft temperature limit. See webui.cpp ThrottledField().
  // Left as 0xFFFFFFFF if the query is unavailable so a run cannot be
  // mistaken for a clean one.
  *throttled = 0xFFFFFFFFu;
  TPropertyTagSimple thr;
  if (tags.GetTag(PROPTAG_GET_THROTTLED, &thr, sizeof(thr))) {
    *throttled = thr.nValue;
  }

  *arm_hz = CMachineInfo::Get()->GetClockRate(CLOCK_ID_ARM);
}

} // extern "C"

#endif /* BMC64_PERF_STATS */
