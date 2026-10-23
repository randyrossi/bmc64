//
// menu_pi45.h
//
// BMC64 - A bare metal Commodore emulator for Raspberry Pi
// Copyright (C) 2026 aminch <https://github.com/aminch>
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.
//
// The menus' differences on the Raspberry Pi 4 and 5, where the display is
// driven by BMC64 itself (src/kms): the machines the machine switch offers,
// and only the CRT shader settings the 3D GPU's crt-pi uses.
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
