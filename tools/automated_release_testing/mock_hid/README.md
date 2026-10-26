# UART-controlled mock HID

Raspberry Pi Pico (RP2040) fixture: one USB boot keyboard, one boot
mouse and one generic two-axis/eight-button gamepad. Controlled separately
through UART0, GP0 TX / GP1 RX, 115200 8N1. No USB CDC interface.

It follows the repository's `tools/keyrah_test` Pico/TinyUSB approach without
changing that fixture or BMC64. USB VID/PID `cafe:424d` is for local testing
only, not a production allocation. The fixed serial is `BMC64MOCK0001`;
multiple fixtures would need distinct serials.

## Flash supplied firmware

The ready-to-flash [bin/mock_hid.uf2](bin/mock_hid.uf2) is included so the
Pico SDK is not required just to use the fixture. See
[build information and checksum](bin/README.md).

Disconnect the Pico from BMC64, hold BOOTSEL while plugging it into the PC,
and copy the UF2 onto the `RPI-RP2` volume. Then reconnect its USB port to
BMC64. Flashing is manual; neither the build nor smoke runner flashes it.

See [wiring](../docs/hardware.md) before connecting the separate UART adapter.

## Build firmware from source

Requirements: CMake, Arm bare-metal GCC with newlib, a Pico SDK checkout
with its TinyUSB submodule initialized. Set these paths to your installed
SDK/toolchain. Newer Pico SDKs may download and build their matching
`picotool` helper when it is not already installed.

From this directory:

```sh
export PICO_SDK_PATH=/path/to/pico-sdk
export PATH=/path/to/arm-none-eabi/bin:$PATH
cmake -S . -B build -DPICO_BOARD=pico -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel 2
```

This produces `build/mock_hid.uf2`. With newer SDKs, UF2 generation may also
require a matching installed `picotool`; follow the SDK's configuration
diagnostic. Do not confuse a Pico-only build with a BMC64 firmware build.

`./build_uf2` does all of this in one step: it finds the SDK and compiler
(from the environment or an existing `build/`), builds, runs the native
tests, copies the UF2 to `bin/` and updates its checksum, size and tool
versions in `bin/README.md`. `./build_uf2 --clean` reconfigures from scratch.

To refresh the supplied firmware by hand after a source change, rebuild, run the
native tests below, copy `build/mock_hid.uf2` to `bin/mock_hid.uf2`, and update
the checksum/tool versions in `bin/README.md`. Do not copy the whole build
directory: ELF files, SDK helpers and native executables are build outputs.

## Build Linux command client and run parser tests

```sh
make test all
```

This uses the PC's C compiler, not the Pico toolchain. Binaries are under
`build-native/`. No extra host libraries are needed.

Example (replace the adapter path):

```sh
./build-native/hidctl /dev/serial/by-id/YOUR_PICO_UART 1 STATUS
./build-native/hidctl /dev/serial/by-id/YOUR_PICO_UART 2 KEY 0 4
sleep 0.1
./build-native/hidctl /dev/serial/by-id/YOUR_PICO_UART 3 KEY 0
./build-native/hidctl /dev/serial/by-id/YOUR_PICO_UART 4 RELEASE
```

Usage `4` is USB keyboard A, not ASCII and not a Commodore key number.
What the guest receives depends on BMC64's layout/mapping.

Only one process may own this UART. The client sends one request, waits up to
four seconds for its correlated response and exits nonzero on error/timeout.
It does not retry: retrying a relative mouse movement or key transition could
duplicate an action. Use a fresh positive request ID for every command and
retain IDs across client invocations. Pending startup/events go to stderr.
The client clears queued serial input when opening; it is a bring-up tool.
The [smoke runner](../docs/smoke-test.md) keeps its own persistent connection
and logs correlated requests/replies.

## Protocol version 1

ASCII, case-sensitive, decimal integers, LF termination; CRLF accepted.
Maximum line length 127 bytes excluding LF (including CR if present).
Stop-and-wait: do not pipeline requests. Partial lines are not executed.
Empty or whitespace-only lines are ignored without a reply; send one to
terminate any UART noise left in an unfinished line.

