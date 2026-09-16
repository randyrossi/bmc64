# keyrah_test

A TinyUSB mock of the Individual Computers Keyrah V3, a USB adapter for
Commodore keyboards/joysticks. It presents the same composite USB device
shape and Shift-Lock protocol as the real hardware, so a host's USB
keyboard/gamepad handling (BMC64/Circle, or any other host) can be exercised
and controlled without needing the physical Keyrah.

Targets the Raspberry Pi Pico (RP2040) - full-speed only, matching the real
Keyrah - and TinyUSB ships built into the Pico SDK.

## What it presents on the wire

Six HID interfaces, matching the real Keyrah's descriptor shape:

| Interface | Class/Sub/Proto | Circle binds it as |
|---|---|---|
| `ITF_KBD1` | 3/1/1 (boot keyboard) | `ukbd1` |
| `ITF_KBD2` | 3/1/1 (boot keyboard) | `ukbd2` |
| `ITF_JOY1` | 3/0/0 (generic HID) | `upad1` |
| `ITF_JOY2` | 3/0/0 (generic HID) | `upad2` |
| `ITF_CONSUMER` | 3/0/0 (generic HID, Consumer Control usage) | not registered as a gamepad |
| `ITF_SYSCTRL` | 3/0/0 (generic HID, System Control usage) | not registered as a gamepad |

`ITF_CONSUMER`/`ITF_SYSCTRL` carry no axes/hats/buttons, so a host that only
registers gamepads with usable input (like Circle's
`CUSBGamePadStandardDevice`) won't register them as one - matching the real
Keyrah, where only 2 of its 4 non-keyboard interfaces are usable pads.

All four report descriptor shapes (keyboard, joystick, consumer, system
control) and `wMaxPacketSize` (48 bytes on every endpoint) are the same as
the real Keyrah V3 firmware's.

`bInterval` for the keyboard and joystick-shaped interfaces is set at build
time via `KEYRAH_KBD_POLL_INTERVAL_MS`/`KEYRAH_PAD_POLL_INTERVAL_MS` (see
`CMakeLists.txt`/`usb_descriptors.h`); both default to 1ms, matching the real
Keyrah.

Plus a CDC (serial) interface used as this mock's own test console - the
real Keyrah has no such thing, but it's a separate USB class so it doesn't
change how the 6 HID interfaces are seen.

The VID:PID is deliberately not the real Keyrah's (`0x18D8:0x0003`), so it's
easy to tell apart in `lsusb`/logs.

## Shift-Lock behaviour

- A ground-truth `shift_locked` flag stands in for the C64's mechanically
  locking Shift-Lock key.
- On a transition, sends a Caps-Lock (`0x39`) tap (make then break, never a
  held key) on `ukbd1` by default; switchable to both keyboard interfaces
  with `k`.
- Watches the LED byte the host sends back (`SET_REPORT`, bit 1 = Caps Lock).
  If it doesn't match `shift_locked` within 100ms, sends another tap, with
  no retry limit, until it matches.
- Optional contact-bounce simulation (`b`): a couple of spurious extra
  events (including brief Left-Shift blips) around the transition.

The keyboard LED response is also switchable (fast / slow / never-responds),
for testing how a host's LED-write handling behaves under different response
times.

## Prerequisites

**Toolchain** - this repo already has one, reuse it rather than installing a
separate one. `get_gnu_toolchain.sh` at the repo root fetches (if needed) and
puts BMC64's `arm-none-eabi-gcc` on `PATH` - it must be **sourced**, not
executed, since it works by exporting `PATH` into your current shell:

```bash
source ../../get_gnu_toolchain.sh   # from tools/keyrah_test; adjust the path if run from elsewhere
```

Do this in the same shell you'll run `cmake`/`make` in below - running it as
`./get_gnu_toolchain.sh` instead only changes `PATH` inside that throwaway
subshell and CMake will fail to find `arm-none-eabi-gcc` right after.

**Pico SDK:**

