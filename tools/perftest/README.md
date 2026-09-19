# BMC64 performance test tools

Workloads and a report tool for the frame-budget / audio performance suite. The
design, metrics and procedure are in
[docs/architecture/PERFORMANCE_TEST_PLAN.md](../../docs/architecture/PERFORMANCE_TEST_PLAN.md).

Results from every run so far, on all boards, are in [RESULTS.md](RESULTS.md).

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

## Collecting and reporting

Build BMC64 with `--perf-stats`, enable logging in `cmdline.txt`, start a
workload, leave it for a minute or more, then:

    ./perf_report.py bmc64.log --label "pi3 c64 stress" --json pi3_c64_stress_vice-3.3.json
    ./perf_report.py bmc64.log --baseline pi3_c64_stress_vice-3.3.json

Or read the serial console directly:

    stty -F /dev/ttyUSB0 115200 raw -echo
    ./perf_report.py /dev/ttyUSB0 --skip 1 --windows 6 --timeout 120

Exit status: 0 pass, 1 fail or regression, 2 no data, 3 invalid run (Pi was
throttled or under-volted). See `./perf_report.py --help`.

The tool reads the network state from the log and prints it (for example
`network: Wi-Fi connected, IP after 11 s`), and counts network events inside the
measured windows. A log holding several boots is split per boot; the last usable
boot is analysed unless you pass `--boot N`. Make sure the network has an address
before starting a workload: a link that keeps retrying DHCP was seen to add 3 to
7 ms to individual frames.
