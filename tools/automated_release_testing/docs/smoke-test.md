# Automated C64 smoke test

This is a smoke test of an already running C64 release, not a candidate
installer or a complete release certification. It requires Python 3.10+,
FFmpeg with V4L2/ALSA input, and the qualified Pico fixture.
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

Without `--execute`, the script only assembles the guest program (source,
listing and PRG) and writes a `PREPARED` result under ignored `results/`. It opens no UART,
capture device or HTTP connection. `PREPARED` is not a hardware PASS.

## Configure once

From the repository root:

```sh
./tools/automated_release_testing/configure_test_setup
```

Install the six [test profiles](profile-suite.md) first. Close existing capture
viewers and keep guest work saved before approving the restart.

The wizard now performs the setup with a live preview:

1. Select the HDMI video device. FFplay opens before you power on the Pi.
2. Power on the Pi if needed, wait for its display, and enter the current
   BMC64 IP address (for example `192.168.1.50`, without `http://`).
3. If the Web UI requires a PIN, enter it at the hidden prompt. Use the same
   PIN on the test profiles. The PIN is kept in memory only, never saved.
4. Verify the initial HTTP connection and choose Wi-Fi or Ethernet. Ethernet
   requires an onboard Ethernet port and cable; USB Ethernet is not supported.
5. Select the Pico UART and optional BMC64 diagnostic UART, ALSA audio device,
   notes and guest assertion timeout.
6. Confirm the listed test-profile updates and C64 restart. The wizard reads
   every settings file first, then saves only the changed `network_device`
   value using `/api/fs/save` and verifies the entire file by reading it back.
   Other settings, Wi-Fi credentials and PINs are preserved. The Web UI save
   endpoint keeps the previous changed file as a `.bak`.
7. With the preview still open, wait for C64 to reboot and show its IP.
   **Enter the address shown after reboot**, even if it is different from the
   address used initially.
8. The wizard verifies the C64 profile, a real restart and the selected network
   connection. Only then does it save the local setup.
9. Press Enter at the final prompt to close the preview and finish.

Do not press Q to close the preview mid-setup. If it exits, setup stops before
its next operation; the process is also closed on cancellation or failure.
The standalone `run_video_test` remains available for later manual checks.

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
Configuration opens video and uses HTTP, but does not open UART adapters,
send USB input, upload a diagnostic or install firmware. The profiles must
already exist, with the Web UI enabled.

After verification, the wizard saves `test_setup.json` with owner-only
permissions beside executable `run_tests`. The settings file is ignored by Git.
It records `network` as `wifi` or `ethernet` and the final verified URL.
Test preflight checks selected profiles against that saved network setting.
Old setup files without `network` still load; rerun configuration to add it.
Run the wizard again to change settings: Enter keeps each saved value, and
`-` clears an optional UART or notes field.

Cancellation before confirming the remote updates changes no settings.
If saving a profile fails, the wizard names the failed file, verified
profiles and unprocessed profiles, and does not restart the Pi or replace the
local setup. A lost save reply may mean that file was written: inspect the
message and rerun setup to finish. Already-written remote profiles are not
automatically rolled back. Likewise, if final address/network verification
fails, the local setup remains unchanged but remote updates remain applied.
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

