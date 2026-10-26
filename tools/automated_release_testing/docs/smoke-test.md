# Automated C64 smoke test

This is a smoke test of an already running C64 release, not a candidate
installer or a complete release certification. It requires Python 3.10+,
VICE's `petcat`, FFmpeg with V4L2/ALSA input, and the qualified Pico fixture.
Python uses only the standard library; no pip packages are required.

## Safety and preparation

- Coordinate target access first. If the `c64-test-de56af` profile is not
  running, the test starts it (Start once, restarting BMC64; the profile must
  be installed, see the [profile suite guide](profile-suite.md)). Autostart
  then resets the emulated C64 and loses its current unsaved state. There is
  no update or firmware flash.
- Close FFplay, VLC, OBS and all serial terminals, including picocom.
- The Pi appears to enable HDMI only if the capture device is active while it
  boots: a reboot with the capture closed has given a black picture until the
  next reboot. Both tests keep recording through their own profile restarts.
  After rebooting the Pi by hand, start `run_video_test` before power-on.
- Use separate adapters for Pico control and BMC64 diagnostics; enable BMC64
  UART logging. Never connect adapter power or 5 V UART signals.
- Keep the PC playback volume low. The diagnostic plays a roughly 1 kHz SID
  tone until it receives Space, then remains running silently.
- Use an unobscured C64 display with normal colour output. Menus, monochrome
  filters, clipping or extreme colour changes can prevent recognition.
- Enable the existing Web UI and confirm its actual URL/port.

## Offline preparation

From the repository root:

```sh
./tools/automated_release_testing/run_smoke
python3 -m unittest discover \
  -s tools/automated_release_testing/unit_tests \
  -t tools/automated_release_testing -p test_smoke.py -v
```

Without `--execute`, the script only generates a BASIC listing, tokenised
PRG and `PREPARED` result under ignored `results/`. It opens no UART,
capture device or HTTP connection. `PREPARED` is not a hardware PASS.

## Configure once

From the repository root:

```sh
./tools/automated_release_testing/configure_test_setup
```

The wizard asks for:

- BMC64 IP address only (for example `192.168.1.50`), without `http://`
  or a trailing slash. The wizard constructs the HTTP URL automatically.
- Pico UART path and optional separate BMC64 diagnostic UART path.
- HDMI video path and ALSA audio device, with capture defaults displayed.
- Optional board/profile/ROM notes and the assertion timeout.
- Whether to prompt for a Web UI PIN at test time. The PIN itself is never
  requested or saved by configuration.

IPv4 and IPv6 addresses are accepted. New setups use HTTP on its default
port. Reconfiguring an existing setup preserves its saved URL scheme and
explicit port while replacing the IP address. A non-default Web UI port can
be set in `test_setup.json`'s `url` or supplied with the runner's
`--url` override.

UART and video devices appear in numbered menus. Enter a number to select,
or type an absolute path manually. Stable names are preferred and their
resolved `/dev/ttyUSB...` or `/dev/video...` paths are shown alongside them;
aliases for the same device are listed only once. The BMC64 UART menu
excludes the selected Pico adapter. Enter `-` to omit the optional BMC64 UART.
Saved paths remain defaults, including when a device is temporarily unplugged.

The wizard cannot infer which adapter is physically wired to the Pico or
which camera is HDMI. Use device names and the
[hardware guide](hardware.md) to identify them; for the validated dongle
choose the MacroSilicon `video-index0` interface. If no devices are found,
the wizard explicitly offers manual path entry. Audio remains a text prompt.
This is configuration only: directory entries are inspected, but devices
are not opened and no HTTP requests, serial connections or captures start.

On confirmation, the wizard saves `test_setup.json` with owner-only
permissions beside executable `run_tests` and the Python scripts.
Both are ignored by Git. Run the wizard again to change settings: Enter
keeps each saved value, and `-` clears an optional UART or notes field.
Cancellation before saving leaves the existing configuration unchanged.
Malformed saved settings cause an explicit error, not a fallback to defaults.

## Run repeatedly

