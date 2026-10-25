# Automated release testing

A Linux-PC-controlled smoke test for BMC64 running on real Raspberry Pi
hardware. It tests an already running C64 release using the existing HTTP
API, a UART-controlled Pico USB keyboard fixture, and HDMI video/audio capture.

## What it tests

- Upload and autostart of a generated C64 diagnostic PRG.
- A unique visual run ID and an advancing guest counter.
- A SID tone in captured HDMI audio.
- A real USB Space keypress producing a visible guest response.
- Silence after the guest receives Space.
- Successful recording finalization and input cleanup.

Each run saves a JSON PASS/FAIL result, recording, recognised screenshots,
API snapshots, Pico exchanges and optional BMC64 UART logs. An HTTP success
or USB acknowledgement alone cannot pass the test.

Validated configuration: Raspberry Pi 3 Model A+, BMC64 5.4.4, C64 PAL,
RP2040 Pico and MacroSilicon MS2109 HDMI capture. Other configurations are
not covered by that validation.

## Requirements

- Linux PC with Python 3.10+, VICE's `petcat`, and FFmpeg with V4L2/ALSA input.
  Python uses only its standard library.
- BMC64 running C64 with its Web UI enabled.
- Raspberry Pi Pico (RP2040), connected to BMC64 as a USB HID device, with a
  separate PC-side 3.3 V UART adapter.
- HDMI capture providing video and audio.
- Optional second UART adapter for BMC64 diagnostic logs.

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
`run_tests`. Run the configurator again only when the rig changes; existing
answers become its defaults. If the Web UI requires a PIN, the test prompts
for it each time rather than storing it.

**`run_tests` executes the hardware test:** it uploads and autostarts the PRG,
resets the emulated C64 and discards its current unsaved state. It also opens
capture devices and sends USB input. It does not reboot the Pi, flash firmware
or install updates.

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
python3 -m unittest discover -s tools/automated_release_testing -p test_smoke.py -v
make -C tools/automated_release_testing/mock_hid test all
```

These use synthetic video/audio, a local HTTP server and simulated UART
devices; they do not contact BMC64 or exercise the Pico.

## Files and generated outputs

| Location | Contents |
| --- | --- |
| `run_smoke.py` | PC orchestration and hardware access |
| `configure_test_setup.py` | Interactive local setup and launcher generation |
| `run_video_test`, `preview_video.py` | Manual FFplay video check using saved settings |
| `diagnostic.py` | Guest generation, visual decoding and audio analysis |
| `test_smoke.py` | Offline tests |
| `docs/` | Hardware setup and operating instructions |
| `mock_hid/` | Pico firmware, native client, protocol and tests |
| `mock_hid/bin/` | Supplied UF2 and build/checksum information |
| `results/` | Ignored per-run evidence, generated PRGs and recordings |
| `mock_hid/build/`, `mock_hid/build-native/` | Ignored build products |
| `local/` | Ignored operator-specific notes and configuration details |
| `run_tests` | Ignored generated launcher using the saved setup |
| `test_setup.json` | Ignored rig settings beside the generated launcher |

## Limitations

This is a C64 keyboard/video/audio smoke test, not full release certification.
It does not install candidates, certify frame timing or performance, test
disk persistence, or provide unattended power/boot recovery. JUnit export
and automatic UART error classification are not provided.

The fixture also exposes a mouse and gamepad, but the automated test only
uses its keyboard. It is generic HID, not a reproduction of a particular
keyboard/controller; the separate [Keyrah fixture](../keyrah_test/README.md)
serves that specialist use case.
