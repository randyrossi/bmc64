//
// network_time_sync.cpp
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

#include "network_time_sync.h"

#include <circle/logger.h>
#include <circle/net/dnsclient.h>
#include <circle/net/netsubsystem.h>
#include <circle/net/ntpclient.h>
#include <circle/sched/scheduler.h>
#include <circle/timer.h>

#include <reent.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

bool ConfigureSystemTimeZone(int offset_minutes) {
  int absolute_offset = offset_minutes;
  if (absolute_offset < 0) {
    absolute_offset = -absolute_offset;
  }

  char timezone[16];
  snprintf(timezone, sizeof(timezone), "UTC%c%d:%02d",
           offset_minutes > 0 ? '-' : '+', absolute_offset / 60,
           absolute_offset % 60);
  if (_setenv_r(_REENT, "TZ", timezone, 1) != 0) {
    return false;
  }
  _tzset_r(_REENT);
  return CTimer::Get()->SetTimeZone(offset_minutes);
}

namespace {

// Set once the first NTP time has been written to the Pi's clock.
volatile bool time_synced = false;

class NetworkTimeSyncTask : public CTask {
public:
  explicit NetworkTimeSyncTask(CNetSubSystem *network) : mNetwork(network) {
    SetName("ntpwait");
  }

  void Run(void) override {
    while (!mNetwork->IsRunning()) {
      CScheduler::Get()->Sleep(1);
    }

    for (;;) {
      CIPAddress server;
      CDNSClient dns(mNetwork);
      if (!dns.Resolve("pool.ntp.org", &server)) {
        CLogger::Get()->Write("ntp", LogWarning,
                              "Cannot resolve NTP server: pool.ntp.org");
        CScheduler::Get()->Sleep(300);
        continue;
      }

      CNTPClient client(mNetwork);
      unsigned timestamp = client.GetTime(server);
      if (timestamp == 0) {
        CLogger::Get()->Write("ntp", LogWarning,
                              "Cannot get time from NTP server: pool.ntp.org");
        CScheduler::Get()->Sleep(300);
        continue;
      }

      if (CTimer::Get()->SetTime(timestamp, FALSE)) {
        time_synced = true;
        CLogger::Get()->Write("ntp", LogNotice,
                              "System time set from NTP: %u UTC", timestamp);
      } else {
        CLogger::Get()->Write("ntp", LogWarning,
                              "Cannot set system time from NTP");
      }
      CScheduler::Get()->Sleep(900);
    }
  }

private:
  CNetSubSystem *mNetwork;
};

}  // namespace

void StartNetworkTimeSync(CNetSubSystem *network) {
  new NetworkTimeSyncTask(network);
}

bool NetworkTimeSynced(void) {
  return time_synced;
}