/*
 * videoarch_machine.h
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

// Machine specific routines

#include "videoarch.h"

struct vkbd_key {
  int x;
  int y;
  int w;
  int h;
  int row;
  int col;
  int layout_row;
  int layout_col;
  int toggle;
  int code;
  int up;
  int down;
  int left;
  int right;
  int state;
};

// special code values for keys
#define VKBD_KEY_HOME -2
#define VKBD_DEL -3
#define VKBD_F1 -4
#define VKBD_F3 -5
#define VKBD_F5 -6
#define VKBD_F7 -7
#define VKBD_CNTRL -8
#define VKBD_RESTORE -9
#define VKBD_RUNSTOP -10
#define VKBD_SHIFTLOCK -11
#define VKBD_RETURN -12
#define VKBD_COMMODORE -13
#define VKBD_SHIFT -14
#define VKBD_CURSDOWN -15
#define VKBD_CURSRIGHT -16
#define VKBD_SPACE -17

typedef struct vkbd_key* vkbd_key_array;

extern void set_refresh_rate(int timing, struct video_canvas_s *canvas);
extern void set_video_font(void);
extern unsigned long calculate_timing(double fps);

extern void set_color_brightness(int display_num, int value);
extern void set_color_contrast(int display_num, int value);
extern void set_color_gamma(int display_num, int value);
extern void set_color_tint(int display_num, int value);
extern void set_video_cache(int value);
extern void set_hw_scale(int value);

extern int get_color_brightness(int display_num);
extern int get_color_contrast(int display_num);
extern int get_color_gamma(int display_num);
extern int get_color_tint(int display_num);

extern void raspi_cartridge_trigger_freeze(void);
extern unsigned int *raspi_get_palette(int index);
extern struct menu_item* menu_build_palette_options(int menu_id, struct menu_item* parent);

extern int is_vic(struct video_canvas_s *canvas);
extern int is_vdc(struct video_canvas_s *canvas);

extern vkbd_key_array get_vkbd();
extern int get_vkbd_width(void);
extern int get_vkbd_height(void);
extern int get_vkbd_size(void);
