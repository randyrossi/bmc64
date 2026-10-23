/*
 * menu_image_contents.h
 *
 * BMC64 - A bare metal Commodore emulator for Raspberry Pi
 * Copyright (C) 2026 aminch <https://github.com/aminch>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef RASPI_MENU_IMAGE_CONTENTS_H
#define RASPI_MENU_IMAGE_CONTENTS_H

// The image browser: the directory of a disk or tape image, drawn like
// LOAD"$",8 with the machine's character ROM. Choosing a program
// autostarts that file instead of the first one.

// Shows the directory of the image at path (full path with volume).
void menu_image_contents_show(const char *path);

#endif
