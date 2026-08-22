# BMC64 Patches

The following patches are applied by `make_all.sh` in this order while
preparing the Circle and circle-stdlib dependencies. They carry BMC64-specific
fixes and device support that are not part of the checked-out upstream
revisions.

| Patch | Description |
| --- | --- |
| `circle_newlib_patch.diff` | Extends Circle's newlib glue with RAM-buffered FatFs file I/O and directory-entry compatibility updates. |
| `circle_patch.diff` | Applies core Circle compatibility changes, including disabling `BUG_ON` assertions and accommodating a USB keyboard HID descriptor variant. |
| `circle_8bitdo_keyboard_patch.diff` | Adds support for the 8BitDo Retro Keyboard, including its report translation and USB device-factory recognition. |
| `circle_8bitdo_gamepad_patch.diff` | Adds Circle USB gamepad drivers for 8BitDo controllers, including Pro and XInput modes, rumble support, and device-factory recognition. |
| `circle_usb_descriptor_patch.diff` | Makes USB string-descriptor reads use the descriptor's reported length, avoiding fixed-size buffer assumptions. |
| `circle_xbox360_gamepad_patch.diff` | Adds a Circle USB driver for the Xbox 360 wireless PC receiver, including LED, rumble, report decoding, and device-factory recognition. |
| `circle_tcpconnection_patch.diff` | Updates Circle TCP connection for BMC64's networking and stop network stalls. |
| `circle_ethernet_patch.diff` | Tracks Ethernet PHY link state for LAN7800 and SMSC951x adapters, preventing receives while the link is down. |
| `circle_kasan_patch.diff` | Adjusts KASAN heap allocation, reallocation, and address validation for BMC64's supported Raspberry Pi targets. This patch is applied only when `make_all.sh` is run with `--kasan`. |

---

**Note:** `circle_stdlib_patch.diff` configures circle-stdlib for BMC64 and extends the
Circle glue interface used by the application. It is deprecated but retained here. It was commented out of `make_all.sh` in the Circle 51 port for v5.0.0. Should be removed in the future.
