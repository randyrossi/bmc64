# BMC64 Project Scope

BMC64 is a bare-metal Commodore emulator for the Raspberry Pi. This document describes the intended scope and priorities of the project and provides guidance when considering feature requests.

## Priorities and Scope

* **Performance on older Raspberry Pi models is a priority.** BMC64 aims to provide fast and responsive emulation on the older and lower-powered Raspberry Pi models it supports. This may sometimes take priority over improvements in emulation accuracy.

* **BMC64 is based on VICE 3.3.** It is not intended to track current VICE releases. Features introduced in later versions of VICE are only rarely backported to BMC64.

* **VICE defaults and settings are preferred.** Where practical, BMC64 aims to use the same settings, defaults, and behaviour as VICE rather than introducing BMC64-specific alternatives. Differences may be necessary where required by the bare-metal environment or BMC64-specific hardware support.

* **BMC64 targets specific Raspberry Pi models.** Supported models include the Raspberry Pi Zero/W, Zero 2 W, 2B, 3B, 3B+, and other supported models within these families.

* **Support for third-party hardware is best effort and community driven.** This includes custom or third-party PCBs and GPIO-connected boards, USB keyboards, gamepads and joysticks, adapters, audio and video devices, and similar peripherals. The project cannot test every device, so compatibility depends on what the maintainers and community are able to verify on real hardware. Reports of what does and doesn't work, and contributions that add or fix support, are very welcome, but a device working (or not working) is not a guarantee.

## Out of Scope

The following are not currently within the intended scope of BMC64:

* **Raspberry Pi 4 and later models.** This includes the Raspberry Pi 4B, Pi 400, Pi 5, Pi 500, and related devices in these families.

* **Support for more original Commodore peripherals.** BMC64 already lets you use a real Commodore keyboard and joysticks (via GPIO), but it is not intended to keep adding support for other original hardware, such as real disk drives, datasettes, or cartridges and other expansion port devices.

* **Keeping pace with current VICE development.** Features added to VICE after version 3.3 should not be assumed to be candidates for inclusion in BMC64.

* **Cycle-exact accuracy at the cost of performance.** Changes such as switching to the x64sc core that would noticeably slow emulation on the older supported Raspberry Pi models are not intended.

A feature being technically possible does not necessarily mean that it falls within the intended scope of the project.

## About This Scope

This document describes the current direction of BMC64. It is intended to set expectations and help determine whether proposed features are appropriate for the project.

The scope is subject to change as the project evolves. Something being listed as out of scope does not mean that it can never be reconsidered, and the absence of something from the out-of-scope list does not necessarily mean that it will be accepted.

Feature requests may be declined or closed when they do not align with the current scope, priorities, or maintainability of the project.
