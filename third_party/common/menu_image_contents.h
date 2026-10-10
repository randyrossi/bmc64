/*
 * menu_image_contents.h
 *
 * This file is part of VICE, the Versatile Commodore Emulator.
 * See README for copyright notice.
 */

#ifndef RASPI_MENU_IMAGE_CONTENTS_H
#define RASPI_MENU_IMAGE_CONTENTS_H

// The image browser: the directory of a disk or tape image, drawn like
// LOAD"$",8 with the machine's character ROM. Choosing a program
// autostarts that file instead of the first one.

// Shows the directory of the image at path (full path with volume).
void menu_image_contents_show(const char *path);

#endif
