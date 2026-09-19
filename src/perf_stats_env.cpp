//
// perf_stats_env.cpp
//
// Kernel-side glue for the opt-in performance instrumentation
// (third_party/common/perf_stats.c). Compiled to an empty object unless
// BMC64_PERF_STATS is defined (./make_all.sh <board> --perf-stats). See
// docs/architecture/PERFORMANCE_TEST_PLAN.md.
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
