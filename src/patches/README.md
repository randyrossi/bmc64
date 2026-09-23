# BMC64 Patches

The following patches are applied by `make_all.sh` in this order while
preparing the Circle and circle-stdlib dependencies. They carry BMC64-specific
changes that are not part of the checked-out upstream revisions.

Before patching, `make_all.sh` checks out Circle 51.1 (tag `Step51.1`) in
`third_party/circle-stdlib/libs/circle` if it is not already there, because
circle-stdlib's own submodule pointer still references an older Circle.

Unless noted, every patch is applied on every build. `circle_kasan_patch.diff`
is applied only with `--kasan`; `circle_diskio_stats_patch.diff` only with
`--io-stats`.

| Patch | Description |
| --- | --- |
| `circle_newlib_patch.diff` | Extends Circle's newlib glue with RAM-buffered FatFs file I/O and directory-entry compatibility updates (adds `d_type` to `struct dirent`). |
| `circle_patch.diff` | Applies core Circle compatibility changes, including disabling `BUG_ON` assertions, enabling FatFs `FF_USE_FASTSEEK` (used by `new_io.cpp` for streamed large disk images), and enabling FatFs `FF_MULTI_PARTITION` with a BMC64 extension for mounting a FAT volume at a forced start sector. |
| `circle_8bitdo_gamepad_patch.diff` | Selects the vendor XInput interface (class `0xFF`/`0x5D`/`0x01`) and fails configuration if it is missing. |
| `circle_kasan_patch.diff` | Adjusts KASAN heap allocation, reallocation, and address validation for BMC64's supported Raspberry Pi targets. This patch is applied only when `make_all.sh` is run with `--kasan`. |
| `circle_diskio_stats_patch.diff` | Adds BMC64 storage I/O instrumentation hooks (`io_stats_disk_read`/`io_stats_disk_write`) to the FatFs `diskio.cpp` glue so `disk_read`/`disk_write` counts, multi-sector sizes and latency can be measured. This patch is applied only when `make_all.sh` is run with `--io-stats` |
