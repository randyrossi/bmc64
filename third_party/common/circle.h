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

#define MIN(a,b) \
   ({ __typeof__ (a) _a = (a); \
       __typeof__ (b) _b = (b); \
     _a < _b ? _a : _b; })

#define MAX(a,b) \
   ({ __typeof__ (a) _a = (a); \
       __typeof__ (b) _b = (b); \
     _a > _b ? _a : _b; })

#define MAX_USB_DEVICES 4
#define MAX_JOY_PORTS 4

#define NUM_GPIO_PINS 23

#define MACHINE_TIMING_NTSC_HDMI 0
#define MACHINE_TIMING_PAL_HDMI 1
#define MACHINE_TIMING_NTSC_COMPOSITE 2
#define MACHINE_TIMING_PAL_COMPOSITE 3
#define MACHINE_TIMING_PAL_CUSTOM_HDMI 4
#define MACHINE_TIMING_NTSC_CUSTOM_HDMI 5
#define MACHINE_TIMING_NTSC_DPI 6
#define MACHINE_TIMING_PAL_DPI 7
#define MACHINE_TIMING_PAL_CUSTOM_DPI 8
#define MACHINE_TIMING_NTSC_CUSTOM_DPI 9

#define FB_NUM_LAYERS   4
#define FB_LAYER_VIC    0
#define FB_LAYER_VDC    1
#define FB_LAYER_STATUS 2
#define FB_LAYER_UI     3

#define USB_PREF_ANALOG 0
#define USB_PREF_HAT 1
#define USB_PREF_HAT_AND_PADDLES 2

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
#define BTN_ASSIGN_RESET_MENU 29
#define BTN_ASSIGN_FLUSH_DISK 30

// These are intermediate values not meant to
// be directly assigned to buttons. Never used as
// an index into anything.
#define BTN_ASSIGN_RESET_HARD2 916
#define BTN_ASSIGN_RESET_SOFT2 918

#define JOYDEV_NUM_JOYDEVS 23
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
#define JOYDEV_USB_2 21
#define JOYDEV_USB_3 22

#define GPIO_CONFIG_DISABLED -1
#define GPIO_CONFIG_NAV_JOY 0
#define GPIO_CONFIG_KYB_JOY 1
#define GPIO_CONFIG_WAVESHARE 2
#define GPIO_CONFIG_USERPORT 3
#define GPIO_CONFIG_CUSTOM 5

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

extern struct joydev_config joydevs[MAX_JOY_PORTS];

extern int custom_gpio_pins[NUM_GPIO_PINS];

// Lower byte is BTN_ASSIGN_ constant. Upper byte can be bank or other arg.
extern unsigned int gpio_bindings[NUM_GPIO_PINS];


// -----------------------------------------------------------------------
// Functions called from emulator layer into kernel layer
// -----------------------------------------------------------------------
extern int circle_get_machine_timing();
extern void circle_sleep(long);
extern unsigned long circle_get_ticks();
extern void circle_yield();
extern void circle_check_gpio();
extern void circle_reset_gpio(int gpio_config);
extern int circle_alloc_fbl(int pixelmode, int layer, uint8_t **pixels,
                            int width, int height, int *pitch);
extern int circle_realloc_fbl(int layer, int shader);
extern void circle_free_fbl(int layer);
extern void circle_clear_fbl(int layer);
extern void circle_show_fbl(int layer);
extern void circle_hide_fbl(int layer);
extern void circle_frames_ready_fbl(int layer1, int layer2, int sync);
extern void circle_set_palette_fbl(int layer, uint8_t index, uint16_t rgb565);
extern void circle_set_palette32_fbl(int layer, uint8_t index, uint32_t argb);
extern void circle_update_palette_fbl(int layer);
extern void circle_set_stretch_fbl(int layer, double hstretch, double vstretch, int hintstr, int vintstr, int use_hintstr, int use_vintstr);
extern void circle_set_src_rect_fbl(int layer, int x, int y, int w, int h);
extern void circle_set_center_offset(int layer, int cx, int cy);
extern void circle_set_valign_fbl(int layer, int align, int padding);
extern void circle_set_halign_fbl(int layer, int align, int padding);
extern void circle_set_padding_fbl(int layer, double lpad, double rpad,
                                   double tpad, double bpad);
extern void circle_set_zlayer_fbl(int layer, int zlayer);
extern int circle_get_zlayer_fbl(int layer);
extern void circle_lock_acquire();
extern void circle_lock_release();
extern void circle_boot_complete();
extern void circle_find_usb(int (*usb)[3]);
extern int circle_mount_usb(int usb);
extern int circle_unmount_usb(int usb);
extern void circle_set_volume(int value);
extern int circle_get_model();
extern unsigned circle_get_arm_clock();
extern int circle_gpio_enabled();
extern int circle_gpio_outputs_enabled();

