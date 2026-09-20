# BMC64 performance test results

Frame-budget and audio measurements on real hardware. How to collect a log, the report format and what each metric
means are in the [README](README.md).

The file has two parts:

- **Headroom.** An idle run and a stress run on each board, to show how much of the frame budget the emulator has left.
- **Device cost.** [Device cost on the Pi Zero W](#device-cost-on-the-pi-zero-w-planet-golf), at the end of this file. It
  began as a search for what was making the Pi Zero slow, and turned into a check of what each part that has been added
  to the system costs: emulated drives, networking, the web UI and more.

## Headroom: idle against stress

This part compares an **idle** run (`perf_idle.prg`, a baseline) with a **stress** run (`perf_stress.prg`, VIC and SID
together) on each board. The extra emulation time under stress shows how much headroom the board has.

All runs in this part: VICE 3.3, BMC64 5.1.6, C64 machine, PAL timing (50 Hz, 20 ms frame), SID MOS8580 with filter on,
JiffyDOS ROMs, no CPU throttling, one run per row. Numbers are from `perf_report.py --skip 3`, i.e. the first three 10
second windows (boot and workload start-up) are dropped. Busy is the share of the 20 ms frame spent working; the rest is
waiting for vblank.

**Compare a board only with itself.** BMC64 forces the cheaper `fast` SID sampling below the Pi 3, and the Pi Zero uses a
different reSID library (teensy-resid) and is mono, so busy percentages are not comparable across boards.

## Results

| # | Board | Workload | Network during the run | Windows | Busy avg | Emulate | Worst frame | Missed vblanks | Audio buffer min (of 4096) | Result |
| --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- |
| 1 | Pi 3B+ (1400 MHz) | idle | Wi-Fi connected, IP after 23 s | 23 | 48.2% | 9.4 ms | 54.0% (10.8 ms) | 0 | 2948 | PASS |
| 2 | Pi 3B+ (1400 MHz) | stress | Wi-Fi connected, IP after 11 s | 25 | 60.5% | 11.8 ms | 66.3% (13.3 ms) | 0 | 2948 | PASS |
| 3 | Pi 2 B (900 MHz) | idle | Ethernet driver up, no IP obtained (probably no cable) | 24 | 31.6% | 5.9 ms | 38.7% (7.7 ms) | 0 | 2724 | PASS |
| 4 | Pi 2 B (900 MHz) | stress | Ethernet driver up, no IP obtained (probably no cable) | 22 | 45.9% | 8.8 ms | 54.2% (10.8 ms) | 0 | 2616 | PASS |
| 5 | Pi Zero W (1000 MHz) | idle | Wi-Fi hardware up, never associated | 22 | 38.5% | 7.2 ms | 42.2% (8.4 ms) | 0 | 2948 | PASS |
| 6 | Pi Zero W (1000 MHz) | idle | Wi-Fi connected, IP after 30 s | 22 | 38.6% | 7.2 ms | 42.2% (8.4 ms) | 0 | 2948 | PASS |
| 7 | Pi Zero W (1000 MHz) | stress | Wi-Fi hardware up, never associated | 23 | 61.6% | 11.9 ms | 77.3% (15.5 ms) | 0 | 2952 | PASS |
| 8 | Pi Zero W (1000 MHz) | stress | Wi-Fi connected, IP after 11 s | 24 | 61.3% | 11.8 ms | 65.1% (13.0 ms) | 0 | 2948 | PASS |

## More detail

| # | Board | Workload | SID mode | Present | Post (avg / max) | Network events in measured windows | Audio buffer empty frames |
| --- | --- | --- | --- | --- | --- | --- | --- |
| 1 | Pi 3B+ | idle | fast resampling | 0.24 ms | 32 / 41 µs | 0 | 0 |
| 2 | Pi 3B+ | stress | fast resampling | 0.24 ms | 38 / 54 µs | 0 | 0 |
| 3 | Pi 2 B | idle | fast | 0.39 ms | 5 / 18 µs | 0 | 0 |
| 4 | Pi 2 B | stress | fast | 0.41 ms | 13 / 47 µs | 0 | 0 |
| 5 | Pi Zero W | idle | fast | 0.41 ms | 106 / 179 µs | 0 | 0 |
| 6 | Pi Zero W | idle | fast | 0.42 ms | 109 / 144 µs | 0 | 0 |
| 7 | Pi Zero W | stress | fast | 0.37 ms | 68 / 153 µs | 0 | 0 |
| 8 | Pi Zero W | stress | fast | 0.39 ms | 70 / 132 µs | 0 | 0 |

Present is copying the frame to the GPU. Post is the per-frame stage that includes the scheduler's `circle_yield()`, so its
maximum is where background tasks (network) show up.

## What stress adds over idle (same board)

| Board | Network | Idle emulate | Stress emulate | Added by stress | Stress busy avg | Left over under stress | Worst stress frame |
| --- | --- | --- | --- | --- | --- | --- | --- |
| Pi 3B+ | Wi-Fi connected | 9.4 ms | 11.8 ms | +2.4 ms | 60.5% | 39.5% | 66.3% |
| Pi 2 B | no IP | 5.9 ms | 8.8 ms | +2.9 ms | 45.9% | 54.1% | 54.2% |
| Pi Zero W | Wi-Fi not joined | 7.2 ms | 11.9 ms | +4.7 ms | 61.6% | 38.4% | 77.3% |
| Pi Zero W | Wi-Fi connected | 7.2 ms | 11.8 ms | +4.6 ms | 61.3% | 38.7% | 65.1% |

"Left over" is 100% minus the stress busy average, as a share of the 20 ms frame. The worst frame is the slowest single
frame of the run, also as a share of the frame.

## What the results show

- **Every run passes with no missed vblanks and no audio underruns.** Busy averages 31.6% to 61.6%, and the worst single
  frame anywhere is 77.3% (row 7).
- **The Pi Zero has the least headroom under stress** (+4.7 ms over idle, against +2.4 ms on the 3B+ and +2.9 ms on the
  Pi 2), but at 61% busy it still passes with room to spare. At default settings these workloads only catch large
  regressions.
- **A connected network cost nothing measurable** on any board tested: rows 1 and 2 (3B+), and rows 6 and 8 against
  rows 5 and 7 (Pi Zero W, single core, the case most likely to show it). The interference seen so far comes from a
  network that keeps failing to connect, not from having Wi-Fi on.
- **A network that keeps retrying can lengthen the worst frames.** An earlier Pi 3B+ stress run is not in the table:
  Wi-Fi was associated but had no DHCP lease, so DHCP kept retrying. Row 2 is the rerun with a working lease. Same
  board and workload:

  | | Retrying (no lease) | Row 2 (lease) |
  | --- | --- | --- |
  | Busy avg | 60.6% | 60.5% |
  | Worst frame | 94.3% (18.9 ms) | 66.3% (13.3 ms) |
  | Max `post` stage | 6.8 ms | 54 µs |
  | Network events in measured windows | 8 | 0 |

  The average is identical; only the tail moved. The retrying run's worst frames (two of 18.9 ms and four of about
  15.3 ms) had 6.8 or 3.3 ms extra in the `post` stage, in exactly the windows where the log shows DHCP retries. That is
  consistent with the network task running inside `circle_yield()` on the emulator core. It is one run each, but the
  offsets line up window for window.