```text
<id> PING
<id> STATUS
<id> KEY <modifiers> [usage1 ... usage6]
<id> MOUSE <buttons> <dx> <dy> <wheel>
<id> PAD <x> <y> <buttons>
<id> RELEASE
```

| Field | Meaning |
| --- | --- |
| id | 1..2147483647; 0 reserved for asynchronous events/unidentified errors |
| KEY modifiers | 0..255 USB modifier bitmask (left Shift = 2) |
| KEY usages | Up to six distinct non-modifier HID usages, 4..223 |
| MOUSE buttons | 0..31 bitmask |
| dx/dy/wheel | -127..127 relative movement; wheel unavailable in boot protocol |
| PAD x/y | 0..255 absolute axes; neutral 128, left/up 0, right/down 255 |
| PAD buttons | 0..255 bitmask; bit 0 is USB button 1 |

KEY replaces the entire keyboard state; `KEY 0` releases it. Modifiers use the
modifier mask, not usages 224..231. Mouse movement is one-shot but mouse
buttons persist. PAD replaces the entire pad state. RELEASE releases all
keys/buttons and centres the pad. The mouse sends the three-byte boot or
five-byte report form according to the host-selected protocol.
In boot protocol, wheel movement and buttons 4/5 are rejected with
`ERROR mouse-boot-protocol-limit` rather than silently discarded.

Replies:

```text
12 OK
13 OK protocol=1 mounted=1 suspended=0 leds=0 lease_ms=2000 pending=0
14 ERROR syntax
15 ERROR usb-unavailable
16 ERROR report-timeout
17 ERROR release-pending
0 EVENT ready protocol=1
0 EVENT watchdog-release
0 ERROR automatic-release-timeout
```

HID commands ACK only after USB report transfer completion. That does not
prove BMC64 or the guest acted on it. Reports have a 250 ms timeout per
interface; RELEASE can take up to 750 ms. A failed/uncertain transfer schedules
a neutral release and blocks new input until that succeeds.

Every syntactically valid command renews a **two-second input lease**.
If holding input longer, send PING at least every 500 ms. With no valid
commands for two seconds, all held input is released. This is a firmware
input watchdog, not a hardware MCU watchdog. It cannot guarantee delivery
while USB is disconnected/suspended or the host is hung; neutral reports
are retried when USB becomes usable. Startup/remount also neutralises inputs.
Malformed/overlong lines never change input state or renew the lease.

For a tap, send KEY, wait at least 100 ms initially, then KEY 0. An immediate
press/release pair can disappear between emulated frames.

## Validation boundaries and hardware qualification

Native tests cover parser command shapes, IDs, ranges and malformed input,
plus host-client exchanges against a simulated serial device.
A successful cross-build is not hardware qualification. Before release use:

1. Confirm the Pico enumerates as keyboard, mouse and gamepad on the actual
   BMC64 board/profile; save the UART enumeration log.
2. Check STATUS, PING, invalid commands and no-USB errors.
3. At the BASIC prompt, press/release A and Shift+A; confirm actual output.
4. Hold Space (USB usage 44), stop commands, and confirm repeat stops after
   roughly two seconds. A does not normally repeat at the C64 BASIC prompt.
5. Check all gamepad axes/buttons with the selected guest port mapping.
6. Check mouse movement/buttons using the configured emulated mouse mode.
7. Reboot/reconnect BMC64 and ensure no input remains stuck.
8. Confirm a wrong/suspended/unresponsive USB host never produces a false
   successful input assertion in the smoke runner.

No real hardware is flashed or exercised by `make test` or CMake.

Build validation: Pico SDK 2.3.1, Arm GCC 9.2.1, `PICO_BOARD=pico`,
Release UF2 build; native protocol and simulated-serial tests pass.
Protocol tests also pass with AddressSanitizer and UndefinedBehaviorSanitizer.
On Pi3A+ C64 PAL, enumeration of all three interfaces, keyboard delivery,
held Space and automatic release are verified. The smoke runner also checks
a visible Space response. This does not qualify modifiers, mouse/gamepad
routing, reconnect handling or an unresponsive USB host.
