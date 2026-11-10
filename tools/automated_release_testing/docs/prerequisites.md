# PC prerequisites

The runner is for a Linux PC connected to a separate BMC64 Raspberry Pi.
Install these packages on the **PC**, not on BMC64. Python 3.10 or newer
is required; no virtual environment or pip packages are needed.

## Debian, Ubuntu and Linux Mint

Run these commands yourself in a terminal:

```sh
sudo apt-get update
sudo apt-get install python3 ffmpeg
```

- `ffmpeg` records HDMI audio/video; `ffplay` provides the manual video check.
- VICE is not needed. The C64 diagnostic's guest program is assembled in
  Python. Only the optional offline test that runs the guest in host VICE
  (`BMC64_VICE_DIR`) uses `x64`, with C64 ROMs you supply.
- No OCR engine is needed. The profile suite reads screens against the
  character ROMs already on the BMC64 SD card, downloaded through the Web UI.

Package availability varies by distribution. Use your distribution's
supported package sources.

Verify installation:

```sh
python3 --version
ffmpeg -version
ffplay -version
```

FFmpeg must include V4L2 and ALSA inputs. The HDMI capture device must offer
MJPEG at 1280x720 and 1920x1080 (the profile suite captures PET at 1080p);
check with `v4l2-ctl --list-formats-ext` from `v4l-utils`.

For native Pico fixture tests/builds, additionally install:

```sh
sudo apt-get install build-essential
```

Rebuilding Pico USB firmware requires the SDK and Arm toolchain described in
the [fixture guide](../mock_hid/README.md). These are **not required** when
flashing the supplied UF2.

## Device access

Your login must be allowed to open the UART, video and audio devices.
On Debian-family systems those permissions commonly use the `dialout`,
`video` and `audio` groups or desktop device ACLs. Check device permissions
and follow your distribution's instructions if access is denied; group
changes normally require logging out and back in.

Do not run the entire test runner as root to work around permissions.
Close serial terminals, FFplay, OBS and other capture users before testing.

## First run

1. Wire and qualify the [hardware](hardware.md), and flash the supplied
   [Pico firmware](../mock_hid/bin/mock_hid.uf2).
2. Run `./configure_test_setup` as described in the [README](../README.md).
3. Use `run_video_test`, starting it before Pi power-on if necessary.
   Confirm a visible BMC64 display, then
   close the preview.
4. Install the six supplied SD-card profiles from the
   [profile suite guide](profile-suite.md).
5. Run `run_tests` for every test, or `run_tests --list` and
   `run_tests TEST...` for individual ones.

Configuration lists device paths without opening hardware. Offline tests
do not access BMC64, but a hardware run opens the saved devices and can
reset/restart the emulators as documented.