## Caveats

- One run per row, taken in separate sessions. Run-to-run noise has not been measured for these workloads (on a real
  game it was about 1 to 2 ms, see below), so do not read small differences as real.
- The network descriptions come from what each log does and does not say. "Probably no cable" (Pi 2) and "never
  associated" (Pi Zero W rows 5 and 7) are inferences: those logs show no IP and no connection attempt.
- Row 2's capture originally contained two boots; only the second produced measurement windows, and it was trimmed to
  that single boot by hand. The numbers are unchanged by the trim, and `perf_report.py` now detects multiple boots in a
  log itself (see `--boot`).
- Audio was only checked through the counters, not by ear. The Pi Zero W has no audio jack, so it needs HDMI audio.
- Dual SID (`perf_sid2`) and the `perf_vic` / `perf_sid` workloads have not been run yet, and neither have heavier
  settings (CRT shader, 1080p, scanlines).
- PAL only; no NTSC runs.

## Repeating a run

Build with `--perf-stats`, capture the serial output and run the tool as described in the [README](README.md). The rows
above use:

```
python3 tools/perftest/perf_report.py <capture> --skip 3
```

The tool also prints the network state it finds in the capture, which is where the "Network" column comes from. Add
`--json <file>` to save a baseline for later comparison with `--baseline`.

---

# Device cost on the Pi Zero W (Planet Golf)

These runs started as a search for what was making a real game slow and glitchy on a Pi Zero W. They turned into a
check of what each part that has been added to the system costs: emulated drives, networking, the web UI and other
options. Each is added one at a time to the same game, and the emulator's frame time and the audio buffer are measured.
The Pi Zero is the board with the least headroom, so it is where the cost shows first.

