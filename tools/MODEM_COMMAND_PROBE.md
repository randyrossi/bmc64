# BMC modem command probe

This probe complements `modem_transport_probe.py`. It cannot inject serial
bytes into BMC64, so use CCGMS to issue commands and use the probe to verify
the TCP effects of dial, escape, hangup, reset, and redial.

The supplied `nhd.slde.zi` driver sends direct modem commands: it uses
`ATW"ssid,password"` to record Wi-Fi settings, `ATI3` to query their status,
and `ATD"host:port"` to open the CNP TCP connection. BMC64 supports that
direct-dial form.

CNP uses binary TCP on `services.c64os.com:6400`. BMC64 therefore passes every
byte on port `6400` through unchanged, including `0xff`; it does not apply its
Telnet negotiation filter to this port. Other TCP ports retain Telnet handling
for traditional BBS sessions.

The `nhd.slde.zi` driver initializes the modem with
`ATE0V0X0F0R0&P0B<baud>`. `V0` requires numeric responses and `R0` requires
carriage-return-only responses. BMC64 supports both, and accepts `&P0` as a
no-op because it already transports C64 bytes directly. The driver may also
issue `ATW"ssid,password"`, `ATI3`, bare `ATC`, and `ATF3` before connecting.
BMC64 records the SSID only so that `ATI3` reports a joined network; it does
not change the Raspberry Pi's Wi-Fi configuration.

On a development computer connected to the same LAN as BMC64, run:

```sh
hostname -I
python3 tools/modem_command_probe.py --host <development-machine-lan-ip>
```

In CCGMS, first verify the local command parser. Each command must return the
shown response. `ATQ1` intentionally has no response; follow it with `ATQ0`.

```text
AT                 OK
ATE0V1Q0X1F0R0B2400  OK
ATI                BMC64 ZIMODEM COMPATIBLE, then OK
ATQ1               no response
ATQ0               OK
```

Dial the command probe:

```text
ATD<development-machine-lan-ip>:6503
```

CCGMS must report `CONNECT` and display `BMC64 command probe connection 1`.
Type one printable character; the probe must log it. After one second of idle
time, type `+++`, wait one second, and check for `OK`. Enter `ATO`; CCGMS must
report `CONNECT` again without a new probe connection. Type another printable
character and confirm it is logged on connection 1.

Enter `+++`, wait one second, and then enter `ATH`. The probe must log
`connection 1 closed by modem`. Dial the same address again, then enter `ATZ`;
the probe must log connection 2 opening and closing and then report `command
probe complete`.

## C64 OS driver and carrier test

Use this after booting an image that includes the BMC modem changes. It checks
the C64 OS SwiftLink driver, its TCP dial request, carrier status, and DTR
hangup. It does not authenticate to CNP; the command probe is only a local TCP
endpoint.

1. On a development computer connected to the same LAN as BMC64, find its LAN
	address and start the probe:

	```sh
	cd <bmc64-checkout>
	hostname -I
	python3 tools/modem_command_probe.py --host <development-machine-lan-ip>
	```

2. In C64 OS, open `Settings`, then `Network`.
3. On the `Drvr` tab select `slde.zi`. BMC64 exposes a SwiftLink at `$DE00`.
4. Set both `Ini.Baud` and `Max.Baud` to `2400` for this first test. This keeps
	baud negotiation out of the initial diagnosis.
5. Click `Save`, then click `Test`. The test result must be `Pass` before
	proceeding. A failure here means C64 OS cannot initialize or communicate
	with the modem; it is not a TCP or carrier problem yet.
6. Open the `WiFi` tab. Enter any non-empty SSID and password, click `Save`,
	then click `Join`. BMC64 retains the SSID only; its host network remains
	unchanged. The status must indicate that the modem is connected before
	continuing.
7. Open the `CNP` tab and enter the development computer's LAN IP address as
	the host and `6503` as the port. Fill in any non-empty temporary username
	and password. Save those settings.
8. Click `Start`.

The probe must print a line like:

```text
connection 1 opened from <bmc64-lan-ip>:<ephemeral-port>
```

The probe will then send its banner. A CNP login error or an immediate C64 OS
disconnect is expected, because the probe is not a CNP server. The TCP
connection opening is the pass condition for this step.

9. Click `Stop` in C64 OS. The probe must print:

```text
connection 1 closed by modem
```

`Start` tests that C64 OS can request a TCP connection and observe carrier.
`Stop` tests DTR: C64 OS lowers DTR and BmcModem must close the TCP connection.
C64 OS does not normally show raw DSR or DCD bits, so these open/close events
are the black-box test for both signals.

## Real CNP transfer test

After the local probe passes, set both `Ini.Baud` and `Max.Baud` to `38400`.
This rate has been validated with the C64 OS CNP service. Connect to
`services.c64os.com` on port `6400`, run a Wikipedia search, and open several
content links. Each completed response must display rather than remain at
`100% downloaded`.

When the CNP service closes its socket, BMC64 reports `NO CARRIER` and the C64
OS Network application must leave its active yellow state. The same state
transition must occur after clicking `Stop`; this verifies that both a remote
carrier drop and a local DTR hangup reach the SwiftLink NMI handler.

### If Start opens no connection

Do not continue to CNP credentials or applications. Record these four items:

1. The exact `Test` result on the `Drvr` tab.
2. The exact C64 OS status shown after clicking `Start`.
3. The complete output of `modem_command_probe.py`.
4. BMC64's `bmc-modem` log lines from the moment `Start` is clicked through the
	next 10 seconds. In particular retain the full line beginning `received
	modem command:` plus any `dialing`, `dial failed`, and `TCP connection
	established` lines.

If the log has a command but no `dialing` line, the C64 OS ZiModem driver is
using a command not implemented by BmcModem. If it logs `dialing` but the probe
does not accept a connection, check that the CNP host is the development
computer's LAN address, port is `6503`, and no firewall is blocking TCP 6503.

### Capture the C64 OS serial trace

If BMC64 console logs are unavailable, BmcModem retains the last 128 bytes
received from the C64. After clicking `Start`, launch CCGMS and enter:

```text
AT+TRACE
```

It returns those bytes as hexadecimal, oldest first. Copy the entire response.
For example, `41 54 45 30 0D` is `ATE0` followed by carriage return. This trace
survives opening CCGMS, but is cleared by rebooting BMC64.

To isolate a C64 OS action, first enter `AT+TRACECLEAR` in CCGMS and wait for
`OK`. Return to C64 OS, perform only the action under test, then reopen CCGMS
and enter `AT+TRACE`. The trace excludes the clear command; ignore the final
`AT+TRACE` bytes when decoding the response.

If `AT+TRACE` is empty after a C64 OS action, repeat the same procedure using
`AT+ACIATRACECLEAR` and `AT+ACIATRACE`. This second trace records every write to
the SwiftLink `$DE00` data register before VICE's transmit alarm forwards it to
BmcModem. If it contains C64 OS bytes while `AT+TRACE` does not, the ACIA
transmit path is dropping them; if both are empty, C64 OS did not call the
driver's transmit routine.