```bash
git clone -b master https://github.com/raspberrypi/pico-sdk.git
cd pico-sdk && git submodule update --init   # pulls in tinyusb
export PICO_SDK_PATH=$(pwd)
```

Plus `cmake` (already installed if you can build BMC64).

## Build

In the same shell as the `source` step above:

```bash
cd tools/keyrah_test
mkdir build && cd build
cmake -DPICO_BOARD=pico ..
make -j4
```

Produces `keyrah_test.uf2`. If `cmake` still can't find `arm-none-eabi-gcc`
(e.g. because the toolchain was set up in a different shell), point it there
directly instead of re-sourcing: `cmake -DPICO_BOARD=pico
-DPICO_TOOLCHAIN_PATH=$ARM_HOME/bin ..`.

## Flash

Hold the Pico's BOOTSEL button, plug it in, release - it appears as a mass
storage device. Drag `keyrah_test.uf2` onto it; it reboots running the mock.

## Wiring (optional)

Two buttons to GND, using the internal pull-ups (no resistors needed):

- GPIO 2 -> button -> GND: simulate typing the letter `a`
- GPIO 3 -> button -> GND: simulate pressing the physical Shift-Lock key

Not required - everything is also reachable from the console below.

## Using the console

The Pico's USB connector can only be enumerated by one host at a time, so
which console you use depends on where the mock's USB is plugged in:

- **Plugged into a PC**: the board enumerates a CDC serial port alongside the
  6 HID interfaces. Connect to it (`screen /dev/tty.usbmodemXXXX 115200`, or
  minicom/PuTTY on Windows).
- **Plugged into a bare-metal host with no way to open a CDC console of its
  own** (e.g. BMC64 on a Raspberry Pi): wire a USB-to-serial adapter (FTDI,
  CP2102, etc.) to the Pico's UART pins - GP0 (TX) to the adapter's RX, GP1
  (RX) to the adapter's TX, GND to GND - and plug that into your PC instead.
  Same console, same commands, independent of the USB link to the host. The
  two physical buttons (typing a key, toggling Shift-Lock) also keep working
  either way.

Either way, type single characters:

```
a       tap letter 'a' on ukbd1
w       tap letter 'a' on ukbd2
z       tap Left Shift on ukbd1
x       tap Left Shift on ukbd1 (with duplicate 0xE1 keycode, like the real Keyrah)
j       tap gamepad button 1 on upad1
l       toggle Shift-Lock (same as the physical button)
1/2/3   LED response mode: fast / slow / silent
+/-     adjust the retry interval by 10ms (default 100ms)
k       toggle Shift-Lock tap: both keyboard interfaces / ukbd1 only
b       toggle contact-bounce simulation
s       toggle the unattended soak test (auto Shift-Lock on a timer)
t       toggle turbo mode (fast typing + shifts + Shift-Lock on a timer)
y       toggle jitter mode (randomly delay USB servicing, simulating a busy real MCU)
r       toggle continuous resend mode (keep resending ukbd1's state every poll instead of only on change)
i       print status
h / ?   this help
```

`s`/`t`/`y`/`r` are stress-test modes for reproducing USB transaction errors
under sustained load rather than one-off manual key taps - they can be
combined (e.g. `t` and `y` together).

## Notes

- `LED_SILENT` blocks inside `tud_hid_set_report_cb()`, which delays the
  control transfer's STATUS stage - a reasonably faithful way to simulate an
  unresponsive device. Setting `led_silent_ms` to its max value blocks
  forever and needs a power cycle to recover; the default (5000ms) recovers
  on its own.
- The joystick/spare interfaces never send data on their own; Circle (or any
  host) still polls them at `bInterval` regardless of whether there's
  anything new to report, matching the real Keyrah's idle interfaces.
- If the UART console produces no output at all, check for a USB hub in the
  path to the PC - USB-serial adapters (FTDI/CH340-style) are commonly
  unreliable behind hubs, especially cheap bus-powered ones. Plug the adapter
  directly into a port on the PC.
