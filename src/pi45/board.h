//
// board.h
//
// What the Pi 4 and 5 need beyond the display and sound, called from
// BMC64's kernel (src/kernel.cpp) and power code (src/power/power.cpp).
//
// This file is part of BMC64's Raspberry Pi 4 and 5 support, from BMA800XL.
// See LICENSE for licensing details.
//

#ifndef BMC64_PI45_BOARD_H_
#define BMC64_PI45_BOARD_H_

// Core 0, often: on the Pi 5 the power button (the Pi 500's power key),
// and the Pi 400's power key, power off as the menu does.
void Pi45BoardPoll(void);

// Each USB keyboard report's keys: the Pi 400's keyboard sends its power
// key (Fn+F10) as Keyboard Power, 0x66.
void Pi45BoardKeys(const unsigned char keys[6]);

// Before restarting or powering off: blanks the screen and tells the
// firmware.
void Pi45BoardBeforeRestart(void);

// Runs the display at the emulated machine's frame rate: its
// CYCLES_PER_SECOND over the cycles in its PAL or NTSC frame.
void Pi45BoardMatchRate(int cycles_per_second, bool ntsc);

// BMC64's integer scaling (scaling_params in cmdline.txt, made for one
// screen size) for the screen in use: the same whole-number scale of
// FBW x FBH, as large as fits (4x of 240 lines at 1080p, for 3x at 720p);
// other sizes in proportion to the screen config.txt asks for (1280x684
// for 720p is 1920x1026 at 1080p).
void Pi45BoardScalingParams(int *fbw, int *fbh, int *sx, int *sy);

#endif
