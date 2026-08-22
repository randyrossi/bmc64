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