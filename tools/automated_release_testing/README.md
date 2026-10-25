# Automated release testing

Linux-PC-controlled tests for BMC64 running on real Raspberry Pi hardware,
using the existing HTTP API, a UART-controlled Pico USB keyboard fixture,
and HDMI video/audio capture.

## Tests available

Every test has a name. `run_tests` runs all of them; `run_tests --list` lists
them and `run_tests TEST...` runs only the named tests.

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
All six cores passed on hardware in run `20261025T212237Z` (BMC64 5.4.4,
Raspberry Pi 3 Model A+, MS2109 capture), PET as a 40-column 3016.

**C64 diagnostic** (`c64-default`):

- Upload and autostart of a generated C64 diagnostic PRG.
- A unique visual run ID and an advancing guest counter.
- A SID tone in captured HDMI audio.
- A real USB Space keypress producing a visible guest response.
- Silence after the guest receives Space.
- Successful recording finalization and input cleanup.

Each hardware run saves a JSON result, recording, screenshots, API snapshots,
Pico exchanges and optional BMC64 UART logs with conservative error
classification. A failed capture saves the last frame even if no guest was
recognised; if no frames arrived, the evidence explicitly says so.
An HTTP success or USB acknowledgement alone cannot pass a test.

Hardware-validated C64 diagnostic configuration: Raspberry Pi 3 Model A+,
BMC64 5.4.4, C64 PAL,
RP2040 Pico and MacroSilicon MS2109 HDMI capture. Other configurations are
not covered by that validation.

## Requirements

- Linux PC with Python 3.10+, FFmpeg/FFplay with V4L2/ALSA input,
  and VICE's `petcat` for the C64 diagnostic. Python uses only its standard
  library.
- BMC64 with its Web UI enabled, and prepared profiles for the suite.
- Raspberry Pi Pico (RP2040), connected to BMC64 as a USB HID device, with a
  separate PC-side 3.3 V UART adapter.
- HDMI capture providing video and audio.
- Optional second UART adapter for BMC64 diagnostic logs.

New PC: start with [prerequisites and installation](docs/prerequisites.md).
No Pico SDK is needed to use the supplied fixture firmware.

## Quick start

1. Follow the [hardware setup](docs/hardware.md).
2. Flash the [supplied Pico firmware](mock_hid/bin/mock_hid.uf2), following
   the [fixture instructions](mock_hid/README.md).
3. From the repository root, configure your rig once:

   ```sh
   python3 tools/automated_release_testing/configure_test_setup.py
   ```

   Answer the prompts for the BMC64 IP address, UARTs, capture devices and setup
   notes. UARTs and video devices have numbered selection menus; manual paths
   are also accepted. Enter keeps a displayed default. Configuration lists
   devices without opening them; it saves ignored `test_setup.json`
   and creates an ignored, executable `run_tests` launcher.

4. **Before running the tests, start the video preview.** Close other viewers
   and run:

   ```sh
   ./tools/automated_release_testing/run_video_test
   ```

   If the Pi is off, start the preview before powering it on manually.
   **Confirm that the BMC64 display is visible and shows its IP address.**
   Check that this matches the IP address saved during configuration.
   Do not proceed if the preview is black or the IP address is not displayed.
   Press Q to close the preview before the automated test. This only previews
   video; it does not control the Pi or produce a PASS/FAIL result.

5. Save guest work and close capture viewers and serial terminals. Run:

   ```sh
   ./tools/automated_release_testing/run_tests
   ```

For subsequent tests, repeat the video/IP check, close the preview, then run
`run_tests`. Every test needs the six test profiles installed first; see the
[profile suite guide](docs/profile-suite.md). Run the configurator again only when the rig changes; existing
answers become its defaults. If the Web UI requires a PIN, the test prompts
for it each time rather than storing it.

To see the test names, or run just some of them:

```sh
./run_tests --list
./run_tests pet-boot-typing
./run_tests c64-boot-typing c64-default
```

The run ends with a summary: one line per test (PASS, FAIL with its stage and
reason, or NOT_RUN), the totals and the evidence folders.

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
python3 tools/automated_release_testing/run_smoke.py
```

That reports `PREPARED`, not a hardware PASS. Direct command-line use of
`run_smoke.py` remains available; hardware access requires `--execute`.

See the [smoke-test guide](docs/smoke-test.md) for options, assertions,
troubleshooting, evidence and cleanup. The diagnostic remains running on the
target and its uniquely named PRG remains on the SD card after the test.

## Offline tests

From the repository root:

```sh
python3 -m unittest discover -s tools/automated_release_testing -p 'test_*.py' -v
make -C tools/automated_release_testing/mock_hid test all
```

These use synthetic video/audio, screens drawn with a synthetic font, a local
HTTP server and simulated UART devices; they do not contact BMC64 or exercise
the Pico. Tests against recorded frames also need the real character ROMs,
which are not in this repository: set `BMC64_ROM_ROOT` to a copy of the SD
card, otherwise they are skipped.

```sh
BMC64_ROM_ROOT=/path/to/sdcard-copy python3 -m unittest discover \
  -s tools/automated_release_testing -p 'test_*.py'
```

## Files and generated outputs

| Location | Contents |
| --- | --- |
| `run_smoke.py` | PC orchestration and hardware access |
| `profile_suite.py`, `suite_profiles.json` | Profile boot/typing suite and default case manifest |
| `screen_text.py` | Reads boot screens and typed text against each core's character ROM |
| `sdcard_profiles/profiles/` | Six ready-to-copy default PAL HDMI profiles with Wi-Fi/Web UI enabled |
| `harness.py` | Capture evidence and conservative UART classification |
| `configure_test_setup.py` | Interactive local setup and launcher generation |
| `run_video_test`, `preview_video.py` | Manual FFplay video check using saved settings |
| `diagnostic.py` | Guest generation, visual decoding and audio analysis |
| `test_smoke.py`, `test_suite.py` | Offline tests |
| `test_data/screens/` | Recorded 1280x720 boot, typed, corrupt and downscaled-PET frames |
| `docs/` | Hardware setup and operating instructions |
| `mock_hid/` | Pico firmware, native client, protocol and tests |
| `mock_hid/bin/` | Supplied UF2 and build/checksum information |
| `results/` | Ignored per-run evidence, generated PRGs and recordings |
| `mock_hid/build/`, `mock_hid/build-native/` | Ignored build products |
| `local/` | Ignored operator-specific notes and configuration details |
| `run_tests` | Ignored generated launcher using the saved setup |
| `test_setup.json` | Ignored rig settings beside the generated launcher |

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