[`tests/basic/guest.py`](../tests/basic/guest.py) holds the guest: 6502
assembly source with the run's marker data, assembled by the small built-in
[`assembler.py`](../tests/basic/assembler.py) into a PRG whose BASIC line is
`10 SYS<start>:REM BMC64 SMOKE <run id>`. The guest uses screen RAM at $0400,
colour RAM at $D800, SID voice 1 and the KERNAL `GETIN` keyboard buffer (as
BASIC's `GET` does). It is not suitable for C128/other cores or relocated
screen memory.

A red 32-by-18-character rectangle surrounds an 80-bit black/white grid.
Each bit occupies three columns and two rows. Bytes, most-significant bit
first: `$B6 $64`, four run-ID bytes, counter, input state, sum of the preceding
eight bytes modulo 256, and `$A5`. State 0 waits for Space; state 1 latches
receipt and closes the SID gate. The counter advances every 10 frames and
wraps at 255. Each update redraws the counter, state and checksum while the
raster is in the bottom border, so captured frames are not half-updated.

The PC searches for the red bounding rectangle in 480x360 RGB samples at
5 fps, then samples cell centres. Checksum validation rejects partially
updated grids. Tone analysis uses mono 8 kHz PCM, AC RMS and Goertzel energy
between 900 and 1100 Hz. Both recorded and raw analysis audio use
sample-count timestamps to avoid small ALSA timestamp reversals. This does
not measure physical HDMI cadence or audio/video synchronization.

## Evidence and cleanup

The diagnostic's directory in the run folder (`07-c64-default/` in a full
run; see [run evidence](test-framework.md#run-evidence) for the files shared
by the run) contains:

- `result.json`: status, stage, assertions, guest hash, target path and
  cleanup errors.
- `diagnostic.asm`, `diagnostic.lst` (addresses and bytes) and
  `diagnostic.prg`: the exact guest program.
- `status-before.json` / `status-after.json`: firmware, board and API profile.
- `capture-1/`: `capture.mkv` with the original MJPEG video and stereo PCM
  audio, `ffmpeg.log` and `capture-command.json`. The capture is reopened
  after the C64 profile is verified, so it is locked to the C64 HDMI mode.
- `ready.ppm` / `response.ppm`: first recognised ready/response frames.
- `failure.ppm` on failure, or `last-frame.ppm` on success: last available
  video frame even when no guest marker was recognised. `capture-health.json`
  records black/flat-frame metrics, frame age, or that no video frames arrived.
- `observations.jsonl`: monotonic observation times, counters, states and
  measured audio values.

The run folder holds `pico.log` (readable) and `pico.raw` (exact transcript) and, with a BMC64 UART:

- Optional `uart-findings.json`: critical signatures, known informational
  autostart probes and unclassified warnings, with raw lines and monotonic times.

`bmc64-uart.log` is the readable UART log. Each line starts with the PC
wall-clock time its first byte arrived, and the runner adds a
`===== [n/total] test-name (machine) =====` line when each test starts.
BMC64 ends lines with LF CR, so any run of CR/LF is one line break; a
partial final line is kept, and fragmented serial reads do not split
messages. Undecodable bytes appear as `�`. `bmc64-uart.raw` keeps the exact
received bytes for diagnosing garbled output. Findings in
`uart-findings.json` carry both the log's wall-clock `time` and the
monotonic time used by test results.

Known panic, abort, assertion, memory and profile-start failure signatures
are critical. Expected VICE disk/tape/cartridge PRG probes,
`archdep_mkdir unimpl`, and the printer, plotter and drive ROM messages every
boot reports (MPS-803, NL-10, 1520, `DriveROM`, the long `dosCMDHD` ROM) are
informational; other error/warning messages are
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
- **Pico invalid-line error or first-command timeout after reboot:** the Pico
  can restart with the Pi and receive UART noise in an unfinished input line.
  The runner sends a leading newline before every command, including after
  profile restarts, then waits for that command's correlated reply. The
  firmware ignores the empty line itself; if it terminated UART noise, the
  resulting `0 ERROR invalid-or-long-line` is logged, not accepted as the
  command response. In `pico.log` that appears after `Pico restarted` as
  `Pico discarded restart noise`; without a restart it is shown as an error. Input is still sent only
  once; uncertain commands are not retried. If timeouts persist, inspect
  `pico.log` for repeated `Pico restarted` lines or errors and check fixture power and UART wiring.
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

Validated on BMC64 5.4.5, Raspberry Pi 3 Model B and Raspberry Pi 4 Model B,
C64 PAL, with the MS2109 dongle and RP2040 fixture. The test takes 20-30 seconds
on this configuration; recording output is MJPEG 1280x720 plus 48 kHz stereo PCM.

This does not qualify other profiles, NTSC, mouse/gamepad routing, modifiers,
reconnect recovery, HDMI no-signal detection, long soaks, disk persistence,
candidate deployment, clean installs or performance/frame timing. JSON is
implemented; JUnit export is not. See the separate
[profile suite](profile-suite.md) for basic boot/typing checks on other cores.
