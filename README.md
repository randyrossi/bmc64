# BMC64

BMC64 is a bare metal C64 emulator for the Raspberry Pi Zero/2/3 models with true 50hz/60hz smooth scrolling and low latency between input & video/audio. Four other Commodore machines are available as well; C128, Vic20, Plus/4 and PET.

**NOTE:** Since V3.0, all machines are all bundled into one release for all Pi models. You can switch from the 'Machine' menu.

## Features

- Quick boot time (C64 in 4.1 seconds over composite!)
- Frames are timed to vsync for true 50/60 hz smooth scrolling (no horizontal tearing!)
- Low latency between input & audio/video
- No shutdown sequence required, just power off
- High C64 compatibility thanks to VICE
- High Plus/4 compatibility thanks to Plus4Emu (Rpi3 Only)
- USB Plug and Play (PnP)
- Network support WiFi & Ethernet (e.g BBS, C64 OS networking)
- Support for 8BitDo Retro Keyboards, including the C64 model, over USB or a 2.4GHz receiver

## Limitations

- BMC64 **only** supports Pi Zero, Pi Zero 2W, Pi 2, and Pi 3 based devices. Raspberry Pi 4 and greater is **not supported**.
- USB Bluetooth gamepads are not supported directly but can be connected via the 8BitDo USB Wireless Adapter 2.
- Not all gamepads will work. Please let me know if yours doesn't.

This project uses VICE for emulation without any O/S (Linux) distribution installed on the Pi. VICE (Versatile Commodore Emulator) platform dependencies are satisfied using circle-stdlib.

For Plus/4 emulation on the Rasbperry Pi 3, a more accurate emulator using Plus4Emu is also available.

## Getting Started

- [Installation](docs/INSTALLATION.md)
- [User Guide](docs/user-guide.md)
- [Documentation](docs/README.md)
- [FAQ](docs/FAQ.md)

## Releases

- [Latest release](https://github.com/randyrossi/bmc64/releases/latest)
- [Pre-releases](https://github.com/randyrossi/bmc64/releases?q=prerelease%3Atrue&expanded=true) - Unreleased features for testing.
- [All releases (v5.0.0 and greater)](https://github.com/randyrossi/bmc64/releases)
- [Older releases (pre v5.0.0)](https://accentual.com/bmc64/downloads/)

---

- [Changelog](docs/CHANGELOG.md)

## Community

Join the official **BMC64 Discord community** to discuss BMC64 and the Commodore 64, get support, share your setup, follow development, and connect with other users.

[![Discord](https://img.shields.io/badge/Discord-Join%20the%20BMC64%20Community-5865F2?logo=discord&logoColor=white)](https://discord.gg/yXU6huQymG)

## Contributing

See the [contribution guidelines](docs/CONTRIBUTING.md).

## Project

- https://github.com/randyrossi/bmc64
