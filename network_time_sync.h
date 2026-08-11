#ifndef NETWORK_TIME_SYNC_H
#define NETWORK_TIME_SYNC_H

class CNetSubSystem;

bool ConfigureSystemTimeZone(int offset_minutes);
void StartNetworkTimeSync(CNetSubSystem *network);

#endif