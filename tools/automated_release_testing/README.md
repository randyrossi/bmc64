# Automated release testing

Linux-PC-controlled tests for BMC64 running on real Raspberry Pi hardware,
using the existing HTTP API, a UART-controlled Pico USB keyboard fixture,
and HDMI video/audio capture.

## Tests available

Every test has a name. `run_tests` runs all of them; `run_tests --list` lists
them and `run_tests TEST...` runs only the named tests. Folder names select
suites: `run_tests boot` runs the six boot/typing tests, and `run_tests basic`
runs the C64 diagnostic. Existing individual names are unchanged.

**Profile boot/typing tests** (`c64-boot-typing` ... `pet-boot-typing`):

- Rotates through C64, C128, VIC-20, Plus/4 VICE,
  Plus4Emu and PET profiles.
- Verifies the API profile ID and exact core after switching.
- Recognises a BASIC banner and READY prompt in captured video.
- Types a unique eight-letter token through USB, without pressing Return,
  and verifies it appears in captured video.
- Records each case separately as PASS, FAIL or NOT_RUN. Continues after
  failures only while HTTP control remains available.

Ready-to-copy [default profiles](sdcard_profiles/profiles) enable Wi-Fi
and Web UI, reusing the SD card's existing WPA configuration.
See the [profile suite guide](docs/profile-suite.md) to install them and
run it. Boot screens and typed text are checked cell by cell against each
core's own character ROM, read from the SD card at run time; no OCR is used.
The HDMI capture is reopened for each case at that core's resolution.
All six cores passed on hardware on BMC64 5.4.5 with an MS2109 capture, on a
Raspberry Pi 3 Model B and a Raspberry Pi 4 Model B, PET as a 40-column 3016.

**C64 diagnostic** (`c64-default`):

- Upload and autostart of a generated C64 diagnostic PRG.
- A unique visual run ID and an advancing guest counter.
- A SID tone in captured HDMI audio.
- A real USB Space keypress producing a visible guest response.
- Silence after the guest receives Space.
- Successful recording finalization and input cleanup.

Each hardware run saves a JSON result, recording, screenshots, API snapshots,
Pico exchanges and BMC64 UART logs with conservative error
classification. A failed capture saves the last frame even if no guest was
recognised; if no frames arrived, the evidence explicitly says so.
An HTTP success or USB acknowledgement alone cannot pass a test.

Hardware-validated C64 diagnostic configuration: Raspberry Pi 3 Model B and
Raspberry Pi 4 Model B, BMC64 5.4.5, C64 PAL,
RP2040 Pico and MacroSilicon MS2109 HDMI capture. Other configurations are
not covered by that validation.

## Requirements

- Linux PC with Python 3.10+ and FFmpeg/FFplay with V4L2/ALSA input.
  Python uses only its standard library; the C64 diagnostic's guest program
  is assembled in Python, so no VICE tools are needed.
- BMC64 with its Web UI enabled, and prepared profiles for the suite.
- Raspberry Pi Pico (RP2040), connected to BMC64 as a USB HID device, with a
  separate PC-side 3.3 V UART adapter.
- HDMI capture providing video and audio.
- A second UART adapter on the BMC64 UART. Hardware runs require it: it gives
  the Pi's address and the diagnostic logs.

New PC: start with [prerequisites and installation](docs/prerequisites.md).
No Pico SDK is needed to use the supplied fixture firmware.

## Quick start

1. Follow the [hardware setup](docs/hardware.md).
2. Flash the [supplied Pico firmware](mock_hid/bin/mock_hid.uf2), following
   the [fixture instructions](mock_hid/README.md).
3. Install the six supplied [test profiles](docs/profile-suite.md) on the SD card.
4. Close other capture viewers. From the repository root, configure your rig:

   ```sh
   ./tools/automated_release_testing/configure_test_setup
   ```

   The wizard selects the HDMI device and opens the preview first. Power on the
   Pi, wait for its display and enter the current IP shown. Choose Wi-Fi or
   Ethernet, then supply the UART paths, audio device and other rig settings.
   Ethernet requires an onboard port and a connected cable, not a USB adapter.

   After confirmation, it updates and verifies `network_device` in all six
   installed test profiles, preserving other settings. It restarts into C64
   while keeping the preview open. **Enter the IP shown AFTER reboot**, which
   may differ from the original Wi-Fi address. Setup verifies C64 and the
   selected network before saving ignored `test_setup.json`. Press Enter at
   the final prompt to close the preview; keep it open until then.

   Save guest work before confirming the update: setup now controls and
   restarts BMC64. UARTs are listed but are not opened. Existing Wi-Fi
   credentials and Web UI PINs are not modified; a required PIN is prompted
   without saving it.

5. Save guest work and close capture viewers and serial terminals. Run:

   ```sh
   ./tools/automated_release_testing/run_tests
   ```

For subsequent tests, use `./tools/automated_release_testing/run_video_test`
if you need a fresh video check, close it, then run `run_tests`.
You don't need to update the IP address. BMC64 logs it on the UART at every
boot and the run follows it. If the saved address doesn't answer, for example
after swapping Pis, the run asks you to reboot the Pi, reads the new address and
saves it to `test_setup.json`.
Run the configurator again when the network or rig changes; existing
answers become its defaults. If the Web UI requires a PIN, the test prompts
for it each time rather than storing it.

To see the test names, or run just some of them:

```sh
./run_tests --list
./run_tests boot
./run_tests basic
./run_tests pet-boot-typing
./run_tests c64-boot-typing c64-default
```

