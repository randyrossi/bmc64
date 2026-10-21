# Raspberry Pi 4 and 5

BMC64 runs on the Raspberry Pi 4 B and Pi 400 (`kernel8.img`, 64-bit) and
the Raspberry Pi 5, Pi 500 and Pi 500+ (`kernel_2712.img`). Every machine
is available, Plus4Emu included.

The C64 is VICE's cycle exact one (x64sc) on these boards, where the
Pi 0 to 3 run the faster x64. Its VICE settings are saved in the `[C64SC]`
section of `vice.ini`, not `[C64]`.

## Boot speed

The Pi 4 and 5 still boot quickly, but they can't match the boot speed of
the Pi 0 to 3: they take a few seconds more. This is because of how their
bootloader works. On the Pi 0 to 3 the bootloader is a small program that
loads straight from the SD card. On the Pi 4 and 5 it is stored in an
EEPROM on the board, and it sets up the memory, the PCIe bus and the SD
card before it loads BMC64. BMC64 can't make this step any shorter.

## Display

These boards have no dispmanx or OpenGL ES, so BMC64 drives the display
itself: the emulator's screen, the menu and the status bar are each a plane
of the display hardware (the HVS), scaled to their place, on the HDMI port
with a display plugged in.

- **Frame rate.** The video mode runs at the emulated machine's frame rate:
  50 or 60 Hz with the HDMI timings, and the machine's own rate with the
  custom ones (`machine_timing=pal-custom` or `ntsc-custom` with
  `cycles_per_second`), when the display has a mode near it.
- **Resolution.** The machine switch menu offers each machine at 720p and
  1080p (`hdmi_mode` 4, 19, 16 or 31; the PET at 1080p), on either HDMI port; the Pi 4 also
  offers the custom modes (`hdmi_timings`). The Pi 5 uses the mode if the
  display lists it. The integer scaling (`scaling_params`) keeps its
  whole-number scale but is made as large as fits the screen: the default
  1152x720 (3x) at 720p is 1536x960 (4x) at 1080p; sizes that aren't
  whole-number scales (the C128's VDC) grow in proportion.
- **CRT shader.** Drawn by the 3D GPU, crt-pi with its default look;
  the CRT Shader menu has Curvature, and Scaling Interpolation smooths it.
- Composite and DPI output aren't supported, and their machines.txt entries
  aren't offered in the machine switch menu.

See [LIMITATIONS_PI4_PI5.md](LIMITATIONS_PI4_PI5.md) for everything these boards
don't support.

## Sound

Over HDMI, at 48 kHz. The Pi 4 B can use its headphone jack
(`audio_out=analog` in `cmdline.txt`, or `auto` to fall back to it when
the display takes no sound); the Pi 400 and the Pi 5 family have none.

## Power

**Shut down** halts the Pi 4 B (it starts again when GPIO3 is pulled low)
and switches the Pi 400 off. The Pi 400's power key (Fn+F10) powers off
as the menu does, and starts it again. The Pi 5's
power button, and the Pi 500's power key, power off as the menu does, and
start the Pi again.

## Serial port

The log goes to GPIO14 (TX) and GPIO15 (RX) on every board, the Pi 5 and
Pi 500 included.

## Building

`./make_all.sh pi4` or `pi5`, then `./make_machines.sh pi4` or `pi5`, as
for the other boards (see [BUILDING.md](BUILDING.md)); they need the 64-bit
Arm GNU Toolchain (`aarch64-none-elf`), which `get_gnu_toolchain.sh`
fetches. The Pi 4 build also makes Circle's `armstub8-rpi4.bin`.
