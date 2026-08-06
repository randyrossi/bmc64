# BMC64 Networking

This document describes the networking support currently implemented in BMC64.
It applies to the C64 build and to the C128 build, although the C128 path is
marked as untested in the changelog.

## What It Does

BMC64 runs without Linux. The Raspberry Pi networking stack is initialized at
startup and is used by a software modem connected to VICE's ACIA1 interface.
The emulated C64 program sees a SwiftLink/Turbo232-compatible ACIA at `$DE00`.
The BMC modem translates modem commands into outbound TCP connections.

Current behavior:

- Networking is available from the `Network` menu on C64 and C128.
- The choices are `Off`, `Ethernet`, and `WiFi`.
- Ethernet uses the Raspberry Pi's onboard Ethernet controller when present.
- Wi-Fi uses the Raspberry Pi's onboard WLAN controller when present.
- The modem resolves DNS names and opens TCP connections.
- The default TCP port is `23`; a different port can be supplied in the dial
  string.
- Basic Telnet negotiation is handled. Telnet control bytes are kept out of the
  data delivered to the C64 terminal.
- The connection is outbound only. BMC64 does not listen for incoming modem
  calls or provide a BBS server.

This is Telnet-style, unencrypted TCP. It does not provide TLS, SSH, VPN, or
HTTP client behavior. 

**DO NOT** use it for credentials that must be protected.

## Enable Ethernet

1. Connect the Raspberry Pi to the network with Ethernet.
2. Boot BMC64 and open `Network`.
3. Set `Network Device` to `Ethernet`.
4. Save the setting and reboot when prompted.
5. Open `Network` again and check `IP Address`.

The Ethernet choice is disabled when the selected Raspberry Pi model does not
have onboard Ethernet. The IP address is populated when the Network menu is
opened and the network stack has a running address.

## Enable Wi-Fi

Wi-Fi requires a Raspberry Pi model with onboard WLAN and the required Wi-Fi
firmware on the SD card.

### Configure from the menu

1. Open `Network` and set `Network Device` to `WiFi`.
2. Accept the reboot prompt, or save the settings and reboot.
3. After reboot, open `Network -> WiFi Settings`.
4. Use the Wi-Fi scan to select an access point when scanning is available.
5. Select `WPA-PSK` or `None` as appropriate.
6. Set the two-letter Wi-Fi country code.
7. Choose `Enter Password & Reboot`, enter the WPA password, and select
   `Save & Reboot`.
8. After reboot, open `Network` and verify the assigned `IP Address`.

Selecting Wi-Fi makes the Wi-Fi stack available at boot. It may take a 5-20 seconds after boot to connect to an access point depending if it needs to retry.

### Configure the file manually

The configuration file is stored at the root of the SD-card volume as
`wpa_supplicant.conf`. The repository includes an example at
`sdcard/wpa_supplicant.conf.example`:

```conf
country=US

network={
    ssid="replace-with-network-name"
    psk="replace-with-network-password"
    key_mgmt=WPA-PSK
}
```

For an open network, use:

```conf
country=US

network={
    ssid="replace-with-network-name"
    key_mgmt=NONE
}
```

After changing the file, reboot BMC64. You will need to select Wifi as the Network Device (and then reboot when prompted) if Wifi was not already selected.

## BMC Modem Commands

The modem is attached to ACIA1. Commands are terminated with Return. The
implemented command set is:

| Command | Result |
| --- | --- |
| `AT` | Returns `OK`. |
| `ATI` | Returns the modem identification and `OK`. |
| `ATE0` / `ATE1` | Disable or enable command echo. |
| `ATZ` | Reset the modem state. |
| `ATD...` | Resolve a host and open a TCP connection. `ATDT` and `ATDP` are accepted. |
| `ATO` | Return to data mode when a socket is still connected. |
| `+++` | Leave data mode after the one-second escape guard interval. |
| `ATH` | Hang up the current connection. |

Dial targets may be hostnames or IPv4 addresses. The port is optional:

```text
ATDTbbs.example.org
ATDTbbs.example.org:2323
ATDT192.168.1.50:6502
```

A successful connection returns `CONNECT`. DNS failure, an unavailable network,
or a refused TCP connection returns `NO CARRIER`.

The modem's serial speed callback is retained for ACIA compatibility; the
network transport itself is TCP and does not depend on a physical baud rate.

## Use CCGMS Ultimate With a BBS

[CCGMS Ultimate on CSDb](https://csdb.dk/release/?id=174485) is a C64 BBS
terminal program released by Alwyz in 2019. Transfer its disk image or program
to the BMC64 SD card and load it on the C64 using the normal BMC64 disk or
Autostart workflow.

The current BMC64 changelog specifically calls out this setup:

1. Enable Ethernet or Wi-Fi and reboot BMC64.
2. Start CCGMS Ultimate.
3. In CCGMS, select the `Swift/DE` modem. `DE` is the ACIA address used by the
   BMC modem; do not select a physical user-port modem or a PC serial device.
4. Use CCGMS's autodialer/connection screen with the BBS hostname or IP address
   and TCP port.
5. If entering commands directly in the terminal, use the BMC modem syntax,
   for example `ATDTbbs.example.org:23`, followed by Return.
6. Wait for `CONNECT`, then follow the BBS login prompts.

Most traditional Internet BBSes use Telnet on port `23`, but some use another
port. Use the port published by the BBS operator. The BMC modem accepts both a
hostname and an IPv4 address, so either form can be entered in CCGMS.

For further details on CCGMS Ultimate refer to the CSDb release page.

## Testing or Debugging 

Before troubleshooting a live BBS, the repository includes a TCP transport
probe. Run it on another computer on the same LAN as BMC64:

```sh
hostname -I
python3 tools/modem_transport_probe.py --host <computer-lan-ip>
```

The probe listens on TCP port `6502`. From CCGMS, dial:

```text
ATDT<computer-lan-ip>:6502
```

The probe exercises receive and transmit data and keeps the connection open.
For a pager-style test that waits for Space, run:

```sh
python3 tools/modem_transport_probe.py \
    --host <computer-lan-ip> --require-space
```

For the fragmented receive test used during modem transport debugging, run:

```sh
python3 tools/modem_transport_probe.py \
    --host <computer-lan-ip> --fragmented-burst --tcp-nodelay
```

See [tools/TRANSPORT_PROBE.md](tools/TRANSPORT_PROBE.md) for the expected
probe output and the interpretation of `unacked` and `retrans` counters.


