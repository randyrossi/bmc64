# Profile boot and typing suite

The suite exercises basic release usability across six emulator cores:
boot into a known profile, verify BASIC is ready, and verify real USB typing
on screen. It does not upload a guest program or rely on machine-specific
autostart support.

## Install the supplied default profiles

Ready-to-copy profiles are provided in
[sdcard_profiles/profiles](../sdcard_profiles/profiles). Each profile uses
default emulator/BMC64 settings except:

```ini
network_device=2
webui_enabled=1
```

The PET profile is the exception: its `vice.ini` selects a 40-column 3016
(BASIC 2, 16K, graphics keyboard, no CRTC: `KernalName="kernal2"`,
`EditorName="edit2g"`, `BasicName="basic2"`, `RamSize=16`, `IOSize=2048`,
`Crtc=0`, `VideoSize=40`, `KeyboardType=4`, `KeymapSymFile="rpi_grus_sym.vkm"`),
as saved by BMC64's Model menu. On the MS2109 capture device, an 80-column
PET pixel is two capture pixels wide, finer than the device actually
resolves: single-pixel gaps in `*`, `M` or lower case `c`/`o` blur shut, with
or without the CRT filter and scaling interpolation. 40 columns doubles that.
The default 80-column 8032 screen is therefore not screen-checked. VICE
discards the whole `vice.ini` and falls back to defaults if any line fails to
apply, so the SD card must hold `/PET/kernal2`, `edit2g` and `basic2`.

All supplied folder IDs share the collision-avoidance suffix `-de56af`:
`c64-test-de56af`, `c128-test-de56af`, `vic20-test-de56af`,
`plus4-test-de56af`, `plus4emu-test-de56af` and `pet-test-de56af`.
The default suite manifest targets these exact IDs, not the readable
display names, so an existing profile named `c64_test` is not selected.

The supplied templates select onboard Wi-Fi and enable the Web UI.
`configure_test_setup` can switch all six installed test profiles to Ethernet
(`network_device=1`) or Wi-Fi (`network_device=2`) over the current HTTP
connection. It preserves other settings, verifies the saved files, restarts
into C64 and asks for its new IP before saving the local setup. No SD remount
is needed to change the test network. Only the manifest's test profiles are
updated, not Main or unrelated profiles. The Ethernet path requires onboard
Ethernet; USB adapters are not supported.

For Wi-Fi, the existing SD-root
`wpa_supplicant.conf` and Wi-Fi firmware are reused unchanged; no network
credentials are bundled. Web UI has **no PIN**, matching its default:
use these profiles only on a trusted test LAN, or set the same PIN in each
profile locally before using them. Never commit a private PIN.

The profiles select standard PAL HDMI entries from the SD card's existing
`machines.txt` and prefer Start once. Apart from PET's, no `vice.ini` is
supplied: the emulator uses its built-in defaults, not Main's saved settings. No cartridges,
autostart or disks are configured. ROMs and machine kernels must already
be installed on the SD card.

With BMC64 powered off and the SD card mounted on the PC, copy the six
subdirectories from the supplied `profiles` folder into `/profiles/` on
the SD card. Create that destination folder if it does not exist.
**Do not replace existing same-ID folders without reviewing/backing them
up**, and do not just merge into previously customised test profiles:
old settings or a `vice.ini` would override the intended defaults.
No `active.txt` is bundled, so copying does not change the power-on profile.
Safely eject the card afterwards.

There is no need to visit every machine's menus to create these profiles.
The files are supplied locally only; the runner never installs them.
Keep the SD card's existing `machines.txt`, `wpa_supplicant.conf`, ROMs,
firmware and normal settings in place.

Standard PET HDMI entries switch the signal from 720p to 1080p. The suite
reopens the capture device after every profile boot and captures PET at
1920x1080, the other cores at 1280x720, so the capture device never has to
follow a mode change mid-stream or rescale the picture.

## Alternatively, create customised profiles manually

