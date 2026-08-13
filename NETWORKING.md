# BMC64 Networking

This document describes the networking support currently implemented in BMC64.
It applies to the C64 build and to the C128 build, although the C128 path is
marked as untested in the changelog.

## What It Does

BMC64 runs without Linux. The Raspberry Pi networking stack is initialized at
startup and is used by a software modem connected to VICE's ACIA1 interface.
The emulated C64 program sees a SwiftLink/Turbo232-compatible ACIA at `$DE00`
by default. The BMC modem translates modem commands into outbound TCP
connections.

Current behavior:

- Networking is available from the `Network` menu on C64 and C128.
- The choices are `Off`, `Ethernet`, and `WiFi`.
- `Modem Address` selects `$DE00` (default) or alternatively `$D700`, `$DF00`,
  or `$DF80`.
  - `$D700` is recommended if you are using an IDE64 and REU at the same time.
- Ethernet uses the Raspberry Pi's onboard Ethernet controller when present.
- Wi-Fi uses the Raspberry Pi's onboard WLAN controller when present.
- The modem resolves DNS names and opens TCP connections.
- The default TCP port is `23`; a different port can be supplied in the dial
  string.
- Basic Telnet negotiation is handled for BBS-style connections. Telnet
  control bytes are kept out of the data delivered to the C64 terminal.
- C64 OS CNP traffic on port `6400` is binary-transparent. Every byte,
  including `0xff`, reaches C64 OS unchanged.
- The connection is outbound only. BMC64 does not listen for incoming modem
  calls or provide a BBS server.

This is Telnet-style, unencrypted TCP. It does not provide TLS, SSH, VPN, or
HTTP client behavior. 

**DO NOT** use it for credentials that must be protected.

# Requirements

Networking requires the Raspberry Pi networking firmware to be present in the
`/firmware` directory at the root of the SD card. Follow the build instructions
in [BUILDING.md](BUILDING.md) and use `build_sdcard.sh` to generate an SD-card
image with the required firmware included.

To install the Wi-Fi firmware manually, build it from the repository root:

```sh
cd third_party/circle-stdlib/libs/circle/addon/wlan/firmware
make firmware
```

Then copy every generated file from that directory to `/firmware` on the SD
card.

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
5. Select the `Modem Address` desired.
6. Select `WPA-PSK` or `None` as appropriate.
7. Set the two-letter Wi-Fi country code.
8. Choose `Enter Password & Reboot`, enter the WPA password, and select
   `Save & Reboot`.
9. After reboot, open `Network` and verify the assigned `IP Address`.

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
| `ATI` / `ATI<n>` | Returns the modem identification and `OK`. |
| `ATI3` | Returns the SSID most recently supplied through `ATW`, then `OK`. |
| `ATE0` / `ATE1` | Disable or enable command echo. |
| `ATQ0` / `ATQ1` | Enable or suppress result codes. |
| `ATV0` / `ATV1` | Select numeric or text result codes. |
| `ATR0` / `ATR1` | Select Return-only or CR/LF result-code termination. |
| `ATF0` through `ATF3` | Accepted for ZiModem compatibility. |
| `ATB<n>` / `ATX<n>` | Accepted; serial rate is controlled by the ACIA configuration. |
| `AT&P<n>`, `AT&F<n>`, `AT&K<n>`, `AT&L<n>`, `AT&W<n>` | Accepted as ZiModem-compatible no-ops. |
| `ATW"ssid,password"` | Records an SSID for `ATI3`; it does not change the Pi's Wi-Fi configuration. |
| `ATC` | Reports whether BMC64's host network is running. |
| `ATC...` | Resolve a host and open TCP while remaining in command mode. |
| `ATZ` | Reset the modem state. |
| `ATD...` | Resolve a host and open TCP data mode. `ATDT`, `ATDP`, and quoted targets are accepted. |
| `ATO` | Return to data mode when a socket is still connected. |
| `+++` | Leave data mode after the one-second escape guard interval. |
| `ATH` / `ATH0` | Hang up the current connection. |
| `AT+TRACE`, `AT+TRACECLEAR` | Read or clear the last 128 serial-backend bytes for diagnostics. |
| `AT+ACIATRACE`, `AT+ACIATRACECLEAR` | Read or clear the last 128 SwiftLink data-register writes for diagnostics. |

Dial targets may be hostnames or IPv4 addresses. The port is optional:

