# BMC64 Patches

The following patches are applied by `make_all.sh` in this order while
preparing the Circle and circle-stdlib dependencies. They carry BMC64-specific
fixes and device support that are not part of the checked-out upstream
revisions.

Unless noted, every patch is applied on every build. `circle_kasan_patch.diff`
is applied only with `--kasan`; `circle_diskio_stats_patch.diff` only with
`--io-stats`.

| Patch | Description |
| --- | --- |
| `circle_newlib_patch.diff` | Extends Circle's newlib glue with RAM-buffered FatFs file I/O and directory-entry compatibility updates (adds `d_type` to `struct dirent`). |
| `circle_patch.diff` | Applies core Circle compatibility changes, including disabling `BUG_ON` assertions, accommodating a USB keyboard HID descriptor variant, enabling FatFs `FF_USE_FASTSEEK` (used by `new_io.cpp` for streamed large disk images), and enabling FatFs `FF_MULTI_PARTITION` with a BMC64 extension for mounting a FAT volume at a forced start sector. |
| `circle_8bitdo_keyboard_patch.diff` | Adds support for the 8BitDo Retro Keyboard, including its report translation and USB device-factory recognition. Also skips binding a driver to the Keyrah V3's second boot-keyboard interface (KBD2), as an experiment to reduce the number of simultaneous periodic USB endpoints the Keyrah asks the DWHCI controller to service - see keyrah-v3-status-and-plan.md. |
| `circle_8bitdo_gamepad_patch.diff` | Adds Circle USB gamepad drivers for 8BitDo controllers, including Pro and XInput modes, rumble support, and device-factory recognition. |
| `circle_usb_descriptor_patch.diff` | Makes USB string-descriptor reads use the descriptor's reported length, avoiding fixed-size buffer assumptions. |
| `circle_xbox360_gamepad_patch.diff` | Adds a Circle USB driver for the Xbox 360 wireless PC receiver, including LED, rumble, report decoding, and device-factory recognition. |
| `circle_tcpconnection_patch.diff` | Updates Circle TCP connection for BMC64's networking and stop network stalls. |
| `circle_ethernet_patch.diff` | Tracks Ethernet PHY link state for LAN7800 and SMSC951x adapters, preventing receives while the link is down. |
| `circle_transfer_error_patch.diff` | Stops `CUSBHIDDevice::CompletionRoutine()` from permanently ending polling of a HID report endpoint after a single non-overrun USB transfer error. |
| `circle_dwhci_channel_patch.diff` | Fixes a silent out-of-bounds write in `CDWHCIDevice::SOFInterruptHandler()` when no DWHCI channel is free for a due transaction. Re-queues the transaction for the next frame instead. |
| `circle_dwhci_periodic_split_patch.diff` | Adds `USE_USB_FIQ_PERIODIC_SPLIT_COMPATIBILITY` (enabled) to `sysconfig.h`: under `USE_USB_FIQ`, widens the delay between a periodic split transaction's Start-Split ACK and its first Complete-Split attempt from 1 microframe back to 2, matching the non-FIQ timing. |
| `circle_kasan_patch.diff` | Adjusts KASAN heap allocation, reallocation, and address validation for BMC64's supported Raspberry Pi targets. This patch is applied only when `make_all.sh` is run with `--kasan`. |
| `circle_diskio_stats_patch.diff` | Adds BMC64 storage I/O instrumentation hooks (`io_stats_disk_read`/`io_stats_disk_write`) to the FatFs `diskio.cpp` glue so `disk_read`/`disk_write` counts, multi-sector sizes and latency can be measured. This patch is applied only when `make_all.sh` is run with `--io-stats` |

---

**Note:** `circle_stdlib_patch.diff` configures circle-stdlib for BMC64 and extends the
Circle glue interface used by the application. It is deprecated but retained here. It was commented out of `make_all.sh` in the Circle 51 port for v5.0.0. Should be removed in the future.