Use a dedicated test SD card or save all guest work first. The suite
restarts BMC64 and discards unsaved emulator state. It never creates or
edits profiles, installs a release, flashes firmware or power-cycles the Pi.

For each machine, switch to its normal working configuration, then use
**Profiles -> New profile from current settings**. Use these display names:

| Display name | API machine/core | Required boot display |
| --- | --- | --- |
| `c64_test` | `C64` | Commodore 64 BASIC V2 and READY |
| `c128_test` | `C128` | Commodore BASIC V7.0 and READY, 40-column VIC output |
| `vic20_test` | `VIC20` | CBM BASIC V2 and READY, normal 22-column screen |
| `plus4_test` | `Plus4` (VICE) | Commodore BASIC V3.5 and READY |
| `plus4emu_test` | `Plus4Emu` | Commodore BASIC V3.5 and READY |
| `pet_test` | `Pet` | Commodore BASIC (2) and READY; 40-column 3016 (see below) |

Prepare every profile with:

- Working ROMs and a normal, unobscured BASIC boot screen. No cartridge,
  autostart, automatically attached disks, open menu or guest application.
- Network and Web UI enabled, the same reachable IP/port, and the same PIN
  as the saved PC setup. Profiles copy settings: enabling Web UI in Main
  afterwards does not update already-created profiles.
- A working USB keyboard mapping and no USB-to-joystick keyboard mode.
- Consistent HDMI output compatible with the capture device. Avoid
  unnecessary changes of HDMI mode during the suite. Select C128 VIC 40-column
  output, not VDC-only output.
- Default scaling, no clipping, and normal screen colours. Screens are read
  cell by cell at the capture's native resolution against the core's own
  character ROM, so custom fonts, palettes or scaling are not qualified.

Save settings in each profile. Clear profile autostart and auto-attached
media. Manually **Start once** into each profile once and confirm that
BASIC, keyboard, Web UI and HDMI work before attempting automation.
See the repository's [Profiles guide](../../../docs/PROFILES.md) for
profile creation and saving.

Profile **display names and IDs are different**. For example, creating
`c64_test` normally creates `/profiles/c64-test/`; duplicates may acquire
suffixes, and renaming a profile does not rename its ID. The suite reads
the existing filesystem API and matches an exact display name **or**
an exact directory ID. It never guesses an ID by replacing underscores.
Missing or ambiguous matches fail preflight before the first restart.

The default manifest, [suite_profiles.json](../suite_profiles.json), maps
the six suffixed folder IDs to exact core names. If you create profiles
manually instead, supply a custom manifest with their actual IDs or names.
`Plus4` and `Plus4Emu` are separate
required cases; their similar boot screens are distinguished by API core
identity. The suite checks profile metadata and rejects attached disks
or autostart before switching. Other settings still need manual preparation.

## Run the suite

Install the supplied profiles (or create your own), install the
[PC prerequisites](prerequisites.md), configure the rig once,
and perform the video/IP check in the [README](../README.md).
Close the preview and all serial terminals.

The files in [tests/boot/](../tests/boot/) define tests named `<machine>-boot-typing`;
the manifest maps their machines to installed profiles:
`c64-boot-typing`, `c128-boot-typing`, `vic20-boot-typing`,
`plus4-boot-typing`, `plus4emu-boot-typing` and `pet-boot-typing`. From this
directory:

```sh
./run_tests                            # every test, including c64-default
./run_tests boot                       # all boot/typing tests, no diagnostic
./run_tests --list                     # test names and descriptions
./run_tests pet-boot-typing            # only the named tests (any number)
```

Selected boot/typing tests run first, in manifest order, then `c64-default`
(the C64 video/audio diagnostic, see the [smoke-test guide](smoke-test.md)).
A single-core result does not qualify the other cores. If the
`c64-test-de56af` profile is not running, `c64-default` starts it first
(Start once, verified like a boot/typing switch).

The existing saved `test_setup.json` and `run_tests` need no regeneration.
The PIN is prompted if configured; it is not stored.

