//
// vice_network.h
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