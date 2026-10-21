//
// board.cpp
//
// See board.h.
//
// Sources:
//  - Raspberry Pi Linux device trees, bcm2712-rpi-5-b.dts (pwr_button:
//    gpio-keys, KEY_POWER, <&gio 20 GPIO_ACTIVE_LOW>, debounce-interval 50,
//    pull-up) and bcm2712-rpi-500.dts (the same key); bcm2712.dtsi (gio at
//    0x7D508500); Circle, include/circle/bcm2712.h (ARM_GPIO1_DATA0, the
//    first bank's levels).
//  - Linux, drivers/firmware/raspberrypi.c (rpi_firmware_notify_reboot) and
//    include/soc/bcm2835/raspberrypi-firmware.h: the firmware is told with
//    the mailbox tag RPI_FIRMWARE_NOTIFY_REBOOT, 0x00030048, no data,
//    before every reboot, halt or power off.
//  - HID Usage Tables, Keyboard/Keypad page: Keyboard Power, 0x66; the
//    Raspberry Pi 400's keyboard sends it for Fn+F10 while the Pi is on
//    (Raspberry Pi, "Have you tried turning it off and on again?",
//    raspberrypi.com news, 2021).
//  - BMC64, src/viceapp.cpp: the cycles a second of each machine's 50 and
//    60 Hz HDMI timings.
//  - Raspberry Pi documentation, config.txt (Video options, Conditional
//    filters): hdmi_group and hdmi_mode, with :0 or :1 for one HDMI port;
//    CEA modes 4 (720p60), 16 (1080p60), 19 (720p50), 31 (1080p50) and 32
//    to 34 (1080p24 to 30); [pi4], [pi400], [pi5], [pi500], [all] and
//    [none] sections.
//
// This file is part of BMC64's Raspberry Pi 4 and 5 support, from BMA800XL.
// See LICENSE for licensing details.
//

#include "board.h"

#include "../defs.h"
#include "../av/fbl.h"
#include "../av/screen.h"

#if RASPPI >= 5
#include <circle/bcm2712.h>
#endif
#include <circle/bcmpropertytags.h>
#include <circle/logger.h>
#include <circle/memio.h>
#include <circle/timer.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

extern "C" void emu_power_interrupt(int power_off);

static const char FromBoard[] = "board";

static volatile bool s_power_key;

void Pi45BoardKeys(const unsigned char keys[6]) {
  static bool down = false;
  bool now = false;
  for (unsigned i = 0; i < 6; i++) {
    now = now || keys[i] == 0x66;
  }
  if (now && !down) {
    s_power_key = true;
  }
  down = now;
}

void Pi45BoardPoll(void) {
  if (s_power_key) {
    s_power_key = false;
    CLogger::Get()->Write(FromBoard, LogNotice, "Power key: power off");
    emu_power_interrupt(1);
    return;
  }
#if RASPPI >= 5
  // The button is low while pressed, held for 50 ms; it counts only once
  // it has been seen released.
  static bool seen_up = false;
  static unsigned down_since = 0;
  const u32 Button = 1u << 20;
  bool down = (read32(ARM_GPIO1_DATA0) & Button) == 0;
  unsigned now = CTimer::GetClockTicks();
  if (!down) {
    seen_up = true;
    down_since = 0;
    return;
  }
  if (!seen_up) {
    return;
  }
  if (down_since == 0) {
    down_since = now | 1;
  } else if (now - down_since >= CLOCKHZ / 20) {
    CLogger::Get()->Write(FromBoard, LogNotice, "Power button: power off");
    seen_up = false;
    emu_power_interrupt(1);
  }
#endif
}

void Pi45BoardBeforeRestart(void) {
  if (Pi45Screen::Get() != 0) {
    Pi45Screen::Get()->Blank();
  }
  // Circle's GetTag() needs room for at least one value, so the simple tag.
  CBcmPropertyTags tags;
  TPropertyTagSimple tag;
  tags.GetTag(0x00030048, &tag, sizeof tag, 0);
}