Terminal output is timestamped and flushed immediately. It identifies the
current case and stage, selected profile, generated typing token, verification
method and cleanup actions. Long waits update about every five seconds with
elapsed time and the latest observation: HTTP/profile state, USB mount state,
capture health, or which screen cells differ from the expected text. A matching
screen still needs two fresh confirmations; progress messages do not change
the pass criteria or deadlines. Blocking HTTP calls can delay an update
until that call completes or times out.

To check the manifest without opening hardware:

```sh
./tools/automated_release_testing/run_smoke
```

This records PREPARED, not PASS. It does not check whether profiles actually
exist on the SD card.

For a subset or different profile names, copy the manifest to an ignored
file such as `local/my-suite.json`, edit it, and supply its path:

```sh
./run_tests --manifest local/my-suite.json --timeout 90
```

The manifest maps machines to profiles; Python test files define the test
names shown by `--list`. Each item needs only `machine` and `profile`; machines may not be
repeated. `c64-default` uses the manifest's C64 profile.
Case order follows the manifest. A subset run proves only its listed cases;
the actual manifest is always recorded in the result.

Direct execution is also available with the normal setup options:

```sh
./tools/automated_release_testing/run_smoke \
  --config tools/automated_release_testing/test_setup.json --execute
```

## What PASS proves

For each selected profile:

1. The start-once API request is issued once and `/api/status` reports the
   expected actual profile ID and exact machine/core, with no profile
   start error. A changed ID or new uptime evidence must confirm the
   transition; an accepted request or transient HTTP outage alone cannot
   pass a restart of the already-active profile. A restart may interrupt
   the HTTP reply after the request has been applied. In that case the
   runner records an unknown request outcome and verifies the same profile
   transition instead of failing immediately or resending the request.
   If the requested profile is already active, `/api/profiles/start` would
   reject it as "Already running". The runner instead sends `/api/reboot`
   once: firmware preserves the active Start-once profile across this reboot.
   A new uptime must still prove a fresh boot of the expected profile/core.
   An unchanged pre-existing profile error is ignored only while waiting
   for that reboot; it must be clear after the verified transition.
2. After the verified switch the HDMI capture is reopened. Two fresh frames
   then show exactly the expected BASIC boot screen: banner, memory line and
   READY, every other checked cell blank, and only the cursor on the input
   row. Characters are compared with the core's character ROM, read from the
   SD card.
3. The Pico remounts as an available USB device. The runner releases input,
   then types a random eight-letter token with explicit press/release
   pairs. It does **not** send Return or execute a BASIC command.
4. Two fresh frames captured after typing show the same screen with exactly
   that token on the input row, followed by the cursor. The final API
   profile and core still match.
5. No classified critical UART message, capture failure, or cleanup error
   occurred during the case.

### Reading the screen with the character ROM

No OCR is used. Tesseract misread real screens (V/U, Z/2, PET `*` as `#`)
and could not tell a corrupt capture from a slow boot. Instead, each case
downloads the character ROM the emulator draws with:

| Core | ROM on the SD card | Character set | Channel |
| --- | --- | --- | --- |
| C64 | `/C64/chargen` | upper case at 0 | red, light text |
| C128 | `/C128/chargen` | upper case at 0 | green, light text |
| VIC20 | `/VIC20/chargen` | upper case at 0 | red, dark text |
| Plus4 | `/PLUS4/kernal` | `$D000` set at 0x1000 | red, dark text |
| Plus4Emu | `/PLUS4EMU/p4kernal.rom` | `$D000` set at 0x1000 | red, dark text |
| Pet | `/PET/chargen` | lower case set at 0x400 | green, light text |

Only the file's size and SHA-256 are recorded in the case result; ROM data is
never written to the repository or the evidence.

