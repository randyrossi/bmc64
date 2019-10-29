/*
 * circle.h
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

#ifndef EMU_COMMON_H
#define EMU_COMMON_H

// This is an interface layer describing both functions
// the kernel needs to invoke on the emulator and for the
// emulator to use some facilities provided by the kernel.

#include <sys/types.h>
#include <stdint.h>

#define MIN(a,b) (((a)<(b))?(a):(b))
#define MAX(a,b) (((a)>(b))?(a):(b))

#define MAX_JOY_PORTS 2

#define MACHINE_TIMING_NTSC_HDMI 0
#define MACHINE_TIMING_PAL_HDMI 1
#define MACHINE_TIMING_NTSC_COMPOSITE 2
#define MACHINE_TIMING_PAL_COMPOSITE 3
#define MACHINE_TIMING_PAL_CUSTOM 4
#define MACHINE_TIMING_NTSC_CUSTOM 5

#define FB_NUM_LAYERS   4
#define FB_LAYER_VIC    0
#define FB_LAYER_VDC    1
#define FB_LAYER_STATUS 2
#define FB_LAYER_UI     3

#define USB_PREF_ANALOG 0
#define USB_PREF_HAT 1

// Make sure does not exceed max choices in ui.h
#define NUM_BUTTON_ASSIGNMENTS 29

// NOTE: BTN_ASSIGN_* are used as indices into choice
// arrays.
#define BTN_ASSIGN_UNDEF 0
#define BTN_ASSIGN_FIRE 1
#define BTN_ASSIGN_MENU 2
#define BTN_ASSIGN_WARP 3
#define BTN_ASSIGN_STATUS_TOGGLE 4
#define BTN_ASSIGN_SWAP_PORTS 5

// Directions and POTs are not available for hotkeys, only buttons
#define BTN_ASSIGN_UP 6
#define BTN_ASSIGN_DOWN 7
#define BTN_ASSIGN_LEFT 8
#define BTN_ASSIGN_RIGHT 9
#define BTN_ASSIGN_POTX 10
#define BTN_ASSIGN_POTY 11

// Back to functions available to anything
#define BTN_ASSIGN_TAPE_MENU 12
#define BTN_ASSIGN_CART_MENU 13
#define BTN_ASSIGN_CART_FREEZE 14
#define BTN_ASSIGN_RESET_HARD 15
#define BTN_ASSIGN_RESET_SOFT 16

// More just for usb buttons
#define BTN_ASSIGN_RUN_STOP_BACK 17
#define BTN_ASSIGN_CUSTOM_KEY_1 18
#define BTN_ASSIGN_CUSTOM_KEY_2 19
#define BTN_ASSIGN_CUSTOM_KEY_3 20
#define BTN_ASSIGN_CUSTOM_KEY_4 21
#define BTN_ASSIGN_CUSTOM_KEY_5 22
#define BTN_ASSIGN_CUSTOM_KEY_6 23

// More to functions available to anything
#define BTN_ASSIGN_ACTIVE_DISPLAY 24
#define BTN_ASSIGN_PIP_LOCATION 25
#define BTN_ASSIGN_PIP_SWAP 26
#define BTN_ASSIGN_40_80_COLUMN 27
#define BTN_ASSIGN_VKBD_TOGGLE 28

// These are intermediate values not meant to
// be directly assigned to buttons. Never used as
// an index into anything.
#define BTN_ASSIGN_RESET_HARD2 916
#define BTN_ASSIGN_RESET_SOFT2 918

// potx and poty occupy 8 bits in joy int values passed
// to joy update calls
#define POTX_BIT_MASK 0x1fe0
#define POTY_BIT_MASK 0x1fe000

// Types of queued joystick events for calls into emulator API
#define PENDING_EMU_JOY_TYPE_ABSOLUTE 0
#define PENDING_EMU_JOY_TYPE_AND 1
#define PENDING_EMU_JOY_TYPE_OR 2

#define JOYDEV_NUM_JOYDEVS 21
#define JOYDEV_NONE 0
#define JOYDEV_NUMPAD 1
#define JOYDEV_KEYSET1 2
#define JOYDEV_KEYSET2 3
#define JOYDEV_ANALOG_0 4
#define JOYDEV_ANALOG_1 5
#define JOYDEV_ANALOG_2 6
#define JOYDEV_ANALOG_3 7
#define JOYDEV_ANALOG_4 8
#define JOYDEV_ANALOG_5 9
#define JOYDEV_DIGITAL_0 10
#define JOYDEV_DIGITAL_1 11
#define JOYDEV_USB_0 12
#define JOYDEV_USB_1 13
#define JOYDEV_GPIO_0 14
#define JOYDEV_GPIO_1 15
#define JOYDEV_CURS_SP 16
#define JOYDEV_NUMS_1 17
#define JOYDEV_NUMS_2 18
#define JOYDEV_CURS_LC 19
#define JOYDEV_MOUSE 20

struct axis_config {
  int use;
  int neutral;
  int min;
  int max;
  int dir;
};

struct hat_config {
  int use;
  int dir[9]; // DIR_XX_INDEX
};

// We maintain two joystick devices that can moved
// to different ports.
struct joydev_config {
  // Which port does this device belong to?
  int port;
  int device;

  // Relevant for usb devices
  struct axis_config axes[4];
  struct hat_config hats[2];
};

extern struct joydev_config joydevs[2];

extern int pot_x_high_value;
extern int pot_x_low_value;
extern int pot_y_high_value;
extern int pot_y_low_value;



extern int circle_get_machine_timing();
extern void circle_sleep(long);
extern unsigned long circle_get_ticks();
extern void circle_wait_vsync();
extern void circle_yield();
extern void circle_check_gpio();

// For FB2
extern int circle_alloc_fbl(int layer, uint8_t **pixels,
                            int width, int height, int *pitch);
extern void circle_free_fbl(int layer);
extern void circle_clear_fbl(int layer);
extern void circle_show_fbl(int layer);
extern void circle_hide_fbl(int layer);
extern void circle_frames_ready_fbl(int layer1, int layer2, int sync);
extern void circle_set_palette_fbl(int layer, uint8_t index, uint16_t rgb565);
extern void circle_set_palette32_fbl(int layer, uint8_t index, uint32_t argb);
extern void circle_update_palette_fbl(int layer);
extern void circle_set_aspect_fbl(int layer, double a);
extern void circle_set_src_rect_fbl(int layer, int x, int y, int w, int h);
extern void circle_set_center_offset(int layer, int cx, int cy);
extern void circle_set_valign_fbl(int layer, int align, int padding);
extern void circle_set_halign_fbl(int layer, int align, int padding);
extern void circle_set_padding_fbl(int layer, double lpad, double rpad, double tpad, double bpad);
extern void circle_set_zlayer_fbl(int layer, int zlayer);
extern int circle_get_zlayer_fbl(int layer);

extern void joy_set_gamepad_info(int num_pads, int num_buttons[2], int axes[2],
                                 int hats[2]);

extern void circle_joy_usb(unsigned device, int value);
extern void circle_emu_joy_interrupt(int type, int port, int device, int value);

extern void circle_usb_pref(int device, int *usb_pref, int *x_axis, int *y_axis,
                            float *x_thresh, float *y_thresh);
extern int circle_ui_activated(void);
extern void circle_ui_key_interrupt(long key, int pressed);
extern void circle_emu_key_interrupt(long key, int pressed);
extern void circle_emu_key_locked(long key, int pressed);

extern int menu_wants_raw_usb(void);
extern void menu_raw_usb(int device, unsigned buttons, const int hats[6],
                         const int axes[16]);

extern int circle_button_function(int device, int button_num, unsigned buttons,
                                  int* btn_assignment, int* is_press);
extern int circle_add_pot_values(int *value, int potx, int poty);
extern int circle_add_button_values(int dev, unsigned button_value);

extern void circle_lock_acquire();
extern void circle_lock_release();

extern void circle_key_pressed(long key);
extern void circle_key_released(long key);

extern void circle_set_demo_mode(int is_demo);
extern void circle_boot_complete();

extern void circle_mouse_move(int x, int y);
extern void circle_mouse_button_left(int pressed);
extern void circle_mouse_button_right(int pressed);
extern void circle_mouse_button_middle(int pressed);
extern void circle_mouse_button_up(int pressed);
extern void circle_mouse_button_down(int pressed);

extern void circle_emu_quick_func_interrupt(int button_assignment);

extern void circle_keyboard_set_latch_keyarr(int row, int col, int value);
extern int circle_gpio_config(void);
extern int circle_num_joysticks(void);
extern long circle_key_binding(int slot);

extern void circle_find_usb(int (*usb)[3]);
extern int circle_mount_usb(int usb);
extern int circle_unmount_usb(int usb);
extern void circle_set_volume(int value);
extern int circle_get_model();

#endif