If checking boot-time HDMI output, first run `run_video_test` as described
in the [hardware guide](hardware.md#hdmi-video), then power on the Pi
manually. Close that preview before starting the smoke test.

After saving guest work and closing capture viewers/serial terminals, run
from the repository root:

```sh
./tools/automated_release_testing/run_tests c64-default
```

Or, from `tools/automated_release_testing/`:

```sh
./run_tests c64-default
```

Plain `./run_tests` runs this diagnostic after all the profile boot/typing
tests.

The launcher finds its own directory, so it also works by absolute path from
another working directory. It loads the saved setup and explicitly enables
hardware execution. It returns the runner's exit status. There are no setup
questions on ordinary runs except the PIN prompt if configured.

Additional runner options override saved values for that invocation:

```sh
./tools/automated_release_testing/run_tests c64-default --timeout 90
```

To use the saved settings for offline generation instead of executing:

```sh
./tools/automated_release_testing/run_smoke \
  --config tools/automated_release_testing/test_setup.json
```

Loading a configuration does not itself enable hardware access. Only the
`run_tests` launcher or an explicit `--execute` does so. Configuration files
cannot contain `execute`, arbitrary command-line options or a saved PIN.

## Direct command-line use

From the repository root, replace the URL and adapter placeholders:

```sh
./tools/automated_release_testing/run_smoke \
  --execute \
  --url http://BMC64_HOST/ \
  --pico-uart /dev/serial/by-id/YOUR_PICO_UART \
  --bmc-uart /dev/serial/by-id/YOUR_BMC64_UART \
  --setup 'Pi model, C64 profile, ROM identity and capture setup'
```

Find adapter paths using the [hardware guide](hardware.md).
Defaults are the MacroSilicon
index0 video interface, MJPEG 1280x720 at requested 30 fps, and ALSA
`plughw:CARD=MS2109,DEV=0`. Override with `--video` and `--audio`.
Use `--ask-pin` if the Web UI requires a PIN; it is prompted without echo
and is not written to the result or command log. HTTP Basic Auth is not
encrypted on an HTTP LAN connection.

`--bmc-uart` is optional; its absence is explicitly recorded as
`uart_collected: false`. Known critical UART signatures fail the run;
other error/warning lines are retained for review. `--setup` records operator-supplied ROM/profile/filter notes;
ROM identity is not automatically measured. Each guest assertion has a
60-second monotonic deadline, adjustable with `--timeout`.

## What PASS proves

1. The existing API reports C64 and accepts a uniquely named PRG upload and
   autostart. Upload does not overwrite another file.
2. HDMI contains the current random 32-bit run ID, a valid marker checksum,
   and at least two different progress counter values.
3. At least two one-second audio blocks after the marker appears contain
   the expected PAL/NTSC-tolerant SID tone.
4. A 100 ms Space press through the Pico produces a visible, latched guest
   response in at least two valid frames. A USB ACK alone cannot pass.
5. After the visible response, two successive audio blocks show the tone
   stopped. Silence must be below 15% of measured tone RMS, with a 0.001
   full-scale noise floor.
6. Capture finishes successfully and cleanup reports no errors.

Missing evidence, capture failure, a frozen counter, stale screen, failed
UART command, or deadline expiry fails the run. Input commands are not
retried. RELEASE is attempted during cleanup; any cleanup failure is retained
and makes the result fail without replacing the original error. A recovery
run creates a separate result and does not erase the first failure.

## Guest marker

[`tests/basic/test_c64_default.py`](../tests/basic/test_c64_default.py) defines the diagnostic, generates C64 BASIC V2 and invokes the installed `petcat`
tokeniser. The guest uses screen RAM at $0400, colour RAM at $D800 and SID
voice 1. It is not suitable for C128/other cores or relocated screen memory.

A red 32-by-18-character rectangle surrounds an 80-bit black/white grid.
Each bit occupies three columns and two rows. Bytes, most-significant bit
first: `$B6 $64`, four run-ID bytes, counter, input state, sum of the preceding
eight bytes modulo 256, and `$A5`. State 0 waits for Space; state 1 latches
receipt and closes the SID gate. The counter wraps at 255.

The PC searches for the red bounding rectangle in 480x360 RGB samples at
5 fps, then samples cell centres. Checksum validation rejects partially
updated grids. Tone analysis uses mono 8 kHz PCM, AC RMS and Goertzel energy
between 900 and 1100 Hz. Both recorded and raw analysis audio use
sample-count timestamps to avoid small ALSA timestamp reversals. This does
not measure physical HDMI cadence or audio/video synchronization.

## Evidence and cleanup

Each ignored `results/<UTC-time>-<run-id>/` contains:

- `result.json`: status, stage, assertions, setup, guest hash, target path
  and cleanup errors.
- `diagnostic.bas` / `diagnostic.prg`: exact generated payload.
- `status-before.json` / `status-after.json`: firmware, board and API profile.
- `capture.mkv`: original MJPEG video and stereo PCM audio.
- `ready.ppm` / `response.ppm`: first recognised ready/response frames.
- `failure.ppm` on failure, or `last-frame.ppm` on success: last available
  video frame even when no guest marker was recognised. `capture-health.json`
  records black/flat-frame metrics, frame age, or that no video frames arrived.
- `observations.jsonl`: monotonic observation times, counters, states and
  measured audio values.
- `pico.log`, optional `bmc64-uart.log`, `ffmpeg.log`, `petcat.log`,
  `volume.json` and `capture-command.json`.
- Optional `uart-findings.json`: critical signatures, known informational
  autostart probes and unclassified warnings, with raw lines and monotonic times.

BMC64 UART logging adds one monotonic timestamp at the start of each
LF-delimited line, rather than at serial-read boundaries. Original UART
bytes, CR/LF endings and partial final lines are retained; fragmented
serial reads do not insert newlines into messages. Binary/garbled source
bytes are retained rather than hidden.

Known panic, abort, assertion, memory and profile-start failure signatures
are critical. Expected VICE disk/tape/cartridge PRG probes and
`archdep_mkdir unimpl` are informational; other error/warning messages are
retained without automatically failing the run. This is not exhaustive
crash detection.

Exit status is zero only for PASS or offline PREPARED; hardware failures are
nonzero. Ctrl+C/SIGTERM attempts input release and recording finalization.
An abruptly killed PC cannot guarantee cleanup; the Pico lease remains a
secondary safeguard, not a host/power recovery mechanism.

The diagnostic PRG remains at `/bmc64-smoke-<run-id>.prg` on the target.
It is not automatically deleted or reset, preserving failure evidence and
avoiding further target changes. Remove old payloads manually through the
Web UI when no longer needed. Recordings can be large; manage retained runs.

## Troubleshooting

- **UART locked/busy:** close picocom or any other serial reader. Pico
  control and BMC64 logging must use different adapters.
- **Pico invalid-line error during manual bring-up:** a stale partial line
  can be rejected before a valid command. The runner sends a leading newline
  and waits for its own correlated reply. It does not retry uncertain input.
- **Capture startup failure:** close competing viewers and check device paths.
  Confirm video preview and HDMI audio independently using the hardware guide.
  Black startup frames fail before uploading the guest. Inspect `failure.ppm`
  and `capture-health.json`; they identify unusable capture, not its root cause.
- **Guest readiness timeout:** inspect the recording, UART log and API status.
  Autostart acceptance does not prove the guest started. Check that C64 is
  selected, no menu obscures the display and the colour grid is visible.
- **Input response timeout:** a Pico ACK is only a USB transfer completion.
  Check keyboard mapping and the guest's visible state in the recording.

To decode-check a saved recording, replace `RUN_DIRECTORY` with its evidence
directory and retain the Matroska millisecond video timebase:

```sh
ffmpeg -hide_banner -v error -i RUN_DIRECTORY/capture.mkv \
  -map 0:v:0 -map 0:a:0 -enc_time_base:v 1:1000 \
  -fps_mode passthrough -f null -
```

Using a coarse 30 fps output timebase for this verification can itself
produce duplicate DTS warnings from rounding variable capture timestamps.

## Validated configuration and limits

Validated on BMC64 5.4.4, Raspberry Pi 3 Model A+, C64 PAL, with the MS2109
dongle and RP2040 fixture. A run takes about one minute on this configuration;
recording output is MJPEG 1280x720 plus 48 kHz stereo PCM.

This does not qualify other profiles, NTSC, mouse/gamepad routing, modifiers,
reconnect recovery, HDMI no-signal detection, long soaks, disk persistence,
candidate deployment, clean installs or performance/frame timing. JSON is
implemented; JUnit export is not. See the separate
[profile suite](profile-suite.md) for basic boot/typing checks on other cores.
