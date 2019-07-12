/*
 * menu_vic20.h
 *
 * Written by
 *  Randy Rossi <randy.rossi@gmail.com>
 *
 * This file is part of VICE, the Versatile Commodore Emulator.
 * See README for copyright notice.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
 *  02111-1307  USA.
 *
 */

unsigned long calculate_timing(double fps);

void set_color_brightness(int display_num, int value);
void set_color_contrast(int display_num, int value);
void set_color_gamma(int display_num, int value);
void set_color_tint(int display_num, int value);

void set_video_cache(int value);
void set_hw_scale(int value);

int get_color_brightness(int display_num);
int get_color_contrast(int display_num);
int get_color_gamma(int display_num);
int get_color_tint(int display_num);

void raspi_cartridge_trigger_freeze(void);
struct menu_item* menu_build_palette_options(int menu_id, struct menu_item* parent);