**Short version**

- **Each emulated drive costs CPU even when nothing uses it.** On the Pi Zero W that was about **+0.8 ms** (1541-II),
  **+0.8 ms** (CMD HD) and **+2.1 ms** (FD-4000) of every 20 ms frame *at rest*, and more under load.
- **Three extra drives together cost about 3.6 ms at rest and 5.6 ms under load** (18% and 28% of the frame).
- With **drive 8 only, or drives 8 and 9, everything is comfortable**, with Wi-Fi networking on. With drives 8, 9 and 10
  there is little margin. With **all four drives, the audio buffer nearly empties** and the game can glitch.
- **Wi-Fi networking (joined, idle), the web UI (enabled, idle), an attached IDE64 image, and starting the game from the
  REU instead of the disk made no measurable difference.** See [What made no measurable difference](#what-made-no-measurable-difference).
- **VICE's "trap idle" drive setting recovered about 40% of the at-rest cost of the three extra drives, and none of it
  under load.** See [Mitigation tried: drive idle method](#mitigation-tried-drive-idle-method-trap-idle).

## What was tested

| | |
| --- | --- |
| Board | Raspberry Pi Zero W, 1000 MHz, HDMI 1280x720 at 50 Hz. Never throttled; hottest run 53 °C |
| Software | BMC64 5.1.7 built with `--perf-stats`. |
| Machine | C64, PAL, SID MOS8580 with filter on, reSID `fast` sampling (teensy-resid on this board), JiffyDOS C64 KERNAL and, except in row L, JiffyDOS 1541-II and 1581 drive ROMs |
| Game | *Planet Golf*, a `.d64` on the SD card, autostarted, then played by hand through the main menu, planet choice (Earth) and load first hole in the game. About 60 to 100 s per run |
| Started from | The game's disk option in rows A to G, K and L, its REU option in row H, not recorded for rows I and J |
| Drives | Unit 8 = 1541-II always. Units 9 to 11, when enabled: 9 = 1541-II, 10 = CMD HD, 11 = FD-4000. True drive emulation left at its default (the logs do not record it) |
| Networking | Wi-Fi joined with an IP address before the game started (2.4 GHz access point). No traffic during the run |
| USB | A gamepad and a Logitech receiver (mouse and keyboard) plugged in throughout |
| Logging | `enable_logging=uart` throughout |

Each row is one run. Devices were added in the order below, so each row differs from the one above by one thing.

## How the numbers are calculated

The frame budget is 20 ms. The audio buffer holds 4096 samples, which is 92 ms; 800 samples is about 18 ms.

The perf build prints one summary line per 10 second window. A game does not behave like the steady synthetic
workloads above, so only *complete windows after the game has started* are used: windows after `AUTOSTART: Done.` that
are a regular 10 s window with at least 420 frames (this drops the boot and warp-speed loading windows). From those:

| Column | Meaning |
| --- | --- |
| **Idle** | Emulation time per frame in the lightest such window. The game is sitting on a quiet screen, so this is almost pure overhead from the configured devices. |
| **Busy** | Mean emulation time per frame over the five busiest windows. This is the game's demanding stretch (loading from the main menu to the start). |
| **Budget** | Busy as a share of the 20 ms frame. |
| **Typical worst frame** | The median of the worst single frame in each of those five windows. Over 20 ms means the frame misses its vblank. |
| **Buffer low** | The lowest audio buffer level seen in any game window (out of 4096). |
| **Empty frames** | Frames where the audio buffer was empty, i.e. an audible drop-out (`audio.fill_zero`, summed over the game windows). |

Verdicts, from the last two columns:

- **Comfortable:** buffer never below about 2000, no empty frames, worst frames well under 20 ms.
- **Workable:** as above, but worst frames reach the 20 ms budget.
- **On the edge:** the buffer drops below about 800 (18 ms of audio) or a drop-out occurs. It can glitch.
- **Glitches:** repeated empty-buffer frames.

## Adding devices one at a time

All times are ms per frame. Δ is the change from the row above.

| Row | Configuration | Idle | Δ idle | Busy | Δ busy | Budget | Typical worst frame | Buffer low | Empty frames | Verdict |
| --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- |
| A | Drive 8 only, networking off | 4.8 | | 12.0 | | 60% | 15.1 | 2536 | 0 | Comfortable |
| B | + Wi-Fi networking | 4.7 | −0.1 | 11.2 | −0.8 | 56% | 14.1 | 2288 | 0 | Comfortable |
| C | + drive 9 (1541-II) | 5.5 | **+0.8** | 12.6 | +1.4 | 63% | 16.6 | 2528 | 0 | Comfortable |
| D | + drive 10 (CMD HD) | 6.3 | **+0.8** | 14.3 | +1.7 | 71% | 19.8 | 2120 | 0 | Workable |
| E | + drive 11 (FD-4000) | 8.3 | **+2.1** | 16.8 | +2.5 | 84% | 24.4 | 16 | 0 | On the edge |
| F | + IDE64 image attached | 8.2 | −0.1 | 16.6 | −0.2 | 83% | 24.4 | 0 | 1 | On the edge |
| G | + web UI enabled | 8.3 | +0.1 | 16.5 | −0.1 | 82% | 24.2 | 28 | 0 | On the edge |
| H | as G, game started from the REU | 8.3 | 0.0 | 16.7 | +0.2 | 83% | 24.2 | 28 | 0 | On the edge |

Two more runs of the full four-drive setup, for comparison:

| Row | Configuration | Idle | Busy | Budget | Typical worst frame | Buffer low | Empty frames | Verdict |
| --- | --- | --- | --- | --- | --- | --- | --- | --- |
| I | 4 drives + IDE64, networking on (the first run, which glitched) | 9.0 | 18.7 | 94% | 29.0 | 0 | 17 | Glitches |
| J | 4 drives + IDE64, networking off | 8.7 | 17.9 | 90% | 26.7 | 756 | 0 | On the edge |

## Cost of each device

Use the **idle** column to quote what a device costs. It repeated to about 0.1 ms across runs (8.3, 8.2, 8.3 and 8.3 ms
for rows E to H), whereas the busy column varied by about 2 ms between runs of the same setup (rows F and I).

| Device (Pi Zero W) | Cost at rest | Cost under load | Share of a 20 ms frame at rest |
| --- | --- | --- | --- |
| Drive 9, 1541-II | +0.8 ms | +1.4 ms | 4% |
| Drive 10, CMD HD | +0.8 ms | +1.7 ms | 4% |
| Drive 11, FD-4000 | +2.1 ms | +2.5 ms | 10% |
| **All three extra drives** | **+3.6 ms** | **+5.6 ms** | **18%** |

The cost is paid whether or not the game touches the drives: row H starts the game from the REU and costs the same as the
disk start in row G.

## What made no measurable difference

| Item | Compared | Change at rest | Change under load | Notes |
| --- | --- | --- | --- | --- |
| Wi-Fi networking, joined with an IP address | A → B (drive 8 only) | −0.1 ms | −0.8 ms | Within noise. On the full four-drive setup (I vs J) busy differed by 0.8 ms, also within noise, but see the note below |
| Web UI enabled and listening | F → G | +0.1 ms | −0.1 ms | Idle only. The logs do not show any page being loaded during the run, so this says nothing about the cost while it is in use |
| IDE64 image attached | E → F | −0.1 ms | −0.2 ms | The `IDE64Image1` line in `vice.ini` |
| Starting the game from the REU instead of the disk | G → H | 0.0 ms | +0.2 ms | Same cost, so the game's own disk access is not what costs time |
| JiffyDOS 1541-II and 1581 drive ROMs, compared with the stock ROMs | K → L | −0.1 ms | −0.1 ms | Measured with trap idle on units 9 to 11 |

Two smaller effects of networking are worth knowing:

- **A short stall when Wi-Fi joins.** In 7 of the 8 networking runs, one or two `Buffer drained` warnings were logged
  when Wi-Fi associated, about 10 s after boot and before the game started. That is an emulator stall of roughly
  80 ms or more. It also shows on a Pi 3B+, and it is harmless at boot, but a reconnect during play would likely do the
  same.
- **On a setup that is already at the limit, a small cost can tip it.** Rows I and J are the same four-drive setup with
  networking on and off. The average moved by only 0.8 ms, but the outcome went from 17 empty-buffer frames to none.

## Mitigation tried: drive idle method (trap idle)

By default VICE runs every enabled drive's CPU for the whole frame, even while the drive is only waiting in its DOS idle
loop. VICE has a per-drive **trap idle** mode (`Drive<unit>IdleMethod=2` in `vice.ini`) that jumps the drive's clock
straight to its next interrupt or alarm when it reaches the idle loop, so the wait is skipped without changing what the
drive does. It covers the 1541 / 1541-II / 1571 family, the 1581 and the FD-2000 / FD-4000, but has no trap for the CMD HD,
and a drive's trap is only installed if its ROM has the stock idle loop at the expected address. BMC64 does not expose the
setting in its menu.

Row K is row G (four drives + IDE64, networking on, web UI on) with trap idle enabled for units 9 to 11 only. Row L is
row K with the JiffyDOS 1541-II and 1581 drive ROMs removed, so the stock ROMs were loaded instead (the C64 KERNAL stayed
JiffyDOS):

| Row | Configuration | Idle | Busy | Budget | Typical worst frame | Buffer low | Empty frames |
| --- | --- | --- | --- | --- | --- | --- | --- |
| B | Drive 8 only, networking on (reference) | 4.7 | 11.2 | 56% | 14.1 | 2288 | 0 |
| G | 4 drives + IDE64 + web UI, no idle method | 8.3 | 16.5 | 82% | 24.2 | 28 | 0 |
| **K** | **as G, trap idle on units 9 to 11** | **6.9** | **16.1** | **81%** | **24.1** | **0** | **1** |
| **L** | **as K, stock drive ROMs instead of JiffyDOS** | **6.8** | **16.0** | **80%** | **24.2** | **28** | **0** |

- **At rest it helped:** 8.3 → 6.9 ms, a saving of 1.4 ms, which is about 40% of the 3.6 ms the three extra drives add
  over row B. That figure repeats well, so it is probably real.
- **Under load it did not help:** busy moved by −0.4 ms, which is inside the run-to-run noise, and the worst frames, the
  audio buffer low point and the drop-out count are the same as row G. The setup is still on the edge.
- **The drive ROM made no difference.** Row L, with stock ROMs, is the same as row K within noise (6.8 vs 6.9 ms idle,
  16.0 vs 16.1 ms busy). So the JiffyDOS drive ROMs were not what limited the saving, and they do not cost anything
  measurable on their own.
- **Not known:** which of the three drives gave the saving. The CMD HD has no trap. The FD-4000 is the likeliest source,
  because its ROM was the stock one in both runs and it is the drive with the largest idle cost. It was not tested
  drive by drive.
- **So this is a partial mitigation.** It does not bring four drives back to a comfortable level, and it leaves the
  roughly 2 ms that the drives add only while the game is busy.

## The four-drive setup sits on the edge

Rows F, G and H are the same setup as row I except for the web UI and game mode, yet row I glitched badly (17 empty
frames, 94% of the budget) and F, G and H barely did (0 to 1 empty frames, about 83%). The busy figure differed by
about 2 ms between runs of the same setup. The game is played by hand, so each run reaches its heavy stretch differently,
and around 16 to 18 ms per frame the result flips between "fine" and "glitching". That is why the verdict for all
four-drive rows is *on the edge*, not *comfortable*, even when one run had no empty frames.

## Other things tried (not measured with this build)

- **BMC64 4.2** on the same Pi Zero W, one drive, no networking: the same game played through every loading screen with
  no glitches. Only a log was captured (start-up drain warnings only), so there is no frame-time data.
- **SID Engine set to Fast (FastSID)** on the four-drive setup with Wi-Fi: the drains continued and the sound was worse,
  so the SID engine was not the cause.
- **Networking off** on the four-drive setup: much better by ear before the perf build existed; row J is the measured
  version.

## Not tested yet

- Turning **true drive emulation off per drive** for units 9 to 11 while leaving their types set. VICE only runs a drive's
  CPU when true emulation is on for it, so this may bring the cost back toward row A.
- Drive **idle method** beyond rows K and L: on each extra drive separately, and on drive 8.
  (`Drive<unit>IdleMethod` defaults to no idling and is not exposed in the BMC64 menu.)
- Dual SID, the CRT shader, 1080p output, mouse emulation and GPIO scanning (the last two were enabled on the SD card
  but were not varied).
- Other drive types, other games, and any board other than the Pi Zero W.

## Caveats

- **One run per row.** The only repeat is the four-drive setup (rows F and I), which differed by about 2 ms busy and
  by 17 empty frames. Treat differences under about 2 ms in the busy column as noise.
- **Played by hand.** Timing and route through the game vary between runs, which is why idle (a quiet screen) is the
  better number for device cost.
- **The web UI was off in every run except G, H, K and L**, because of the absence of a `Web UI listening` line in the log.
- **Drive types are specific to this card** (1541-II, CMD HD, FD-4000). Other types may cost differently.
- The summary figures were computed from the `[perf]` lines with a one-off script following the rules above.
  `perf_report.py` targets steady synthetic workloads and does not do this.
- Only regular 10 s windows with 420 or more frames are used, so a window with a long pause or a stall could be left
  out.