extern int circle_sound_init(const char *param, int *speed, int *fragsize,
                        int *fragnr, int *channels);
extern int circle_sound_write(int16_t *pbuf, size_t nr);
extern void circle_sound_close(void);
extern int circle_sound_suspend(void);
extern int circle_sound_resume(void);
extern int circle_sound_bufferspace(void);

extern uint8_t circle_get_userport_ddr(void);
extern uint8_t circle_get_userport(void);
extern void circle_set_userport(uint8_t value);
extern void circle_kernel_core_init_complete(int core);
extern void circle_get_fbl_dimensions(int layer,
                                      int *display_w, int *display_h,
                                      int *fb_w, int *fb_h,
                                      int *src_w, int *src_h,
                                      int *dst_w, int *dst_h);
extern void circle_get_scaling_params(int display,
                                      int *fbw, int *fbh,
                                      int *sx, int *sy);
extern void circle_set_interpolation(int enable);
extern void circle_set_use_shader(int enable);
extern void circle_set_shader_params(int curvature,
			float curvature_x,
			float curvature_y,
			int mask,
			float mask_brightness,
			int gamma,
			int fake_gamma,
			int scanlines,
			int multisample,
			float scanline_weight,
			float scanline_gap_brightness,
			float bloom_factor,
			float input_gamma,
			float output_gamma,
			int sharper,
			int bilinear_interpolation);

// -----------------------------------------------------------------------
// Functions called from kernel layer into emulator layer
// -----------------------------------------------------------------------

// Init some common layer stuff about the machine being emulated.
// Must be called before launching emulator's main_program func.
extern void emu_machine_init(int raster_skip_enabled, int raster_skip2_enabled);

// Compares the previous button state for 'button_num' with
// the current state and will return a press or release event
// for that button if the button has a button assignment.
extern int emu_button_function(int device, int button_num, unsigned buttons,
                               int* btn_assignment, int* is_press);

// Ask emulator to logically OR in the joystick latch value associated
// with a USB button assignment.  button_bit is the power of 2 representing
// the list of USB buttons discovered and held by usb_button_bits array.
extern int emu_add_button_values(int device, unsigned button_bit);

// Functions to trigger emulated mouse move and button events
extern void emu_mouse_move(int x, int y);
extern void emu_mouse_button_left(int pressed);
extern void emu_mouse_button_right(int pressed);
extern void emu_mouse_button_middle(int pressed);
extern void emu_mouse_wheel_up(int pressed);
extern void emu_mouse_wheel_down(int pressed);

// Queue a joystick latch event for the main loop. Interrupt safe.
extern void emu_joy_interrupt_abs(int port, int device,
                                  int js_up,
                                  int js_down,
                                  int js_left,
                                  int js_right,
                                  int js_fire,
                                  int pot_x, int pot_y);

// Queue a quick function request for the main loop. Interrupt safe.
extern void emu_quick_func_interrupt(int button_assignment);

// Ask emulator what the current gpio config index is.
extern int emu_get_gpio_config(void);

// Set a joystick latch value from a USB device. Interrupt safe.
extern void emu_set_joy_usb_interrupt(unsigned device, int value);

// Get the keycode binding for the given custom binding index.
extern long emu_get_key_binding(int index);

// Ask emulator to press/release keys by keycode.
extern void emu_key_pressed(long keycode);
extern void emu_key_released(long keycode);

// Get number of virtual joysticks available in this emulator.
extern int emu_get_num_joysticks(void);

// Enable/disable demo mode for this emulator.
extern void emu_set_demo_mode(int is_demo);

// Test whether the UI is currently activated or not.
extern int emu_is_ui_activated(void);

// Send a key press/release to the UI. Should be called only when ui
// is activated.
extern void emu_ui_key_interrupt(long key, int pressed);

// Gets usb preferences set by config for this USB device.
extern void emu_get_usb_pref(int device, int *usb_pref,
                             int *x_axis, int *y_axis,
                             float *x_thresh, float *y_thresh);

// Tell emulator about known gamepad configuration. Used after usb init.
extern void emu_set_gamepad_info(int num_pads,
                                 int num_buttons[2],
                                 int axes[2],
                                 int hats[2]);

// Test whether emulator is in a config mode where it wants to receive
// raw usb data.
extern int emu_wants_raw_usb(void);

// Send the emulator raw usb data. Used for configuring usb devices in menu.
extern void emu_set_raw_usb(int device,
                            unsigned buttons,
                            const int hats[6],
                            const int axes[16]);

extern void emu_exit(void);

#endif
