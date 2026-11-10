# BMC64 Patches

The following patches are applied by `make_all.sh` in this order while
preparing the Circle and circle-stdlib dependencies. They carry BMC64-specific
changes that are not part of the checked-out upstream revisions.

Patches are applied to the submodule revisions pinned by circle-stdlib v21
(Circle 51.1.1, tag `Step51.1.1`).

Unless noted, every patch is applied on every build. `circle_kasan_patch.diff`
is applied only with `--kasan`; `circle_diskio_stats_patch.diff` only with
`--io-stats`.

| Patch | Description |
| --- | --- |
| `circle_newlib_patch.diff` | Extends Circle's newlib glue with RAM-buffered FatFs file I/O and directory-entry compatibility updates (adds `d_type` to `struct dirent`). |
| `circle_patch.diff` | Applies core Circle compatibility changes, including disabling `BUG_ON` assertions, enabling FatFs `FF_USE_FASTSEEK` (used by `new_io.cpp` for streamed large disk images), and enabling FatFs `FF_MULTI_PARTITION` with a BMC64 extension for mounting a FAT volume at a forced start sector. |
| `circle_tcp_receive_window_patch.diff` | Adds `CTCPConnection::SetReceiveWindow()` so BMC64 can choose the TCP receive window at run time (default 10 segments, up to 44, the most that fits without window scaling). BMC64 uses 44 on Wi-Fi, where the larger window nearly doubles web UI upload speed, and keeps 10 on Ethernet, where the Pi 3's USB Ethernet drops the end of larger bursts. |
| `circle_kasan_patch.diff` | Adjusts KASAN heap allocation, reallocation, and address validation for BMC64's supported Raspberry Pi targets. This patch is applied only when `make_all.sh` is run with `--kasan`. |
| `circle_diskio_stats_patch.diff` | Adds BMC64 storage I/O instrumentation hooks (`io_stats_disk_read`/`io_stats_disk_write`) to the FatFs `diskio.cpp` glue so `disk_read`/`disk_write` counts, multi-sector sizes and latency can be measured. This patch is applied only when `make_all.sh` is run with `--io-stats` |