For each frame, the matcher finds the character grid. The pixel size is known
from the core's HDMI mode (3x3 capture pixels, 6x3 for VIC-20, 4x4 for PET), so
it only searches for the position that best fits the known boot text, up to
one character cell from where the text should sit. It averages all the capture
pixels of each character pixel, so scanlines cancel out however the renderer
draws them: the Pi 3 uses two bright rows and a dark one, the Pi 4 one bright
row between two dim ones. It thresholds 55% of the way from background to ink
(MJPEG colour bleeds into neighbouring pixels at about half strength) and
compares each 8x8 cell with its ROM glyph. Every cell from the
top of the screen to two rows below the input row must match within three
bits of 64, and every other cell must be blank. If the boot banner itself no
longer matches in more than half its cells, the frame is reported as
unreadable: a corrupt capture or an unexpected screen, not a slow boot.

The typing alphabet is derived from the ROM: only letters whose glyph
differs from every other letter by at least six bits are typed (for
example `ADJKLMPQRVWXYZ` on the C64 font). Each typed letter must be within
two bits of its glyph and at least two bits closer to it than to any
other letter. The token is chosen before the switch and never used to fit the
grid.

Custom colours, fonts, viewport/scaling changes and boot layouts outside
the supplied defaults are not qualified. A mismatch report names the row,
column, expected character and bit distance; inspect the saved frame rather
than loosening the thresholds.

Every assertion has the configured timeout, normally 60 seconds; total case
time can exceed that because boot, remount and typing have separate deadlines.
HTTP calls have bounded waits. The final profile/core status is retried
until the timeout if a reply is lost, and the wait is shown in the terminal.
Input commands are not retried when their result is uncertain.

### Capture per case

A capture left running across profile switches recorded corrupted, tiled
frames after an HDMI mode change: run `20261025T181524Z` lost Plus4Emu and
PET this way, even though both booted. The suite therefore keeps one
recording segment per case. Once a case's profile/core is verified, it
closes the previous segment and reopens the capture device at that core's
size (PET 1920x1080, others 1280x720), then waits for nonblack video and
audio. Analysis uses the native frame without rescaling. If three
consecutive fresh frames are unreadable, the capture is reopened once more
within that case. The previous segment keeps recording through the next
profile switch, so transitions remain in the evidence.

The suite records audio, but does **not** assert audio content. Run the
existing C64 diagnostic separately for SID tone and silence checks.

If a case fails, its evidence and original error are retained. The suite
attempts input release and waits for usable HTTP control before continuing.
If HTTP control cannot be recovered, remaining cases are NOT_RUN and no
more switches are requested. There is no automatic power recovery.

The terminal ends with a summary: one line per case (PASS, FAIL with its
stage and reason, or NOT_RUN), then the totals and the total run time.

The overall hardware result is PASS only if **all selected cases pass**
and cleanup succeeds. Any FAIL, NOT_RUN, interrupted run or cleanup failure
produces a nonzero exit status.

## End state

All profile switches use **Start once**, so the normal power-on profile is
not changed. The last attempted profile remains active; there is no automatic
return to the starting machine. Its typed token is left on screen without
Return. Switch back manually, or power off/on manually to return to the
usual power-on profile. Never assume an unsuccessful switch left a particular
machine active: inspect the saved status and the target.

## Evidence and diagnosis

