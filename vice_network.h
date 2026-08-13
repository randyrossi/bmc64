#ifndef _vice_network_h
#define _vice_network_h

#include <circle/machineinfo.h>

class CBcm4343Device;
class CNetSubSystem;
class ViceStdioApp;
struct wifi_access_point;

int ViceNetworkHasOnboardWifi(TMachineModel machine_model);
int ViceNetworkHasOnboardEthernet(TMachineModel machine_model);
bool ViceNetworkHasWifiFirmware(const char *firmware_path);
unsigned int ViceNetworkCollectWifiScanResults(
    CBcm4343Device *wlan, struct wifi_access_point *access_points,
    unsigned int max_access_points, unsigned int count,
    unsigned int *result_messages);
void ViceNetworkSetSubsystem(CNetSubSystem *subsystem);
void ViceNetworkSetStdioApp(ViceStdioApp *app);
void ViceNetworkNotifyStatusChanged(void);

#endif