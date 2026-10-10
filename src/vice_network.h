#ifndef _vice_network_h
#define _vice_network_h

#include <circle/machineinfo.h>

// The machine builds with networking and the BMC modem.
// The PET has networking for the web UI only; it has no modem.
#if defined(RASPI_C64) || defined(RASPI_C128) || defined(RASPI_VIC20) || \
    defined(RASPI_PLUS4) || defined(RASPI_PLUS4EMU) || defined(RASPI_PET)
#define BMC64_NETWORK 1
#else
#define BMC64_NETWORK 0
#endif

class CBcm4343Device;
class CNetSubSystem;
class ViceStdioApp;
struct wifi_access_point;

int ViceNetworkHasOnboardWifi(TMachineModel machine_model);
int ViceNetworkHasOnboardEthernet(TMachineModel machine_model);
// firmware_path is a folder on the card, ending in '/' ("/firmware/").
bool ViceNetworkHasWifiFirmware(const char *firmware_path);
unsigned int ViceNetworkCollectWifiScanResults(
    CBcm4343Device *wlan, struct wifi_access_point *access_points,
    unsigned int max_access_points, unsigned int count,
    unsigned int *result_messages);
void ViceNetworkSetSubsystem(CNetSubSystem *subsystem);
void ViceNetworkSetStdioApp(ViceStdioApp *app);
void ViceNetworkNotifyStatusChanged(void);

#endif