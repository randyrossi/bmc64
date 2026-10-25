# GPIO Machine Signal

BMC64 can use three GPIO pins to tell the outside world **which machine is
running**. You can use this to:

- **Tell a microcontroller which machine is running**, for example a Raspberry
  Pi Pico that turns a real C64 keyboard into a USB keyboard. It can switch its
  key layout to suit the machine automatically.
- **Light an RGB LED** in a different colour for each machine, for example in
  place of the red power LED in a C64 case.

It works alongside every GPIO configuration (#1 to #5), so you can keep using
your joysticks, keyboard or buttons as you do now. It is the same on every
machine.

Pimmodore-64 uses the machine code to switch its keyboard layout to match the 
running machine. See the [Community Projects](COMMUNITY_PROJECTS.md) page for information and support.

---

## Contents

- [GPIO Machine Signal](#gpio-machine-signal)
  - [Contents](#contents)
  - [Turning it on](#turning-it-on)
  - [The pins](#the-pins)
  - [Machine code](#machine-code)
  - [Machine LED](#machine-led)
  - [When the pins change](#when-the-pins-change)
  - [Wiring a microcontroller](#wiring-a-microcontroller)
  - [Wiring an RGB LED](#wiring-an-rgb-led)
    - [Option 1: Resistors only](#option-1-resistors-only)
    - [Option 2: A KY-016 style RGB LED module](#option-2-a-ky-016-style-rgb-led-module)
    - [Boards that won't work](#boards-that-wont-work)
  - [Safety](#safety)

---

## Turning it on

Add `enable_gpio_machine_signal` to the first line of `cmdline.txt` on the SD
card, with one of these values:

| Value | What the pins do |
|---|---|
| `off` | Nothing. This is the default, also used if the option is missing or the value isn't recognised. |
| `code` | Carry a 3-bit number for the running machine, for a microcontroller. See [Machine code](#machine-code). |
| `led-high` | Light an RGB LED in a colour for each machine. A pin is driven high to turn its colour on. See [Machine LED](#machine-led). |
| `led-low` | The same, but a pin is driven low to turn its colour on. |

For example:

```
enable_gpio_machine_signal=led-high
```

`cmdline.txt` options must be on one line and separated by spaces.
`cmdline.txt` is kept when you switch machines, so the setting applies to
every machine.

It doesn't need `enable_gpio_outputs`, and it has no effect when DPI is
enabled, as DPI uses the GPIO pins.

## The pins

| GPIO | Header pin | Machine code | Machine LED |
|---|---|---|---|
| GPIO 9 | 21 | bit 0 | Red |
| GPIO 10 | 19 | bit 1 | Green |
| GPIO 11 | 23 | bit 2 | Blue |
| GND | 6, 9, 14, 20, 25, 30, 34, 39 | Ground | Ground |

None of the built-in GPIO configurations (#1 to #4) use these pins. In GPIO
Config #5 (Custom), GPIO 9 and GPIO 10 are shown as *Machine signal:
Reserved* while the option is on. Their bindings are kept, and work again
when the option is turned off.

## Machine code

With `code`, the three pins carry a fixed 3-bit number that identifies the
running machine. A pin is high (3.3V) for a 1 and low (0V) for a 0.

| Value | GPIO 11 | GPIO 10 | GPIO 9 | Meaning |
|---|---|---|---|---|
| 0 | 0 | 0 | 0 | Not used (never sent by BMC64) |
| 1 | 0 | 0 | 1 | C64 |
| 2 | 0 | 1 | 0 | C128 |
| 3 | 0 | 1 | 1 | PET with a Business keyboard |
| 4 | 1 | 0 | 0 | PET with a Graphics keyboard |
| 5 | 1 | 0 | 1 | VIC-20 |
| 6 | 1 | 1 | 0 | Plus/4 (both the VICE and Plus4Emu versions) |
| 7 | 1 | 1 | 1 | Unknown, or BMC64 has not set the pins yet |

These values are fixed so other projects can rely on them. New values will not
be added by changing existing ones.

The PET is split by keyboard type because the Business and Graphics keyboards
have very different layouts. The keyboard type follows the PET model selected
in the menu.

Value 7 is what a receiver reads while the Pi is starting up, because until
BMC64 sets up the pins they are inputs with pull-up resistors. A receiver
should treat 7 as "unknown".

## Machine LED

With `led-high` or `led-low`, each pin drives one colour of an RGB LED, and
BMC64 lights a colour for each machine:

| Machine | Colour | Red (GPIO 9) | Green (GPIO 10) | Blue (GPIO 11) |
|---|---|---|---|---|
| C64 | Red | on | off | off |
| C128 | Blue | off | off | on |
| VIC-20 | Green | off | on | off |
| Plus/4 (VICE and Plus4Emu) | Cyan | off | on | on |
| PET, Graphics keyboard | Yellow | on | on | off |
| PET, Business keyboard | Magenta | on | off | on |
| Unknown | Off | off | off | off |

White (all on) is not used. C64 is red to match the original power LED.

Which value to use depends on how the LED is wired:

- **`led-high`:** a pin is driven high to turn its colour on. Use this for a
  common-cathode LED (the shared leg goes to GND), a KY-016 style module, or
  three separate LEDs each going to GND.
- **`led-low`:** a pin is driven low to turn its colour on. Use this for a
  common-anode LED whose shared leg goes to **3.3V**.

## When the pins change

- When BMC64 starts, as soon as GPIO is set up.
- When you change the GPIO configuration.
- When you change the PET model, if that changes the keyboard type.

Switching machine in BMC64 restarts the Pi, so the pins are set again when
the new machine starts. The pins hold their value at all other times. They are
not updated by the emulation itself, so the machine signal has no effect on
emulation speed or timing.

## Wiring a microcontroller

For `code`, connect the pins directly to the inputs of a 3.3V microcontroller
(for example a Raspberry Pi Pico), and connect a GND pin on the Pi to GND on
the microcontroller. No other parts are needed.

```
Pi GPIO 9   ────────── input (bit 0)
Pi GPIO 10  ────────── input (bit 1)
Pi GPIO 11  ────────── input (bit 2)
Pi GND      ────────── GND
```

**Do not connect these pins to a 5V microcontroller input that has a pull-up
to 5V**, or to any 5V output. Raspberry Pi GPIO pins are 3.3V only.

When reading the value, it is good practice to only act on a value once it has
been the same for two reads in a row. The three pins don't change at exactly
the same moment, so a receiver may briefly see a value in between.

## Wiring an RGB LED

**Never connect an LED directly to a GPIO pin without a resistor.** Without
one, the LED draws as much current as the pin can supply, which can damage the
Pi.

There are two ways to wire the LED.

### Option 1: Resistors only

The simplest wiring. Each colour has its own resistor between the GPIO pin and
the LED.

Common-cathode LED (`led-high`):

```
GPIO 9  ──[R]──▶|── red   ┐
GPIO 10 ──[R]──▶|── green ├── common cathode ── GND
GPIO 11 ──[R]──▶|── blue  ┘
```

Common-anode LED (`led-low`):

```
         ┌── red   ──|◀──[R]── GPIO 9
3.3V ────┼── green ──|◀──[R]── GPIO 10
         └── blue  ──|◀──[R]── GPIO 11
```

Connect the common anode to **3.3V, never 5V**. With 5V, the LED would still
have voltage across it when the pin is "off", and current would flow into the
GPIO pin.

Three separate LEDs work the same way as a common-cathode LED: each LED's long
leg goes to its resistor and pin, and all the short legs go to GND.

Suggested resistors:

| Colour | Brighter | Dimmer, more balanced |
|---|---|---|
| Red | 330Ω (about 4 mA) | 4.7KΩ (well under 1 mA) |
| Green | 100Ω (about 2–3 mA) | 10KΩ (well under 1 mA) |
| Blue | 100Ω (about 1–3 mA) | 10KΩ (well under 1 mA) |

Green and blue LEDs need almost all of the Pi's 3.3V just to light, so with
this wiring they are dimmer than red, and mixed colours such as yellow and
cyan can look uneven. That is fine for an indicator.

Modern RGB LEDs are bright enough to be seen even at the very low currents of
the "dimmer" column, and the higher resistors on green and blue help balance
the colours.

### Option 2: A KY-016 style RGB LED module

These small modules have an RGB LED with the resistors already fitted, and
connect with jumper wires. Most are **common cathode**, so use `led-high`,
and connect the module's `−` pin to GND.

It works directly on the Pi pins, but the module's resistors are usually
chosen for 5V, so on 3.3V green and blue are dim.

Boards sold as "KY-016" vary. Check that yours has resistors on it before
connecting it straight to the Pi.

### Boards that won't work

- **Bidirectional logic level shifter boards** (the common BSS138 4 or 8
  channel boards). They shift signals but can't supply LED current.
- **IRF520 MOSFET modules.** They don't switch on properly from 3.3V.
- **WS2812 / NeoPixel LEDs.** They need a serial data signal, not three on/off
  pins.

## Safety

- The pins are only driven when `enable_gpio_machine_signal` is set to
  something other than `off`.
- Before turning it on, make sure **nothing else is connected to GPIO 9, 10
  or 11**, such as a button or switch to GND. If a pin is driven high while
  something shorts it to ground, the pin can be damaged.
- Raspberry Pi GPIO is 3.3V only. Never connect 5V to these pins.
- Keep the current from each pin to a few mA. Use the resistor values above.