```text
ATDTbbs.example.org
ATDTbbs.example.org:2323
ATDT192.168.1.50:6502
```

A successful connection returns `CONNECT`. DNS failure, an unavailable network,
or a refused TCP connection returns `NO CARRIER`.

TCP is independent of a physical serial device, but the emulated ACIA still
paces bytes at the speed selected by the terminal program or C64 OS driver.
Set the desired rate in the C64 client; for C64 OS, set both baud fields to
the same value. `38400` has been validated with CNP transfers.

## Using a BBS

[CCGMS Ultimate on CSDb](https://csdb.dk/release/?id=174485) is a C64 BBS
terminal program released by Alwyz in 2019. Transfer its disk image or program
to the BMC64 SD card and load it on the C64 using the normal BMC64 disk or
Autostart workflow.

The current BMC64 changelog specifically calls out this setup:

1. Enable Ethernet or Wi-Fi and reboot BMC64.
2. Start CCGMS Ultimate.
3. In CCGMS, select the `Swift/DE` or appropriate modem. 
   * `D7`, `DE` or `DF` should match the Modem address selected.
4. Use CCGMS's autodialer/connection screen with the BBS hostname or IP address
   and TCP port.
5. If entering commands directly in the terminal, use the BMC modem syntax,
   for example `ATDTbbs.example.org:23`, followed by Return.
6. Wait for `CONNECT`, then follow the BBS login prompts.

Most traditional Internet BBSes use Telnet on port `23`, but some use another
port. Use the port published by the BBS operator. The BMC modem accepts both a
hostname and an IPv4 address, so either form can be entered in CCGMS.

For further details on CCGMS Ultimate refer to the CSDb release page.

## Using C64 OS Networking

C64 OS provides SwiftLink drivers for the different Modem addresses that can be set in the BMC64 Networking options.

Modem address `$D700` with the `sld7.zi` driver is recommended when using a IDE64 C64 OS image with
an REU enabled, to avoid address collisions

Modem address `$DE00` with the `slde.zi` driver is recommended when using a CMD-HD C64 OS image with
an REU enabled, to avoid address collisions

The driver sends a ZiModem-compatible initialization command, uses `ATW` and `ATI3` to identify the host connection, and dials the CNP service using a quoted `ATD` target.

Networking in C64 OS uses C64 Network Protocol (CNP) and you need to connect to a CNP server and have an account on it. For full information on C64 OS Networking read the [C64 OS Networking Guide](https://c64os.com/c64os/networkingguide/).

Quick start for BMC64:

1. Configure Ethernet or Wi-Fi in BMC64 and confirm that `Network Status` shows as
  connected. 
    * Note: The C64 OS Wi-Fi fields do not configure the Raspberry Pi WiFi.
1. In C64 OS, open `Settings`, then `Network`. On the `Drvr` tab, select the
  driver matching BMC64's `Modem Address`: `sld7.zi` for `$D700`, `slde.zi`
  for `$DE00`, or `sldf.zi` for `$DF00`.
1. Set both `Ini.Baud` and `Max.Baud` to `38400`, save the settings, and run
  `Test`. It must report `Pass` before attempting CNP.
1. On the `WiFi` tab, enter non-empty values and use `Join` so C64 OS can
  complete its driver workflow. BMC64 leaves the host network unchanged.
1. On the `CNP` tab, configure `services.c64os.com` as the host and `6400` as
  the port, then supply your C64 OS service credentials and click `Start`.
1. Open the C64 OS Wikipedia application, run a search, and select several
  content links. Each item should download and display; this exercises CNP's
  binary payload path rather than just the initial connection.
1. Click `Stop` when finished. C64 OS should leave its active yellow state;
  BMC64 closes the TCP connection when the driver lowers DTR. A remote CNP
  disconnect produces the same state transition through the SwiftLink NMI.

## Testing or Debugging 

### Modem Transport Probe

Before troubleshooting a live BBS, the repository includes a TCP transport
probe. This was original used to solve TCP stalling issues.

Run it on another computer on the same LAN as BMC64:

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

### Modem Command Probe

This probe was use for testing the connection to a CNP server from within C64 OS.

Run it on another computer on the same LAN as BMC64:

For a local, credential-free driver and DTR test before using CNP, follow
[tools/MODEM_COMMAND_PROBE.md](tools/MODEM_COMMAND_PROBE.md).