// The lines of the CEA mode config.txt asks for (hdmi_mode in hdmi_group
// 1, for this board), as BMC64's cmdline.txt scaling is set up for it; 0
// for any other mode (the size the firmware set is kept: on the Pi 4 a
// custom mode, hdmi_timings).
static unsigned ConfigLines(void) {
  FILE *fp = fopen("/config.txt", "r");
  if (fp == 0) {
    return 0;
  }
#if RASPPI >= 5
  const char *board = "pi5";
#else
  const char *board = "pi4";
#endif
  bool active = true;
  int group = 1, mode = 0;
  char line[256];
  while (fgets(line, sizeof line, fp) != 0) {
    char *p = line;
    while (*p == ' ' || *p == '\t') {
      p++;
    }
    if (*p == '[') {
      p++;
      // Sections for the other Pis ([pi4] also covers the Pi 400, [pi5]
      // the Pi 500) and [none] don't apply here.
      active = strncmp(p, board, 3) == 0 ||
               (strncmp(p, "pi", 2) != 0 && strncmp(p, "none", 4) != 0);
      continue;
    }
    if (!active) {
      continue;
    }
    // hdmi_group=, hdmi_mode=, or with :0 / :1 for one port.
    bool is_group = strncmp(p, "hdmi_group", 10) == 0;
    bool is_mode = strncmp(p, "hdmi_mode", 9) == 0;
    if (!is_group && !is_mode) {
      continue;
    }
    char *eq = strchr(p, '=');
    if (eq == 0) {
      continue;
    }
    int value = atoi(eq + 1);
    if (is_group) {
      group = value;
    } else {
      mode = value;
    }
  }
  fclose(fp);
  if (group != 1) {
    return 0;
  }
  switch (mode) {
  case 4:
  case 19:
    return 720;
  case 16:
  case 31:
  case 32:
  case 33:
  case 34:
    return 1080;
  default:
    return 0;
  }
}

void Pi45BoardMatchRate(int cycles_per_second, bool ntsc) {
  // The cycles a frame: the cycles a second of the HDMI timings, which run
  // the machine at exactly 50 or 60 frames a second, over 50 or 60.
#if defined(RASPI_PLUS4) || defined(RASPI_PLUS4EMU)
  const double pal = 1778400 / 50.0, ntsc_cycles = 1792080 / 60.0;
#elif defined(RASPI_VIC20)
  const double pal = 1107600 / 50.0, ntsc_cycles = 1017900 / 60.0;
#elif defined(RASPI_PET)
  const double pal = 1001600 / 50.0, ntsc_cycles = 1013760 / 60.0;
#else
  const double pal = 982800 / 50.0, ntsc_cycles = 1025700 / 60.0;
#endif
  double cycles_per_frame = ntsc ? ntsc_cycles : pal;
  if (cycles_per_second <= 0) {
    return;
  }
  double fps = cycles_per_second / cycles_per_frame;
  CLogger::Get()->Write(FromBoard, LogNotice, "The machine runs at %.3f frames a second", fps);
  FrameBufferLayer::MatchRate(fps, SAMPLE_RATE, ConfigLines());
}

void Pi45BoardScalingParams(int *fbw, int *fbh, int *sx, int *sy) {
  Pi45Screen *screen = Pi45Screen::Get();
  if (screen == 0 || !screen->Ready() || *fbw <= 0 || *fbh <= 0 || *sx <= 0 || *sy <= 0) {
    return;
  }
  int width = (int)screen->Width(), height = (int)screen->Height();
  if (*sx % *fbw != 0 || *sy % *fbh != 0) {
    // Not whole-number scales but a fit to the screen config.txt asks for
    // (machines.txt sets them together; the C128's VDC is 1280x684 for
    // 720p): in proportion to the screen in use, within its width.
    int made_for = (int)ConfigLines();
    if (made_for == 0 || made_for == height) {
      return;
    }
    double f = (double)height / made_for;
    if (*sx * f > width) {
      f = (double)width / *sx;
    }
    int new_sx = (int)(*sx * f) & ~1, new_sy = (int)(*sy * f) & ~1;
    if (new_sx > 0 && new_sy > 0) {
      *sx = new_sx;
      *sy = new_sy;
    }
    return;
  }
  int kx = *sx / *fbw, ky = *sy / *fbh;
  // The vertical scale that fills the most of the screen, the horizontal
  // kept in proportion (whole numbers, within the screen).
  for (int k = height / *fbh; k >= 1; k--) {
    if ((kx * k) % ky != 0) {
      continue;
    }
    int new_kx = kx * k / ky;
    if (new_kx >= 1 && new_kx * *fbw <= width) {
      *sx = new_kx * *fbw;
      *sy = k * *fbh;
      return;
    }
  }
}