The run ends with a summary: one line per test (PASS, FAIL with its stage and
reason, or NOT_RUN), the totals, the total run time and the run's evidence folder.

The terminal reports preflight, fixture/capture startup, each profile switch,
boot recognition, the token being typed, verification and cleanup. Long waits
print an update about every five seconds with elapsed time; suite screen
checks name the cells that do not yet match the expected text. Output is
flushed immediately, including when redirected to a log.

**`c64-default` executes the C64 hardware diagnostic:** if the C64 test
profile from `suite_profiles.json` is not already running, it starts it
(Start once, restarting BMC64). It then uploads and autostarts the PRG,
resetting the emulated C64 and discarding its current unsaved state. It also
opens capture devices and sends USB input. It does not flash firmware or
install updates.

**The boot/typing tests restart BMC64 through the selected profiles**, losing
unsaved guest state. It does not install firmware or create profiles. Each
profile uses Start once, or reboots in place if it is already active,
leaving the normal power-on profile unchanged.
It leaves the last attempted profile active; it does not automatically
restore the starting machine or power-cycle a failed target.

For offline diagnostic generation without any hardware access:

```sh
./tools/automated_release_testing/run_smoke
```

That reports `PREPARED`, not a hardware PASS. Direct command-line use of
`run_smoke` is the direct CLI; hardware access requires `--execute`.
The root contains only small launchers; their Python implementations live in
`utils/`. All launchers work from any working directory.

See the [smoke-test guide](docs/smoke-test.md) for options, assertions,
troubleshooting, evidence and cleanup. The diagnostic remains running on the
target and its uniquely named PRG remains on the SD card after the test.

## Offline tests

From the repository root:

```sh
python3 -m unittest discover \
  -s tools/automated_release_testing/unit_tests \
  -t tools/automated_release_testing -p 'test_*.py' -v
make -C tools/automated_release_testing/mock_hid test all
```

These use synthetic video/audio, screens drawn with a synthetic font, a local
HTTP server and simulated UART devices; they do not contact BMC64 or exercise
the Pico. Tests against recorded frames also need the real character ROMs,
which are not in this repository: set `BMC64_ROM_ROOT` to a copy of the SD
card, otherwise they are skipped.

```sh
BMC64_ROM_ROOT=/path/to/sdcard-copy python3 -m unittest discover \
  -s tools/automated_release_testing/unit_tests \
  -t tools/automated_release_testing -p 'test_*.py'
```

To also run the C64 diagnostic's guest program in host VICE (`x64`) and
decode its screen, set `BMC64_VICE_DIR` to a VICE data directory that
contains the C64 ROMs; otherwise that test is skipped.

Hardware tests live in `tests/<suite>/test_*.py`. Offline regression tests
live separately in `unit_tests/`; the hardware runner never discovers them.
See [writing and organising tests](docs/test-framework.md) for the lifecycle,
suite base classes and cleanup contract.

## Files and generated outputs

| Location | Contents |
| --- | --- |
| `run_tests` | Execute tests with the saved rig setup |
| `run_smoke` | Direct CLI; offline preparation unless `--execute` is supplied |
| `configure_test_setup` | Interactive local rig configuration |
| `run_video_test` | Manual FFplay video check using saved settings |
| `utils/` | Python implementations of the CLI, setup wizard and video preview |
| `framework/` | Discovery, before/run/after lifecycle, cleanup and reporting |
| `hardware/` | HTTP client, UART/Pico access and FFmpeg capture |
| `support/` | Cross-suite profiles, keyboard operations, evidence and constants |
| `tests/boot/` | Six explicit test flows with local machine expectations; shared boot operations and ROM screen reader |
| `tests/basic/` | Full C64 diagnostic test, its 6502 guest program and assembler; signal decoding/audio helpers |
| `suite_profiles.json` | Machine-to-profile mapping; not a test registration file |
| `sdcard_profiles/profiles/` | Six ready-to-copy default PAL HDMI profiles with Wi-Fi/Web UI enabled |
| `unit_tests/` | Offline framework, screen, transport and diagnostic regression tests |
| `test_data/screens/` | Recorded boot, typed, corrupt and downscaled-PET frames from a Pi 3, and `*-pi4*` frames from a Pi 4 |
| `docs/` | Hardware setup and operating instructions |
| `mock_hid/` | Pico firmware, native client, protocol and tests |
| `mock_hid/bin/` | Supplied UF2 and build/checksum information |
| `results/` | Ignored per-run evidence, generated PRGs and recordings |
| `mock_hid/build/`, `mock_hid/build-native/` | Ignored build products |
| `local/` | Ignored operator-specific notes and configuration details |
| `test_setup.json` | Ignored rig settings beside the launchers |

## Limitations

These are basic boot/input checks and a C64 keyboard/video/audio diagnostic,
not full release certification.
It does not install candidates, certify frame timing or performance, test
disk persistence, or provide unattended power/boot recovery. JUnit export
is not provided. UART classification flags known fatal signatures and retains
other errors as warnings; it cannot detect every possible firmware failure.
Screen checks fail closed if any checked cell differs from the expected text;
actual screen layouts, ROMs and capture settings need hardware qualification.

The fixture also exposes a mouse and gamepad, but the automated test only
uses its keyboard. It is generic HID, not a reproduction of a particular
keyboard/controller; the separate [Keyrah fixture](../keyrah_test/README.md)
serves that specialist use case.
