//
// menu_pi45.h
//
// The menus' differences on the Raspberry Pi 4 and 5, where the display is
// driven by BMC64 itself (src/kms): the machines the machine switch offers,
// and only the CRT shader settings the 3D GPU's crt-pi uses.
//
// This file is part of BMC64's Raspberry Pi 4 and 5 support, from BMA800XL.
// See LICENSE for licensing details.
//

#ifndef MENU_PI45_H
#define MENU_PI45_H

struct menu_item;
struct machine_entry;

// The machines.txt entries these boards can show (from load_machines()):
// HDMI only; each 720p entry is followed by the same machine at 1080p (the
// PET's entries are 1080p already). The Pi 4 keeps the custom HDMI
// modes, which its firmware sets; the Pi 5's firmware doesn't, so the Pi 5
// leaves them out. The entries are numbered again.
void menu_pi45_machines(struct machine_entry **head);

// Takes the shader settings that have no effect here out of the CRT
// Shader folder (they are still saved and loaded).
void menu_pi45_trim_shader(struct menu_item *shader);

// 1 if NAME is a setting these boards no longer use (the Video menu's
// Resolution), which is then ignored.
int menu_pi45_load(const char *name, int value);

#endif
