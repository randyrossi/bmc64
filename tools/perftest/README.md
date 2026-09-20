# BMC64 performance test tools

Workloads and a report tool for measuring how much of each video frame's time budget the emulator uses, whether any
vertical blank is missed, and how the audio buffer behaves. They are used to compare boards, builds and settings, and to
find out what a device or option costs.

Results from every run so far, on all boards, are in [RESULTS.md](RESULTS.md).

The measurement is built into BMC64 but compiled out unless you ask for it (`--perf-stats`, below), so a normal build
carries none of it. It covers the VICE-based machines only, not Plus4Emu.

## Contents

- [BMC64 performance test tools](#bmc64-performance-test-tools)
  - [Contents](#contents)
  - [Workloads](#workloads)
  - [Collecting a log](#collecting-a-log)
  - [Report format](#report-format)
    - [Window fields](#window-fields)
    - [Per-frame statistics](#per-frame-statistics)
    - [Other fields](#other-fields)
  - [What the metrics mean](#what-the-metrics-mean)
  - [The report tool](#the-report-tool)
  - [Measuring a real program](#measuring-a-real-program)

## Workloads

`perf_workload.asm` builds five C64 programs (needs `xa`, `sudo apt install xa65`):

    make

| PRG | Stresses |
| --- | --- |
| `perf_idle.prg` | baseline |
| `perf_vic.prg` | sprites + per-line raster register writes |
| `perf_sid.prg` | reSID, 3 voices with filter |
| `perf_sid2.prg` | reSID on two SIDs (enable dual SID, SID 2 at `$D420`) |
| `perf_stress.prg` | VIC + SID together |

The built `.prg` files are committed so `xa` is only needed to change them.
`build_sdcard.sh --perf-stats` copies them into `prg/` on the staged SD card; if
you build another way, copy them to any folder the file browser lists. They are
loud: turn the volume down.

Each program runs forever and does the same emulated work every time, with no input, media or dependence on the host,
so the numbers can be compared between runs, builds and emulator cores.

## Collecting a log

The report is printed as text on the Pi's serial port, so you need a serial adapter to capture it. File logging can be used but is not recommended as file logging will influence the performance metrics.

1. **Build BMC64 with the instrumentation.** Pass `--perf-stats`, for example
   `./build_sdcard.sh pi0 --machine c64 --perf-stats`. If you use the individual scripts, pass it to **both**
   `make_all.sh` and `make_machines.sh` (see [BUILDING.md](../../docs/BUILDING.md)).
2. **Turn on UART logging.** Use the top-level `Logging` menu item and choose `UART` (it asks for a reboot), or put
   `enable_logging=uart` in the machine's `cmdline.txt`. See
   [Logging](../../docs/USER_GUIDE.md#logging). Prefer `UART` for measuring: the `File` mode writes to the SD card too
   and can slow emulation.
3. **Connect a USB-to-TTL serial adapter** (an FT232RL board set to 3.3 V) to the Pi's GPIO header and to your PC. The
   wiring is in [UART Debugging](../../docs/USER_GUIDE.md#uart-debugging): GND to pin 6, the adapter's RXD to pin 8
   (GPIO14/TXD) and its TXD to pin 10 (GPIO15/RXD). Do not connect the adapter's power pin.
4. **Open a terminal on the adapter before booting the Pi.** For example `picocom -b 115200 /dev/ttyUSB0` (115200 baud,
   8N1, no flow control; leave with Ctrl-A Ctrl-X). Capturing from boot means the tool can read the start-up lines, and
   the network state comes from them.
5. **Boot, wait for the network to settle, and start a workload** from the file browser (or your own program). Leave it
   running for a minute or more: each 10 second window prints one `[perf]` line, and the tool ignores the first window
   or more while the workload starts. Do not open menus or change settings during the run. A paused emulator produces
   partial windows.
6. **Save the text.** Select everything from the boot lines down in the terminal window, copy it, and paste it into a
   text file such as `bmc64.log`. Check that each `[perf]` line is on one line.
7. **Process the file with the report tool:**

       ./perf_report.py bmc64.log --label "pi3 c64 stress" --json pi3_c64_stress_vice-3.3.json
       ./perf_report.py bmc64.log --baseline pi3_c64_stress_vice-3.3.json

   The first saves a summary you can use as a baseline. The second compares a later run against it.

The tool can also read the serial device directly instead of a saved file. Close your terminal first, so that only one
program is reading the port:

    stty -F /dev/ttyUSB0 115200 raw -echo
    ./perf_report.py /dev/ttyUSB0 --skip 1 --windows 6 --timeout 120

Practical points:

- **Start each capture from a cold boot,** so the log has one clean boot. A log holding several boots is split per boot,
  and the last usable boot is analysed unless you pass `--boot N`.
- **Make sure the network has an address before starting a workload.** A link that keeps retrying DHCP was seen to add 3
  to 7 ms to individual frames. The tool prints the network state it finds in the log.
- **On the Pi Zero the report is sent from inside the emulator's own core,** in 32 byte chunks 20 ms apart, and the
  frames that carry a chunk are left out of the measurement. This is why a healthy Pi Zero window shows about 471 frames
  instead of 500.
- **Anything else logged while a report is being sent splits that line.** The tool then ignores that window and prints
  `note: ignored N malformed [perf] line(s)`. Avoid causing other log output during a run.

## Report format

With `--perf-stats`, BMC64 prints one line per 10 second window:

    [perf] {"v":1,"core":"vice-3.3","bmc64":"5.1.6","pi":1,"win":7,...}

The part after `[perf] ` is a single line of JSON, about 900 bytes. Here is a real one from a Pi Zero W running
`perf_stress.prg`, laid out over several lines:

```
{
  "v": 1, "core": "vice-3.3", "bmc64": "5.1.6", "pi": 1,
  "win": 7, "dur_ms": 10000, "nominal_us": 20000,
  "frames": 471, "gaps": 0, "missed": 0, "dropped": 0,
  "busy":   { "n": 471, "min": 12040, "avg": 12246, "max": 12790 },
  "emu":    { "n": 471, "min": 11562, "avg": 11769, "max": 11902 },
  "post":   { "n": 471, "min": 44,    "avg": 66,    "max": 118 },
  "ready":  { "n": 471, "min": 347,   "avg": 393,   "max": 886 },
  "swap":   { "n": 471, "min": 6418,  "avg": 7734,  "max": 8046 },
  "tail":   { "n": 471, "min": 9,     "avg": 16,    "max": 24 },
  "period": { "n": 413, "min": 19726, "avg": 20002, "max": 20324 },
  "busy_hist": { "edges_pct": [25, 50, 60, 70, 80, 90, 100],
                 "n": [0, 0, 0, 471, 0, 0, 0, 0] },
  "audio": {
    "fill":  { "n": 471, "min": 2976, "avg": 3467, "max": 3672 },
    "fill_zero": 0, "fill_cap": 4096,
    "write": { "n": 500, "min": 24, "avg": 40, "max": 62 },
    "full_waits": 0, "silent": 0
  },
  "sid": { "wait": { "n": 0, "min": 0, "avg": 0, "max": 0 } },
  "worst": { "at_ms": 1411, "busy": 12790, "emu": 11830, "post": 64,
             "ready": 886, "tail": 10 },
  "env": { "fresh": 1, "temp_mc": 50843, "throttled": 0, "arm_hz": 1000000000 }
}
```

All times are in **microseconds**, from the 1 MHz system timer. A window closes at the first frame after 10 seconds.

### Window fields

| Field | Meaning |
| --- | --- |
| `v` | Report format version, currently 1. The tool rejects other versions. |
| `core` | The emulator core that produced the numbers (`vice-3.3`). Results from different cores are never mixed. |
| `bmc64` | BMC64 version. |
| `pi` | Board model number as Circle reports it (1 on a Pi Zero W). |
| `win` | Window number, counted from the end of boot, starting at 1. |
| `dur_ms` | How long the window was. Normally about 10000, but a window that overlaps boot, warp-speed loading or a pause can be longer or hold fewer frames. |
| `nominal_us` | The machine's frame period: 20000 for PAL at 50 Hz. Every busy figure is judged against it. |
| `frames` | Frames measured in the window. Only frames presented with vertical-blank sync count. Warp frames, the frame that ends an emulator pause and (on the Pi Zero) frames that carried part of the report are left out. |
| `gaps` | Frames skipped because the emulator had been paused for half a second or more (menu open, snapshot, and so on). |
| `missed` | Vertical blanks missed: frames whose start-to-start period was more than 1.5 times `nominal_us`. |
| `dropped` | Reports never sent because the previous one was still being sent. A running total since boot. |

### Per-frame statistics

Each of these is `{"n", "min", "avg", "max"}`: `n` is how many frames were measured, the rest are microseconds.

| Field | Meaning |
| --- | --- |
| `emu` | Time spent emulating one frame: from the end of the previous frame's post-frame step to the start of this one. This is elapsed time on the core, so it also includes any interrupt or task that ran on the same core meanwhile. |
| `post` | The post-frame step before the picture is handed to the display: menu and on-screen display drawing, and the scheduler's `circle_yield()`, which is where background tasks such as networking get to run. |
| `ready` | Preparing and copying the finished frame for the display. |
| `swap` | The display swap, which normally blocks until vertical blank. This is the **slack**: time the emulator was waiting for the next frame. It is not part of `busy`. |
| `tail` | Housekeeping after the swap, such as input and GPIO scanning. |
| `busy` | `emu + post + ready + tail`: everything except the vblank wait. This is the time the frame actually needed. |
| `period` | Time between the starts of consecutive frames, which should be `nominal_us`. Recorded only when both frames around it were locked to vblank, so `n` is smaller than `frames`. |

A frame is timed like this:

```
 previous frame ends                                                        this frame ends
        |<---- emu ---->|<- post ->|<- ready ->|<------- swap ------->|<- tail ->|
                                               (waits for vblank = slack)
        |<----------------------- period, about nominal_us ------------------------->|
        busy = emu + post + ready + tail
```

### Other fields

| Field | Meaning |
| --- | --- |
| `busy_hist` | Frames counted by `busy` as a percentage of `nominal_us`. `edges_pct` gives the upper edges, so `n` has eight buckets: below 25%, 50%, 60%, 70%, 80%, 90%, 100%, and a last bucket for 100% or more. Frames in that last bucket could not finish inside their period. |
| `audio.fill` | The level of the audio buffer in samples, read once per frame right after vertical blank, which is when it is lowest. |
| `audio.fill_cap` | The buffer's capacity in samples. 4096 samples is 92 ms of sound at 44.1 kHz. |
| `audio.fill_zero` | Frames on which the buffer was empty, which is an audible drop-out. |
| `audio.write` | How long each hand-over of audio to the driver took. |
| `audio.full_waits` | Times a hand-over of audio found the buffer already full and had to wait for the GPU to play some out. The emulator was running ahead of the sound. |
| `audio.silent` | Silent packets the driver had to send because no audio was ready (an underrun). |
| `sid.wait` | Dual SID only, on the Pi 2 and Pi 3: how long the emulator core blocked waiting for the helper core that computes SID 2. `n` is 0 otherwise. |
| `worst` | The slowest frame in the window by `busy`: `at_ms` is when it began, in milliseconds into the window, and the rest are its stage times. Use it to see which stage a stall was in and what else happened at that moment. |
| `env.temp_mc` | SoC temperature in millidegrees Celsius. |
| `env.throttled` | The firmware's throttle bits, 0 when clean. Under-voltage, a capped clock, throttling or the soft temperature limit, now or since boot, make a run invalid. |
| `env.arm_hz` | The ARM clock frequency. |
| `env.fresh` | 1 if this report asked the firmware for `temp_mc`, `throttled` and `arm_hz`; otherwise they are repeated from the last query. The firmware also services the frame present, so it is asked only every sixth report and flagged, so that a slow frame right after a query can be spotted. |

SID computation is not measured separately, so that the numbers do not depend on emulator internals. To find its
cost, compare a SID workload against the idle workload.

## What the metrics mean

- **Utilisation** is `busy` as a share of `nominal_us`. At 61%, 39% of each frame is spare. The average alone is not
  enough: a frame whose `busy` reaches 100% misses its vertical blank, so watch `busy.max`, `busy_hist` and `missed`.
- **`swap` is the headroom.** The emulator waits there for the next frame, so it shrinks as load rises.
- **`emu` is what the emulator's own work costs.** It changes with the program, the SID mode and the devices configured.
  Because it is elapsed time, an interrupt handler or background task on the same core inflates it. On the single-core
  Pi Zero that includes the USB and network stacks.
- **`post` shows background tasks.** Its maximum is where a network task running inside `circle_yield()` shows up.
- **The audio buffer shows what you would hear.** `audio.fill.min` is how close the buffer came to running out.
  `fill_zero` above 0 means an audible drop-out, and `silent` above 0 means an underrun.
- **Compare a board only with itself.** BMC64 uses the cheaper `fast` SID sampling below the Pi 3, and the Pi Zero uses a
  different reSID library (teensy-resid) and is mono, so busy percentages are not comparable across boards.

## The report tool

`perf_report.py` reads `[perf]` lines from a log file, from stdin (`-`) or from a serial device, and prints a summary.
It needs only Python 3.

| Option | Meaning |
| --- | --- |
| `--skip N` | Ignore the first N windows while the workload starts (default 1). |
| `--windows N` | Keep N windows after the skipped ones, and stop reading once a boot has that many (default: all). Useful with a serial device. |
| `--boot N` | Which boot to analyse when the log holds several (default: the last one with more than `--skip` windows). |
| `--timeout S` | Stop reading after S seconds. For serial devices. |
| `--label TEXT` | A label shown in the summary. |
| `--json FILE` | Write the summary as JSON, for use as a baseline. |
| `--baseline FILE` | Compare against a saved summary. |
| `--tolerance PCT` | How much a figure may rise against the baseline before it counts as a regression (default 5). |
| `--allow-miss` | Do not fail on missed vertical blanks. For workloads that are meant to be over budget. |
| `--ignore-throttle` | Do not mark the run invalid because of firmware throttle bits. |

When more than two windows are kept, it drops windows in which the emulator was paused (`gaps` above 0) and windows with
fewer than 80% of the median frame count, and says so. A real summary, from a Pi Zero W running `perf_stress.prg` with `--skip 3`:

```
== pi0 c64 stress
core vice-3.3 | bmc64 5.1.6 | Pi model 1 | 24 window(s), 11313 frames | 1000 MHz | max 55.1 C
frame period 20000 us   busy avg 12259 us (61.3%)   busy max 13023 us (65.1%)
  per frame avg us   emulate 11781 | post/OSD 70 | present 392 | vblank wait 7721 | tail 14
  busy histogram     <25%:0  <50%:0  <60%:0  <70%:11313  <80%:0  <90%:0  <100%:0  >=100%:0
missed vblank 0   period max 20871 us   over-budget frames 0
audio fill min/avg 2948/3448 of 4096 samples   empty 0   silent pkts 0   write avg/max 40/64 us
network: Wi-Fi connected, IP after 11 s; NTP ok
RESULT: PASS
```

Line by line: the board, build and clock; the frame budget and busy average and maximum; the average time in each frame
stage (`present` is `ready`, `vblank wait` is `swap`); the busy histogram; missed vertical blanks, the longest period and
frames over budget; the audio buffer; and the network state.

**Pass and fail.** The run fails if there is any missed vertical blank (unless `--allow-miss`), any frame with an empty
audio buffer, or any silent audio packet. It is reported as **invalid** if the firmware throttle bits show
under-voltage, a capped clock, throttling or the soft temperature limit, because the numbers are then not comparable.

**Baselines.** With `--baseline`, it flags a regression if `busy` average or maximum, utilisation, SID 2 wait or audio
write time rise by more than the tolerance, or if any of these counts grows: missed vertical blanks, over-budget frames,
empty audio frames, silent packets. It also warns if the board, frame period or network state differs from the baseline.

**Network.** It reads the network state from the same log (for example `Wi-Fi connected, IP after 11 s`, `Wi-Fi
associated, NO DHCP lease`, or `Wi-Fi tried to associate but never connected`) and counts network events inside the
measured windows. SSIDs, MAC addresses and IP addresses are not put in a saved JSON summary.

**Slow frames.** If any window's slowest frame did not fit in a frame period, it lists which stage was slow, how far
into the window it began, and whether it followed a firmware query (`env.fresh`).

**Exit status:** 0 pass, 1 fail or regression, 2 no data, 3 invalid run.

## Measuring a real program

The synthetic workloads are steady, which is what `perf_report.py` is designed for. A real game or demo is not, and
comparing devices or settings with it needs a different reading of the same lines:

- Use only the windows after the program has started, and prefer a plain reading of the raw `[perf]` lines to the tool's
  pass or fail, which assumes a steady workload.
- The idle emulation time (the lightest window, with the program on a quiet screen) is a much more repeatable measure of
  what a device costs than the busy time. In [RESULTS.md](RESULTS.md) it repeated to about 0.1 ms, while the busy time
  varied by about 2 ms between runs of the same setup.
- A game that prints many log lines during the run, such as `Sound: Warning - Buffer drained`, splits reports and loses
  those windows. The game runs in [RESULTS.md](RESULTS.md) re-joined the split lines with a small one-off script.

[RESULTS.md](RESULTS.md) has a worked example: the cost of each emulated drive, networking and the web UI on a Pi Zero W
running a real game.