A run writes one ignored `results/<UTC-time>-<run-id>/` folder for every
selected test; see [run evidence](test-framework.md#run-evidence) for the files
shared by the run. Each boot test's numbered directory
(`01-c64-boot-typing/` ...) contains:

- `result.json`, status snapshots, the switch polling log and
  `switch-request.json`, recording the accepted/unknown request outcome and
  endpoint (start or reboot).
- `capture-1/` (and `capture-2/` after a reopen): that test's recording
  segment, which continues through the next test's profile switch.
- Accepted `boot.ppm` and `typing.ppm`, the latest frame examined at each
  stage (`*-last.ppm`), and `boot-frames.jsonl`/`typing-frames.jsonl` with
  every fresh frame's timestamp, decision, fitted grid and first mismatched
  cells.
- In `result.json`: `font` (ROM path, offset, size, SHA-256),
  `typing_alphabet`, `typing_method=rom_glyphs`, `screen_grid` and the
  typed letters' bit distances in `typing_glyph_distances`.
- `failure.ppm` for failed tests that received frames, including black
  frames. Successful tests retain `last-frame.ppm`. If no frame arrived,
  `capture-health.json` says so instead of fabricating a screenshot.

Case timestamps and UART findings use the same PC monotonic clock, allowing
association with the continuous UART and capture evidence. UART text is
preserved line-by-line even when reads fragment individual messages.

Failure categories distinguish missing frames/audio, black or flat video,
stalled capture, capture startup, an unreadable screen (`screen_unreadable`),
profile-switch timeout/error, an unusable character ROM (`font_rom`), a boot
screen that does not match (`guest_not_ready`), missing typed text
(`typing_not_visible`), unavailable Pico and critical UART findings.
`fixture_or_api` covers remaining transport/device errors with their actual
exception text.

Known panic, abort, assertion, memory and profile-failure signatures are
critical. Known VICE disk/tape/cartridge probes during PRG autostart are
informational. Other error/warning lines are retained for review rather
than automatically failing an otherwise healthy boot. This classifier is
conservative, not an exhaustive firmware health checker.

If a screen check fails, open `*-last.ppm` and the matching `*-frames.jsonl`
before changing anything. `screen_unreadable` with smeared or tiled frames
points at the capture device; a few mismatched cells on an otherwise clean
frame point at the guest screen (unexpected text, a different BASIC memory
size, a wrong ROM or a non-default layout).
Black frames identify unusable capture evidence, not the cause of the
missing HDMI signal. Never mark a release good from an incomplete run.

## Coverage limits

Offline tests cover the screen matcher on screens drawn with a synthetic
font (every core's layout, blank/solid cursor, off-centre text, Pi 3 and
Pi 4 style scanlines, every single-letter substitution, missing/extra/reordered input, smeared, tiled
and noisy frames, font validation), frame freshness and capture reopening,
stale/wrong profile rejection, failed typing, lost status replies, HTTP loss,
continuation/blocking, capture evidence and UART classification. These need
no ROM data.

Tests against recorded frames need the real character ROMs, which are not in
this repository. Point `BMC64_ROM_ROOT` at a copy of the SD card (or any folder
with the same `C64/chargen`, `PLUS4/kernal`, ... layout); without it those
tests are skipped. The fixtures in `test_data/screens/` are native 1280x720
frames from the BMC64 5.4.4 recordings (Raspberry Pi 3 Model A+, MS2109):
boot screens for C64, C128, VIC20, Plus4 and Plus4Emu, live typed tokens
for C64, C128, VIC20 and Plus4 (with every single-letter substitution
rejected), a half-typed Plus4 screen, the corrupt Plus4Emu and PET captures
from run `20261025T181524Z`, and PET downscaled to 720p, which must never pass.

PET fixtures are 1920x1080 frames of the 40-column 3016: its boot screen and
a live typed token. An 80-column 8032 frame at 1080p and PET downscaled to
720p must never pass. A live C128 frame whose `K` failed at the old midpoint
threshold (`c128-typed-WQJKPDWJ.png`) and a Pi 3 C64 frame whose `J` reached
the 2-bit limit at a threshold of 0.60 (`c64-typed-QAKYKJRQ.png`) guard the ink
threshold. The `*-pi4*` fixtures are Pi 4 frames from BMC64 5.4.5, whose softer
scanlines failed the earlier reader.

All six cores passed on hardware on BMC64 5.4.5 with an RP2040 Pico, MS2109
capture and the supplied profiles, on a Raspberry Pi 3 Model B (run
`20261030T173145Z`) and a Raspberry Pi 4 Model B (run `20261030T171421Z`),
both over Ethernet. That qualifies this rig and configuration only. Qualify it again on a different Pi, ROM set,
profile, keyboard mapping or capture device before relying on it as a
release gate. The earlier C64 diagnostic hardware
validation does not qualify this suite.

These checks do not certify audio on other cores, BASIC execution, disk
I/O, peripherals, frame timing, cold power-on, installation, update paths,
long-running stability or unattended recovery.
