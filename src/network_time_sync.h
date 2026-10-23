#ifndef NETWORK_TIME_SYNC_H
#define NETWORK_TIME_SYNC_H

class CNetSubSystem;

bool ConfigureSystemTimeZone(int offset_minutes);
void StartNetworkTimeSync(CNetSubSystem *network);
// True once the Pi's clock has been set from the network.
bool NetworkTimeSynced(void);

#endif