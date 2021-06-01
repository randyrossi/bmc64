//
// kernel.cpp
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     https://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "kernel.h"

#include <errno.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <circle/gpiopin.h>

CKernel *static_kernel = NULL;

#define MAX_KEY_CODES 128
#define TICKS_PER_SECOND 1000000L

// A global to control whether our special VICE CIA port changes
// should take effect. Only set when gpio_outputs_enabled is allowed.
int raspi_userport_enabled;

// Usb key states
static bool key_states[MAX_KEY_CODES];
static unsigned char mod_states;
static bool uiLeftShift = false;
static bool uiRightShift = false;

static int vol_percent_to_vchiq(int percent) {
  int range = VCHIQ_SOUND_VOLUME_MAX-(-2720);
  return range * ((float)percent)/100.0 + (-2720);
}

// Real keyboard matrix states
static bool kbdMatrixStates[8][8];
// These for translating row/col scans into equivalent keycodes.
#if defined(RASPI_PLUS4) | defined(RASPI_PLUS4EMU)
static long kbdMatrixKeyCodes[8][8] = {
 {KEYCODE_Backspace,  KEYCODE_3,         KEYCODE_5, KEYCODE_7, KEYCODE_9, KEYCODE_Left,         KEYCODE_Up,           KEYCODE_1},
 {KEYCODE_Return,     KEYCODE_w,         KEYCODE_r, KEYCODE_y, KEYCODE_i, KEYCODE_p,            KEYCODE_Dash,         KEYCODE_BackQuote},
 {KEYCODE_BackSlash,  KEYCODE_a,         KEYCODE_d, KEYCODE_g, KEYCODE_j, KEYCODE_l,            KEYCODE_SingleQuote,  KEYCODE_Tab},
 {KEYCODE_F7,         KEYCODE_4,         KEYCODE_6, KEYCODE_8, KEYCODE_0, KEYCODE_Right,        KEYCODE_Down,         KEYCODE_2},
 {KEYCODE_F1,         KEYCODE_z,         KEYCODE_c, KEYCODE_b, KEYCODE_m, KEYCODE_Period,       KEYCODE_RightShift,   KEYCODE_Space},
 {KEYCODE_F3,         KEYCODE_s,         KEYCODE_f, KEYCODE_h, KEYCODE_k, KEYCODE_SemiColon,    KEYCODE_RightBracket, KEYCODE_LeftControl},
 {KEYCODE_F5,         KEYCODE_e,         KEYCODE_t, KEYCODE_u, KEYCODE_o, KEYCODE_LeftBracket,  KEYCODE_Equals,       KEYCODE_q},
 {KEYCODE_Insert,     KEYCODE_LeftShift, KEYCODE_x, KEYCODE_v, KEYCODE_n, KEYCODE_Comma,        KEYCODE_Slash,        KEYCODE_Escape},
};
#else
static long kbdMatrixKeyCodes[8][8] = {
 {KEYCODE_Backspace, KEYCODE_3,         KEYCODE_5, KEYCODE_7, KEYCODE_9, KEYCODE_Dash,        KEYCODE_Insert,       KEYCODE_1},
 {KEYCODE_Return,    KEYCODE_w,         KEYCODE_r, KEYCODE_y, KEYCODE_i, KEYCODE_p,           KEYCODE_RightBracket, KEYCODE_BackQuote},
 {KEYCODE_Right,     KEYCODE_a,         KEYCODE_d, KEYCODE_g, KEYCODE_j, KEYCODE_l,           KEYCODE_SingleQuote,  KEYCODE_Tab},
 {KEYCODE_F7,        KEYCODE_4,         KEYCODE_6, KEYCODE_8, KEYCODE_0, KEYCODE_Equals,      KEYCODE_Home,         KEYCODE_2},
 {KEYCODE_F1,        KEYCODE_z,         KEYCODE_c, KEYCODE_b, KEYCODE_m, KEYCODE_Period,      KEYCODE_RightShift,   KEYCODE_Space},
 {KEYCODE_F3,        KEYCODE_s,         KEYCODE_f, KEYCODE_h, KEYCODE_k, KEYCODE_SemiColon,   KEYCODE_BackSlash,    KEYCODE_LeftControl},
 {KEYCODE_F5,        KEYCODE_e,         KEYCODE_t, KEYCODE_u, KEYCODE_o, KEYCODE_LeftBracket, KEYCODE_Delete,       KEYCODE_q},
 {KEYCODE_Down,      KEYCODE_LeftShift, KEYCODE_x, KEYCODE_v, KEYCODE_n, KEYCODE_Comma,       KEYCODE_Slash,        KEYCODE_Escape},
};
#endif
static int kbdRestoreState;

extern "C" {
int circle_get_machine_timing() {
  return static_kernel->circle_get_machine_timing();
}

void circle_sleep(long delay) {
  // Timer guaranteed to be ready before vice can call this.
  return static_kernel->circle_sleep(delay);
}

unsigned long circle_get_ticks() {
  // Timer guaranteed to be ready before vice can call this.
  return static_kernel->circle_get_ticks();
}

int circle_sound_bufferspace() {
  // Sound init will happen before this so this is okay
  return static_kernel->circle_sound_bufferspace();
}

int circle_sound_init(const char *param, int *speed, int *fragsize, int *fragnr,
                      int *channels) {
  // VCHIQ is guaranteed to have been constructed but not necessarily
  // initialized so we defer its initialization until this method is
  // called by vice.
  return static_kernel->circle_sound_init(param, speed, fragsize, fragnr,
                                          channels);
}

int circle_sound_write(int16_t *pbuf, size_t nr) {
  // Sound init will happen before this so this is okay
  return static_kernel->circle_sound_write(pbuf, nr);
}

void circle_sound_close(void) {
  // Sound init will happen before this so this is okay
  static_kernel->circle_sound_close();
}

int circle_sound_suspend(void) {
  // Sound init will happen before this so this is okay
  return static_kernel->circle_sound_suspend();
}

int circle_sound_resume(void) {
  // Sound init will happen before this so this is okay
  return static_kernel->circle_sound_resume();
}

void circle_yield(void) {
  // Scheduler guaranteed to be ready before vice calls this.
  static_kernel->circle_yield();
}

void circle_check_gpio() {
  // GPIO pins guaranteed to be setup before vice calls this.
  static_kernel->circle_check_gpio();
}

void circle_reset_gpio(int gpio_config) {
  // Ensure GPIO pins are in correct configuration for current mode.
  static_kernel->circle_reset_gpio(gpio_config);
}

void circle_lock_acquire() {
  // Always ok
  static_kernel->circle_lock_acquire();
}

void circle_lock_release() {
  // Always ok
  static_kernel->circle_lock_release();
}

void circle_boot_complete() {
  // Always ok
  static_kernel->circle_boot_complete();
}

int circle_cycles_per_sec() {
  // Always ok
  return static_kernel->circle_cycles_per_second();
}

int circle_alloc_fbl(int layer, int pixelmode, uint8_t **pixels,
                     int width, int height, int *pitch) {
  return static_kernel->circle_alloc_fbl(layer, pixelmode, pixels, width, height, pitch);
}

int circle_realloc_fbl(int layer, int shader) {
  return static_kernel->circle_realloc_fbl(layer, shader);
}

void circle_free_fbl(int layer) {
  static_kernel->circle_free_fbl(layer);
}

void circle_clear_fbl(int layer) {
  static_kernel->circle_clear_fbl(layer);
}

void circle_show_fbl(int layer) {
  static_kernel->circle_show_fbl(layer);
}

void circle_hide_fbl(int layer) {
  static_kernel->circle_hide_fbl(layer);
}

void circle_frames_ready_fbl(int layer1, int layer2, int sync) {
  static_kernel->circle_frames_ready_fbl(layer1, layer2, sync);
}

void circle_set_palette_fbl(int layer, uint8_t index, uint16_t rgb565) {
  static_kernel->circle_set_palette_fbl(layer, index, rgb565);
}

void circle_set_palette32_fbl(int layer, uint8_t index, uint32_t argb) {
  static_kernel->circle_set_palette32_fbl(layer, index, argb);
}

void circle_update_palette_fbl(int layer) {
  static_kernel->circle_update_palette_fbl(layer);
}

void circle_set_stretch_fbl(int layer, double hstretch, double vstretch, int hintstr, int vintstr, int use_hintstr, int use_vintstr) {
  static_kernel->circle_set_stretch_fbl(layer, hstretch, vstretch, hintstr, vintstr, use_hintstr, use_vintstr);
}

void circle_set_center_offset(int layer, int cx, int cy) {
  static_kernel->circle_set_center_offset(layer, cx, cy);
}

void circle_set_src_rect_fbl(int layer, int x, int y, int w, int h) {
  static_kernel->circle_set_src_rect_fbl(layer, x,y,w,h);
}

void circle_set_valign_fbl(int layer, int align, int padding) {
  static_kernel->circle_set_valign_fbl(layer, align, padding);
}

void circle_set_halign_fbl(int layer, int align, int padding) {
  static_kernel->circle_set_halign_fbl(layer, align, padding);
}

void circle_set_padding_fbl(int layer, double lpad, double rpad, double tpad, double bpad) {
  static_kernel->circle_set_padding_fbl(layer, lpad, rpad, tpad, bpad);
}

void circle_set_zlayer_fbl(int layer, int zlayer) {
  static_kernel->circle_set_zlayer_fbl(layer, zlayer);
}

int circle_get_zlayer_fbl(int layer) {
  return static_kernel->circle_get_zlayer_fbl(layer);
}

void circle_find_usb(int (*usb)[3]) {
  return static_kernel->circle_find_usb(usb);
}

int circle_mount_usb(int usb) {
  return static_kernel->circle_mount_usb(usb);
}

int circle_unmount_usb(int usb) {
  return static_kernel->circle_unmount_usb(usb);
}

void circle_set_volume(int value) {
  static_kernel->circle_set_volume(value);
}

int circle_get_model() {
  return static_kernel->circle_get_model();
}

unsigned circle_get_arm_clock() {
  return static_kernel->circle_get_arm_clock();
}

int circle_gpio_enabled() {
  return static_kernel->circle_gpio_enabled();
}

int circle_gpio_outputs_enabled() {
  return static_kernel->circle_gpio_outputs_enabled();
}

void circle_kernel_core_init_complete(int core) {
  static_kernel->circle_kernel_core_init_complete(core);
}

void circle_get_fbl_dimensions(int layer, int *display_w, int *display_h,
                               int *fb_w, int *fb_h,
                               int *src_w, int *src_h,
                               int *dst_w, int *dst_h) {
  static_kernel->circle_get_fbl_dimensions(layer, display_w, display_h,
                                           fb_w, fb_h,
                                           src_w, src_h, dst_w, dst_h);
}

void circle_get_scaling_params(int display,
                               int *fbw, int *fbh,
                               int *sx, int *sy) {
  static_kernel->circle_get_scaling_params(display, fbw, fbh, sx, sy);
}

void circle_set_interpolation(int enable) {
  static_kernel->circle_set_interpolation(enable);
}

void circle_set_use_shader(int enable) {
  static_kernel->circle_set_use_shader(enable);
}

void circle_set_shader_params(int curvature,
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
                int bilinear_interpolation) {
  static_kernel->circle_set_shader_params(curvature,
			curvature_x,
			curvature_y,
			mask,
			mask_brightness,
			gamma,
			fake_gamma,
			scanlines,
			multisample,
			scanline_weight,
			scanline_gap_brightness,
			bloom_factor,
			input_gamma,
			output_gamma,
			sharper,
                        bilinear_interpolation);
}
};

namespace {

long func_to_keycode(int btn_func) {
   switch (btn_func) {
      case BTN_ASSIGN_UP:
         return KEYCODE_Up;
      case BTN_ASSIGN_DOWN:
         return KEYCODE_Down;
      case BTN_ASSIGN_LEFT:
         return KEYCODE_Left;
      case BTN_ASSIGN_RIGHT:
         return KEYCODE_Right;
      case BTN_ASSIGN_FIRE:
         return KEYCODE_Return;
      default:
         return 0;
   }
}

}

CKernel::CKernel(void)
    : ViceStdioApp("vice"), mViceSound(nullptr),
      mNumJoy(emu_get_num_joysticks()),
      mVolume(100), mNumCoresComplete(0),
      mNeedSoundInit(false), mNumSoundChannels(1) {
  static_kernel = this;
  mod_states = 0;
  memset(key_states, 0, MAX_KEY_CODES * sizeof(bool));

  // Only used for pins that are used as buttons. See viceapp.h.
  for (int i = 0; i < NUM_GPIO_PINS; i++) {
    gpio_debounce_state[i] = BTN_UP;
    gpio_prev_state[i] = HIGH;
  }

  kbdRestoreState = HIGH;
  for (int i = 0; i < 8; i++) {
    for (int j = 0; j < 8; j++) {
      kbdMatrixStates[i][j] = HIGH;
    }
  }

  fbl[FB_LAYER_VIC].SetLayer(0);
  fbl[FB_LAYER_VIC].SetTransparency(false);

  fbl[FB_LAYER_VDC].SetLayer(1);
  fbl[FB_LAYER_VDC].SetTransparency(false);

  fbl[FB_LAYER_STATUS].SetLayer(2);
  fbl[FB_LAYER_STATUS].SetTransparency(true);

  fbl[FB_LAYER_UI].SetLayer(3);
  fbl[FB_LAYER_UI].SetTransparency(true);

  if (circle_gpio_outputs_enabled()) {
     raspi_userport_enabled = 1;
  }
}

bool CKernel::Initialize(void) {
  if (!ViceStdioApp::Initialize()) {
    return false;
  }

  return true;
}

static void exec_button_func(int button_func, int is_press, int is_ui) {
   // KEEP THIS IN SYNC WITH kbd.c
   switch (button_func) {
     case BTN_ASSIGN_MENU:
       if (is_press) {
          emu_key_pressed(KEYCODE_F12);
       } else {
          emu_key_released(KEYCODE_F12);
       }
       break;
     case BTN_ASSIGN_WARP:
     case BTN_ASSIGN_SWAP_PORTS:
     case BTN_ASSIGN_STATUS_TOGGLE:
     case BTN_ASSIGN_TAPE_MENU:
     case BTN_ASSIGN_CART_MENU:
     case BTN_ASSIGN_CART_FREEZE:
     case BTN_ASSIGN_RESET_MENU:
     case BTN_ASSIGN_RESET_HARD:
     case BTN_ASSIGN_RESET_SOFT:
     case BTN_ASSIGN_ACTIVE_DISPLAY:
     case BTN_ASSIGN_PIP_LOCATION:
     case BTN_ASSIGN_PIP_SWAP:
     case BTN_ASSIGN_40_80_COLUMN:
     case BTN_ASSIGN_VKBD_TOGGLE:
     case BTN_ASSIGN_FLUSH_DISK:
       if (is_press) {
          emu_quick_func_interrupt(button_func);
       }
       break;
     case BTN_ASSIGN_CUSTOM_KEY_1:
     case BTN_ASSIGN_CUSTOM_KEY_2:
     case BTN_ASSIGN_CUSTOM_KEY_3:
     case BTN_ASSIGN_CUSTOM_KEY_4:
     case BTN_ASSIGN_CUSTOM_KEY_5:
     case BTN_ASSIGN_CUSTOM_KEY_6:
        if (is_press) {
           emu_key_pressed(
               emu_get_key_binding(button_func - BTN_ASSIGN_CUSTOM_KEY_1));
        } else {
           emu_key_released(
               emu_get_key_binding(button_func - BTN_ASSIGN_CUSTOM_KEY_1));
        }
        break;
     case BTN_ASSIGN_RUN_STOP_BACK:
       if (is_ui) {
         emu_ui_key_interrupt(KEYCODE_Escape, is_press);
       } else {
         if (is_press) {
            emu_key_pressed(KEYCODE_Escape);
         } else {
            emu_key_released(KEYCODE_Escape);
         }
       }
       break;
     // Only do direction/fire button assignments for UI, joy is handled
     // in circle_add_usb_values seperately.
     case BTN_ASSIGN_UP:
     case BTN_ASSIGN_DOWN:
     case BTN_ASSIGN_LEFT:
     case BTN_ASSIGN_RIGHT:
     case BTN_ASSIGN_FIRE:
       if (is_ui) {
         emu_ui_key_interrupt(func_to_keycode(button_func), is_press);
       }
       break;
     default:
       break;
   }
}

// KEEP THIS IN SYNC WITH kbd.c
static void handle_button_function(bool is_ui, int device, unsigned buttons) {
  int button_num = 0;

  int button_func;
  int is_press;

  while (emu_button_function(device, button_num, buttons,
                             &button_func, &is_press) >= 0) {
    exec_button_func(button_func, is_press, is_ui);
    button_num++;
  }
}

#if 0 // COUNT INVOCATIONS PER SECOND
static unsigned long entry_delay = 5 * TICKS_PER_SECOND;
static unsigned long entry_start = 0;
static long invoked;
#endif

// Interrupt handler. Make this quick.
void CKernel::GamePadStatusHandler(unsigned nDeviceIndex,
                                   const TGamePadState *pState) {

#if 0 // COUNT INVOCATIONS PER SECOND
invoked++;
if (static_kernel->circle_get_ticks() - entry_start >= entry_delay) {
   printf ("%ld\n", invoked / 5);
   invoked = 0;
   entry_start = static_kernel->circle_get_ticks();
}
#endif

  static int dpad_to_joy[8] = {0x01, 0x09, 0x08, 0x0a, 0x02, 0x06, 0x04, 0x05};

  static unsigned int prev_buttons[MAX_USB_DEVICES] = {0, 0, 0, 0};
  static int prev_dpad[MAX_USB_DEVICES] = {8, 8, 8, 8};
  static int prev_axes_dirs[MAX_USB_DEVICES][4] = {{0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}};
  static int prev_xaxes_values[MAX_USB_DEVICES] = {0,0,0,0};
  static int prev_yaxes_values[MAX_USB_DEVICES] = {0,0,0,0};

  if (nDeviceIndex >= MAX_USB_DEVICES)
    return;

  if (emu_wants_raw_usb()) {
    // Send the raw usb data and we're done.
    int axes[16];
    for (int i = 0; i < pState->naxes; i++) {
      axes[i] = pState->axes[i].value;
    }
    emu_set_raw_usb(nDeviceIndex, pState->buttons, pState->hats, axes);
    return;
  }

  int ui_activated = emu_is_ui_activated();

  unsigned b = pState->buttons;

  // usb_pref is the value of the usb pref menu item
  int usb_pref;
  int axis_x;
  int axis_y;
  float thresh_x;
  float thresh_y;
  emu_get_usb_pref(nDeviceIndex, &usb_pref, &axis_x, &axis_y, &thresh_x,
                   &thresh_y);

  int max_index = axis_x;
  if (axis_y > max_index)
    max_index = axis_y;

  if ((usb_pref == USB_PREF_HAT || usb_pref == USB_PREF_HAT_AND_PADDLES) &&
        pState->nhats > 0) {
    int dpad = pState->hats[0];
    bool has_changed =
        (prev_buttons[nDeviceIndex] != b) || (prev_dpad[nDeviceIndex] != dpad);

    if (usb_pref == USB_PREF_HAT_AND_PADDLES) {
       int xval = pState->axes[axis_x].value;
       int yval = pState->axes[axis_y].value;
       has_changed |=
          prev_xaxes_values[nDeviceIndex] != xval ||
	   prev_yaxes_values[nDeviceIndex] != yval;
       prev_xaxes_values[nDeviceIndex] = xval;
       prev_yaxes_values[nDeviceIndex] = yval;
    }

    if (has_changed) {
      int old_dpad = prev_dpad[nDeviceIndex];
      prev_buttons[nDeviceIndex] = b;
      prev_dpad[nDeviceIndex] = dpad;

      // If the UI is activated, route to the menu.
      if (ui_activated) {
        if (dpad == 0 && old_dpad != 0) {
          emu_ui_key_interrupt(KEYCODE_Up, 1);
        } else if (dpad != 0 && old_dpad == 0) {
          emu_ui_key_interrupt(KEYCODE_Up, 0);
        }
        if (dpad == 4 && old_dpad != 4) {
          emu_ui_key_interrupt(KEYCODE_Down, 1);
        } else if (dpad != 4 && old_dpad == 4) {
          emu_ui_key_interrupt(KEYCODE_Down, 0);
        }
        if (dpad == 6 && old_dpad != 6) {
          emu_ui_key_interrupt(KEYCODE_Left, 1);
        } else if (dpad != 6 && old_dpad == 6) {
          emu_ui_key_interrupt(KEYCODE_Left, 0);
        }
        if (dpad == 2 && old_dpad != 2) {
          emu_ui_key_interrupt(KEYCODE_Right, 1);
        } else if (dpad != 2 && old_dpad == 2) {
          emu_ui_key_interrupt(KEYCODE_Right, 0);
        }
        handle_button_function(true, nDeviceIndex, b);
        return;
      }

      handle_button_function(false, nDeviceIndex, b);

      int value = 0;
      if (dpad < 8)
        value |= dpad_to_joy[dpad];
      value |= emu_add_button_values(nDeviceIndex, b);

      // Handle axes as paddles here. This will potentially overwrite
      // 2nd/3rd button configs from the call above if they were
      // assigned.  The UI does not prevent the user from assigning
      // potx/poty as buttons and specifying axes as paddles at the same
      // time.
      if (usb_pref == USB_PREF_HAT_AND_PADDLES && pState->naxes > max_index) {
         int minx = pState->axes[axis_x].minimum;
         int maxx = pState->axes[axis_x].maximum;
         int miny = pState->axes[axis_y].minimum;
         int maxy = pState->axes[axis_y].maximum;
         int distx = maxx - minx;
         int disty = maxy - miny;
         double scalex = distx / 255.0d;
         double scaley = disty / 255.0d;
         unsigned char valuex = (pState->axes[axis_x].value - minx) / scalex;
         unsigned char valuey = (pState->axes[axis_y].value - miny) / scaley;
         value &= ~ 0x1fffe0; // null out potx and poty
         value |= (valuex << 5);
         value |= (valuey << 13);
      }

      emu_set_joy_usb_interrupt(nDeviceIndex, value);
    }


  } else if (usb_pref == USB_PREF_ANALOG && pState->naxes > max_index) {
    // TODO: Do this just once at init
    int minx = pState->axes[axis_x].minimum;
    int maxx = pState->axes[axis_x].maximum;
    int miny = pState->axes[axis_y].minimum;
    int maxy = pState->axes[axis_y].maximum;
    int tx = (maxx - minx) / 2 * thresh_x;
    int mx = (maxx + minx) / 2;
    int ty = (maxy - miny) / 2 * thresh_y;
    int my = (maxy + miny) / 2;
    int a_left = pState->axes[axis_x].value < mx - tx;
    int a_right = pState->axes[axis_x].value > mx + tx;
    int a_up = pState->axes[axis_y].value < my - ty;
    int a_down = pState->axes[axis_y].value > my + ty;
    bool has_changed = (prev_buttons[nDeviceIndex] != b) ||
                       (prev_axes_dirs[nDeviceIndex][0] != a_up) ||
                       (prev_axes_dirs[nDeviceIndex][1] != a_down) ||
                       (prev_axes_dirs[nDeviceIndex][2] != a_left) ||
                       (prev_axes_dirs[nDeviceIndex][3] != a_right);
    if (has_changed) {
      int prev_a_up = prev_axes_dirs[nDeviceIndex][0];
      int prev_a_down = prev_axes_dirs[nDeviceIndex][1];
      int prev_a_left = prev_axes_dirs[nDeviceIndex][2];
      int prev_a_right = prev_axes_dirs[nDeviceIndex][3];
      prev_axes_dirs[nDeviceIndex][0] = a_up;
      prev_axes_dirs[nDeviceIndex][1] = a_down;
      prev_axes_dirs[nDeviceIndex][2] = a_left;
      prev_axes_dirs[nDeviceIndex][3] = a_right;
      prev_buttons[nDeviceIndex] = b;
      // If the UI is activated, route to the menu.

      if (ui_activated) {
        if (a_up && !prev_a_up) {
          emu_ui_key_interrupt(KEYCODE_Up, 1);
        } else if (!a_up && prev_a_up) {
          emu_ui_key_interrupt(KEYCODE_Up, 0);
        }
        if (a_down && !prev_a_down) {
          emu_ui_key_interrupt(KEYCODE_Down, 1);
        } else if (!a_down && prev_a_down) {
          emu_ui_key_interrupt(KEYCODE_Down, 0);
        }
        if (a_left && !prev_a_left) {
          emu_ui_key_interrupt(KEYCODE_Left, 1);
        } else if (!a_left && prev_a_left) {
          emu_ui_key_interrupt(KEYCODE_Left, 0);
        }
        if (a_right && !prev_a_right) {
          emu_ui_key_interrupt(KEYCODE_Right, 1);
        } else if (!a_right && prev_a_right) {
          emu_ui_key_interrupt(KEYCODE_Right, 0);
        }
        handle_button_function(true, nDeviceIndex, b);
        return;
      }

      handle_button_function(false, nDeviceIndex, b);

      int value = 0;
      if (a_left)
        value |= 0x4;
      if (a_right)
        value |= 0x8;
      if (a_up)
        value |= 0x1;
      if (a_down)
        value |= 0x2;
      value |= emu_add_button_values(nDeviceIndex, b);
      emu_set_joy_usb_interrupt(nDeviceIndex, value);
    }
  }
}

void CKernel::SetupUSBKeyboard() {
  CUSBKeyboardDevice *pKeyboard =
      (CUSBKeyboardDevice *)mDeviceNameService.GetDevice("ukbd1", FALSE);
  if (pKeyboard) {
    pKeyboard->RegisterKeyStatusHandlerRaw(KeyStatusHandlerRaw);
  }
}

void CKernel::SetupUSBMouse() {
  CMouseDevice *pMouse =
      (CMouseDevice *)mDeviceNameService.GetDevice("mouse1", FALSE);
  if (pMouse) {
    pMouse->RegisterStatusHandler(MouseStatusHandler);
  }
}

void CKernel::SetupUSBGamepads() {
  unsigned num_pads = 0;
  int num_buttons[MAX_USB_DEVICES] = {0, 0, 0, 0};
  int num_axes[MAX_USB_DEVICES] = {0, 0, 0, 0};
  int num_hats[MAX_USB_DEVICES] = {0, 0, 0, 0};
  while (num_pads < MAX_USB_DEVICES) {
    CString DeviceName;
    DeviceName.Format("upad%u", num_pads + 1);

    CUSBGamePadDevice *game_pad =
        (CUSBGamePadDevice *)mDeviceNameService.GetDevice(DeviceName, FALSE);

    if (game_pad == 0) {
      break;
    }

    const TGamePadState *pState = game_pad->GetInitialState();
    assert(pState != 0);

    num_axes[num_pads] = pState->naxes;
    num_hats[num_pads] = pState->nhats;
    num_buttons[num_pads] = pState->nbuttons;

    game_pad->RegisterStatusHandler(GamePadStatusHandler);
    num_pads++;
  }

  // Tell the emulator what we found
  emu_set_gamepad_info(num_pads, num_buttons, num_axes, num_hats);
}

ViceApp::TShutdownMode CKernel::Run(void) {
  SetupUSBKeyboard();
  SetupUSBMouse();
  SetupUSBGamepads();

  emu_set_demo_mode(mViceOptions.DemoEnabled());

#ifndef ARM_ALLOW_MULTI_CORE
  mEmulatorCore->LaunchEmulator(mTimingOption);
#else
  // This core will do nothing but service interrupts from
  // usb or gpio.
  printf("Core 0 idle\n");

  asm("dsb\n\t"
      "1: wfi\n\t"
      "b 1b\n\t");
#endif
  return ShutdownHalt;
}

void CKernel::ScanKeyboard() {
  int ui_activated = emu_is_ui_activated();

  int restore = gpioPins[GPIO_KBD_RESTORE_INDEX]->Read();
  // For restore, there is no public API that triggers it so we will
  // pass the keycode that will.  NOTE: On the plus/4, this key sym
  // will be the CLR key according to the keymap.
  if (restore == LOW && kbdRestoreState == HIGH) {
     emu_key_pressed(restore_key_sym);
  } else if (restore == HIGH && kbdRestoreState == LOW) {
     emu_key_released(restore_key_sym);
  }
  kbdRestoreState = restore;

  // Keyboard scan
  for (int kbdPA = 0; kbdPA < 8; kbdPA++) {
    gpioPins[kbdPA]->SetMode(GPIOModeOutput);
    gpioPins[kbdPA]->Write(LOW);
    circle_sleep(10);
    for (int kbdPB = 0; kbdPB < 8; kbdPB++) {
      // Read PB line
      int val = gpioPins[kbdPB + 8]->Read();

      // My PA/PB to keycode matrix is transposed and I'm too lazy to fix
      // it. Just swap PB and PA here for the keycode lookup.
      long keycode = kbdMatrixKeyCodes[kbdPB][kbdPA];

      if (ui_activated) {
        if (val == LOW && kbdMatrixStates[kbdPA][kbdPB] == HIGH) {
          if (keycode == KEYCODE_LeftShift) {
             uiLeftShift = true;
          } else if (keycode == KEYCODE_RightShift) {
             uiRightShift = true;
          }

          if (keycode == KEYCODE_Right && (uiLeftShift || uiRightShift)) {
             emu_key_pressed(KEYCODE_Left);
          } else if (keycode == KEYCODE_Down && (uiLeftShift || uiRightShift)) {
             emu_key_pressed(KEYCODE_Up);
          } else {
             emu_key_pressed(keycode);
          }
        } else if (val == HIGH && kbdMatrixStates[kbdPA][kbdPB] == LOW) {
          if (keycode == KEYCODE_LeftShift) {
             uiLeftShift = false;
          } else if (keycode == KEYCODE_RightShift) {
             uiRightShift = false;
          }
          if (keycode == KEYCODE_Right && (uiLeftShift || uiRightShift)) {
             emu_key_released(KEYCODE_Left);
          } else if (keycode == KEYCODE_Down && (uiLeftShift || uiRightShift)) {
             emu_key_released(KEYCODE_Up);
          } else {
             emu_key_released(keycode);
          }
        }
      } else {
        // TODO: Need to watch out for key combos here.  Hook into
        // the handle functions directly in kbd.c so we can invoke the
        // same hotkey funcs.
        if (val == LOW && kbdMatrixStates[kbdPA][kbdPB] == HIGH) {
          emu_key_pressed(keycode);
        } else if (val == HIGH && kbdMatrixStates[kbdPA][kbdPB] == LOW) {
          emu_key_released(keycode);
        }
      }
      kbdMatrixStates[kbdPA][kbdPB] = val;
    }
    gpioPins[kbdPA]->SetMode(GPIOModeInputPullUp);
  }
}

// Read joystick state.
// If gpioConfig is 0, the NavButtons+Joys config is used where pins can
// be grounded.
// If gpioConfig is 1, the Keyboard+Joys PCB config is used (where
// selector is used to drive pins low instead of GND).
// If gpioConfig is 2, the Waveshare HAT layout is used.
void CKernel::ReadJoystick(int device, int gpioConfig) {
  // For remembering button states for UI only
  static int js_prev_0[5] = {HIGH, HIGH, HIGH, HIGH, HIGH};
  static int js_prev_1[5] = {HIGH, HIGH, HIGH, HIGH, HIGH};

  int *js_prev;
  CGPIOPin **js_pins = NULL;
  CGPIOPin *js_selector = NULL;
  int port = 0;
  int devd = 0;
  int ui_activated = emu_is_ui_activated();

  // If ui is activated, don't bail if port assignment can't be done
  // since the event will always go to the ui. We want the joystick to
  // function in the ui even if the control port is not assigned to be
  // gpio.
  if (device == 0) {
    if (joydevs[0].device == JOYDEV_GPIO_0) {
      port = joydevs[0].port;
      devd = JOYDEV_GPIO_0;
    } else if (joydevs[1].device == JOYDEV_GPIO_0) {
      port = joydevs[1].port;
      devd = JOYDEV_GPIO_0;
    } else if (!ui_activated) {
      return;
    }

    js_prev = js_prev_0;
    switch (gpioConfig) {
       case GPIO_CONFIG_NAV_JOY:
          js_pins = config_0_joystickPins1;
          break;
       case GPIO_CONFIG_KYB_JOY:
          js_selector = gpioPins[GPIO_JS1_SELECT_INDEX];
          js_pins = config_1_joystickPins1;
          break;
       case GPIO_CONFIG_WAVESHARE:
          js_pins = config_2_joystickPins;
          break;
       case GPIO_CONFIG_USERPORT:
          js_pins = config_3_joystickPins1;
          break;
       default:
         assert(false);
    }
  } else {
    if (joydevs[0].device == JOYDEV_GPIO_1) {
      port = joydevs[0].port;
      devd = JOYDEV_GPIO_1;
    } else if (joydevs[1].device == JOYDEV_GPIO_1) {
      port = joydevs[1].port;
      devd = JOYDEV_GPIO_1;
    } else if (!ui_activated) {
      return;
    }

    js_prev = js_prev_1;
    switch (gpioConfig) {
       case GPIO_CONFIG_NAV_JOY:
         js_pins = config_0_joystickPins2;
         break;
       case GPIO_CONFIG_KYB_JOY:
         js_selector = gpioPins[GPIO_JS2_SELECT_INDEX];
         js_pins = config_1_joystickPins2;
         break;
       case GPIO_CONFIG_USERPORT:
          js_pins = config_3_joystickPins2;
          break;
       default:
         assert(false);
    }
  }

  if (gpioConfig == 1) {
    // Drive the select pin low. Don't leave this routine
    // before setting it as input-pullup again.
    js_selector->SetMode(GPIOModeOutput);
    js_selector->Write(LOW);
    circle_sleep(10);
  }

  int js_up = js_pins[JOY_UP]->Read();
  int js_down = js_pins[JOY_DOWN]->Read();
  int js_left = js_pins[JOY_LEFT]->Read();
  int js_right = js_pins[JOY_RIGHT]->Read();
  int js_fire = js_pins[JOY_FIRE]->Read();
  int js_potx = gpioConfig == 2 ? js_pins[JOY_POTX]->Read() : HIGH;
  int js_poty = gpioConfig == 2 ? js_pins[JOY_POTY]->Read() : HIGH;

  if (ui_activated) {
    if (js_up == LOW && js_prev[JOY_UP] != LOW) {
      emu_ui_key_interrupt(KEYCODE_Up, 1);
    } else if (js_up != LOW && js_prev[JOY_UP] == LOW) {
      emu_ui_key_interrupt(KEYCODE_Up, 0);
    }

    if (js_down == LOW && js_prev[JOY_DOWN] != LOW) {
      emu_ui_key_interrupt(KEYCODE_Down, 1);
    } else if (js_down != LOW && js_prev[JOY_DOWN] == LOW) {
      emu_ui_key_interrupt(KEYCODE_Down, 0);
    }

    if (js_left == LOW && js_prev[JOY_LEFT] != LOW) {
      emu_ui_key_interrupt(KEYCODE_Left, 1);
    } else if (js_left != LOW && js_prev[JOY_LEFT] == LOW) {
      emu_ui_key_interrupt(KEYCODE_Left, 0);
    }

    if (js_right == LOW && js_prev[JOY_RIGHT] != LOW) {
      emu_ui_key_interrupt(KEYCODE_Right, 1);
    } else if (js_right != LOW && js_prev[JOY_RIGHT] == LOW) {
      emu_ui_key_interrupt(KEYCODE_Right, 0);
    }

    if (js_fire == LOW && js_prev[JOY_FIRE] != LOW) {
      emu_ui_key_interrupt(KEYCODE_Return, 1);
    } else if (js_fire != LOW && js_prev[JOY_FIRE] == LOW) {
      emu_ui_key_interrupt(KEYCODE_Return, 0);
    }
    js_prev[JOY_UP] = js_up;
    js_prev[JOY_DOWN] = js_down;
    js_prev[JOY_LEFT] = js_left;
    js_prev[JOY_RIGHT] = js_right;
    js_prev[JOY_FIRE] = js_fire;
    // not necessary to remember pot values as they are not used for ui
  } else {
    emu_joy_interrupt_abs(port, devd,
                          js_up == LOW,
                          js_down == LOW,
                          js_left == LOW,
                          js_right == LOW,
                          js_fire == LOW,
                          js_potx == LOW,
                          js_poty == LOW);
  }

  if (gpioConfig == 1) {
     js_selector->SetMode(GPIOModeInputPullUp);
  }
}

void CKernel::ReadCustomGPIO() {
  int i;
  unsigned int bank;
  unsigned int func;
  int value;

  int js_up_1 = HIGH;
  int js_down_1 = HIGH;
  int js_left_1 = HIGH;
  int js_right_1 = HIGH;
  int js_fire_1 = HIGH;
  int js_potx_1 = HIGH;
  int js_poty_1 = HIGH;

  int js_up_2 = HIGH;
  int js_down_2 = HIGH;
  int js_left_2 = HIGH;
  int js_right_2 = HIGH;
  int js_fire_2 = HIGH;
  int js_potx_2 = HIGH;
  int js_poty_2 = HIGH;

  int ui_activated = emu_is_ui_activated();
  int port_is_gpio_joy[2] = {0,0};

  for (i = 0 ; i < NUM_GPIO_PINS; i++) {
    bank = gpio_bindings[i] >> 8;
    func = gpio_bindings[i] & 0xFF;
    if (bank > 0) {
      // This is for a joystick bank
      value = gpioPins[i]->Read();
      if (ui_activated) {
        if (value == LOW && gpio_prev_state[i] != LOW) {
          emu_ui_key_interrupt(func_to_keycode(func), 1);
        } else if (value != LOW && gpio_prev_state[i] == LOW) {
          emu_ui_key_interrupt(func_to_keycode(func), 0);
        }
        gpio_prev_state[i] = value;
      } else {
        int dev_match = bank == 1 ? JOYDEV_GPIO_0 : JOYDEV_GPIO_1;

        int port;
        if (joydevs[0].device == dev_match) {
          port = joydevs[0].port;
        } else if (joydevs[1].device == dev_match) {
          port = joydevs[1].port;
        } else {
          continue;
        }

        port_is_gpio_joy[port-1] = 1;

        switch (func) {
          case BTN_ASSIGN_UP:
            if (port == 1) {
              js_up_1 &= value;
            } else {
              js_up_2 &= value;
            }
            break;
          case BTN_ASSIGN_DOWN:
            if (port == 1) {
              js_down_1 &= value;
            } else {
              js_down_2 &= value;
            }
            break;
          case BTN_ASSIGN_LEFT:
            if (port == 1) {
              js_left_1 &= value;
            } else {
              js_left_2 &= value;
            }
            break;
          case BTN_ASSIGN_RIGHT:
            if (port == 1) {
              js_right_1 &= value;
            } else {
              js_right_2 &= value;
            }
            break;
          case BTN_ASSIGN_FIRE:
            if (port == 1) {
              js_fire_1 &= value;
            } else {
              js_fire_2 &= value;
            }
            break;
          case BTN_ASSIGN_POTX:
            if (port == 1) {
              js_potx_1 &= value;
            } else {
              js_potx_2 &= value;
            }
            break;
          case BTN_ASSIGN_POTY:
            if (port == 1) {
              js_poty_1 &= value;
            } else {
              js_poty_2 &= value;
            }
            break;
          }
        }
      } else {
        int debounced = ReadDebounced(i);
        if (debounced == BTN_PRESS) {
          exec_button_func(func, 1, ui_activated);
        } else if (debounced == BTN_RELEASE) {
          exec_button_func(func, 0, ui_activated);
        }
     }
   }

   // Only send a value if there was a device match
   // The device here doesn't really matter.
   if (port_is_gpio_joy[0]) {
      emu_joy_interrupt_abs(1, JOYDEV_GPIO_0,
                         js_up_1 == LOW,
                         js_down_1 == LOW,
                         js_left_1 == LOW,
                         js_right_1 == LOW,
                         js_fire_1 == LOW,
                         js_potx_1 == LOW,
                         js_poty_1 == LOW);
   }

   // The device here doesn't really matter.
   if (port_is_gpio_joy[1]) {
      emu_joy_interrupt_abs(2, JOYDEV_GPIO_1,
                         js_up_2 == LOW,
                         js_down_2 == LOW,
                         js_left_2 == LOW,
                         js_right_2 == LOW,
                         js_fire_2 == LOW,
                         js_potx_2 == LOW,
                         js_poty_2 == LOW);
   }
}

// Configure user port DDR
void CKernel::SetupUserport() {
  // Unless enable_gpio_outputs is true, this will have no effect. Menu item
  // should reflect this.
  if (circle_gpio_outputs_enabled()) {
    uint8_t ddr = circle_get_userport_ddr();
    for (int i = 0; i < 8; i++) {
      uint8_t bit_pos = 1<<i;
      uint8_t ddr_value = ddr & bit_pos;
      config_3_userportPins[i]->SetMode(ddr_value ? GPIOModeOutput : GPIOModeInputPullUp);
    }
  }
}

// Read input pins and send to output pins
void CKernel::ReadWriteUserport() {
  // Unless enable_gpio_outputs is true, this will have no effect. Menu item
  // should reflect this.
  if (circle_gpio_outputs_enabled()) {
    uint8_t ddr = circle_get_userport_ddr();
    uint8_t value = circle_get_userport();
    uint8_t new_value = 0;
    for (int i = 0; i < 8; i++) {
      uint8_t bit_pos = 1<<i;
      uint8_t ddr_value = ddr & bit_pos;
      uint8_t data_value = value & bit_pos;
      if (ddr_value) {
        // output bit
        config_3_userportPins[i]->Write(data_value ? HIGH : LOW);
        new_value |= data_value;
      } else {
        // input bit
        if (config_3_userportPins[i]->Read() == HIGH) {
          new_value |= bit_pos;
        }
      }
    }
    circle_set_userport(new_value);
  }
}

void CKernel::circle_sleep(long delay) { mTimer.SimpleusDelay(delay); }

unsigned long CKernel::circle_get_ticks() { return mTimer.GetClockTicks(); }

// Called from VICE: Core 1
int CKernel::circle_sound_init(const char *param, int *speed, int *fragsize,
                               int *fragnr, int *channels) {
  *speed = SAMPLE_RATE;
  *fragsize = FRAG_SIZE;
  *fragnr = NUM_FRAGS;
  mNumSoundChannels = *channels;

  // NOTE: We init sound after boot is complete to avoid an initial
  // sound sync issue if a cartridge is attached. But if it's already
  // initialised, cancel and restart here in case channels has changed.
  if (mViceSound) {
     mViceSound->CancelPlayback();
     mViceSound->Playback(vol_percent_to_vchiq(mVolume), mNumSoundChannels);
  }
  return 0;
}

// Called from VICE: Core 1
int CKernel::circle_sound_write(int16_t *pbuf, size_t nr) {
  if (mViceSound) {
    return mViceSound->AddChunk(pbuf, nr);
  }
  return 0;
}

void CKernel::circle_sound_close(void) {
  // Nothing to do here since we never actually close vc4.
}

int CKernel::circle_sound_suspend(void) { return 0; }

int CKernel::circle_sound_resume(void) { return 0; }

int CKernel::circle_sound_bufferspace(void) {
  if (mViceSound) {
    return mViceSound->BufferSpaceSamples();
  }
  return FRAG_SIZE * NUM_FRAGS;
}

void CKernel::circle_yield(void) { CScheduler::Get()->Yield(); }

void CKernel::MouseStatusHandler(unsigned nButtons, int deltaX, int deltaY) {
  static unsigned int prev_buttons = {0};

  emu_mouse_move(deltaX, deltaY);

  if ((prev_buttons & MOUSE_BUTTON_LEFT) && !(nButtons & MOUSE_BUTTON_LEFT)) {
    emu_mouse_button_left(0);
  } else if (!(prev_buttons & MOUSE_BUTTON_LEFT) &&
             (nButtons & MOUSE_BUTTON_LEFT)) {
    emu_mouse_button_left(1);
  }
  if ((prev_buttons & MOUSE_BUTTON_RIGHT) && !(nButtons & MOUSE_BUTTON_RIGHT)) {
    emu_mouse_button_right(0);
  } else if (!(prev_buttons & MOUSE_BUTTON_RIGHT) &&
             (nButtons & MOUSE_BUTTON_RIGHT)) {
    emu_mouse_button_right(1);
  }
  prev_buttons = nButtons;
}

void CKernel::KeyStatusHandlerRaw(unsigned char ucModifiers,
                                  const unsigned char RawKeys[6]) {

  bool new_states[MAX_KEY_CODES];
  memset(new_states, 0, MAX_KEY_CODES * sizeof(bool));

  // Compare previous to present and handle press/release that come from
  // modifier keys.
  int v = 1;
  for (int i = 0; i < 8; i++) {
    if ((ucModifiers & v) && !(mod_states & v)) {
      switch (i) {
      case 0: // LeftControl
        emu_key_pressed(KEYCODE_LeftControl);
        break;
      case 4: // RightControl
        emu_key_pressed(KEYCODE_RightControl);
        break;
      case 1: // LeftShift
        if (emu_is_ui_activated()) {
          uiLeftShift = true;
        }
        emu_key_pressed(KEYCODE_LeftShift);
        break;
      case 5: // RightShift
        if (emu_is_ui_activated()) {
          uiRightShift = true;
        }
        emu_key_pressed(KEYCODE_RightShift);
        break;
      case 3: // LeftSuper
        emu_key_pressed(KEYCODE_LeftSuper);
        break;
      case 2: // LeftAlt
        emu_key_pressed(KEYCODE_LeftAlt);
        break;
      case 6: // RightAlt
        emu_key_pressed(KEYCODE_RightAlt);
        break;
      default:
        break;
      }
    } else if (!(ucModifiers & v) && (mod_states & v)) {
      switch (i) {
      case 0: // LeftControl
        emu_key_released(KEYCODE_LeftControl);
        break;
      case 4: // RightControl
        emu_key_released(KEYCODE_RightControl);
        break;
      case 1: // LeftShift
        if (emu_is_ui_activated()) {
          uiLeftShift = false;
        }
        emu_key_released(KEYCODE_LeftShift);
        break;
      case 5: // RightShift
        if (emu_is_ui_activated()) {
          uiRightShift = false;
        }
        emu_key_released(KEYCODE_RightShift);
        break;
      case 3: // LeftSuper
        emu_key_released(KEYCODE_LeftSuper);
        break;
      case 2: // LeftAlt
        emu_key_released(KEYCODE_LeftAlt);
        break;
      case 6: // RightAlt
        emu_key_released(KEYCODE_RightAlt);
        break;
      default:
        break;
      }
    }
    v = v * 2;
  }
  mod_states = ucModifiers;

  // Set new states
  for (unsigned i = 0; i < 6; i++) {
    const unsigned char key = RawKeys[i];
    if (key != 0) {
      new_states[key] = true;
    }
  }

  // Compare previous to present and handle key press/release events.
  int ui_activated = emu_is_ui_activated();
  for (unsigned i = 1; i < MAX_KEY_CODES; i++) {
    if (key_states[i] == true && new_states[i] == false) {
      if (ui_activated) {
        // We have to handle shift+left/right here or else our ui
        // isn't navigable by keyrah with real C64 board. Keep
        // key_states below managing the state of the original key,
        // not the translated one.
        if ((uiLeftShift || uiRightShift) && i == KEYCODE_Right) {
          emu_key_released(KEYCODE_Left);
        } else if ((uiLeftShift || uiRightShift) && i == KEYCODE_Down) {
          emu_key_released(KEYCODE_Up);
        } else {
          emu_key_released(i);
        }
      } else {
        emu_key_released(i);
      }
    } else if (key_states[i] == false && new_states[i] == true) {
      if (ui_activated) {
        // See above note on shift.
        if ((uiLeftShift || uiRightShift) && i == KEYCODE_Right) {
          emu_key_pressed(KEYCODE_Left);
        } else if ((uiLeftShift || uiRightShift) && i == KEYCODE_Down) {
          emu_key_pressed(KEYCODE_Up);
        } else {
          emu_key_pressed(i);
        }
      } else {
        emu_key_pressed(i);
      }
    }
    key_states[i] = new_states[i];
  }
}

int CKernel::ReadDebounced(int pinIndex) {
  CGPIOPin *pin = gpioPins[pinIndex];

  if (gpio_debounce_state[pinIndex] == BTN_PRESS) {
    gpio_debounce_state[pinIndex] = BTN_DOWN;
  } else if (gpio_debounce_state[pinIndex] == BTN_RELEASE) {
    gpio_debounce_state[pinIndex] = BTN_UP;
  }

  if (pin->Read() == LOW) {
    if (gpio_debounce_state[pinIndex] == BTN_UP) {
      circle_sleep(5);
      if (pin->Read() == LOW) {
        gpio_debounce_state[pinIndex] = BTN_PRESS;
      }
    }
  } else {
    if (gpio_debounce_state[pinIndex] == BTN_DOWN) {
      if (pin->Read() == HIGH) {
        circle_sleep(5);
        if (pin->Read() == HIGH) {
          gpio_debounce_state[pinIndex] = BTN_RELEASE;
        }
      }
    }
  }
  return gpio_debounce_state[pinIndex];
}

// Called from main emulation loop before pending event queues are
// drained. Checks whether any of our gpio pins have triggered some
// function. Also scans a real C64 keyboard and joysticks if enabled.
// Otherwise, just scans gpio joysticks.
void CKernel::circle_check_gpio() {

  // TODO: Find a better place for this. Piggy back on emulation loop
  // to initialize sound when helper cores were late initializing the sid
  // tables.
#ifdef ARM_ALLOW_MULTI_CORE
  circle_lock_acquire();
  if (mNeedSoundInit && mNumCoresComplete >= 2) {
     mViceSound = new ViceSound(&mVCHIQ, mViceOptions.GetAudioOut());
     mViceSound->Playback(vol_percent_to_vchiq(mVolume), mNumSoundChannels);
     mNeedSoundInit = false;
  }
  circle_lock_release();
#endif

  int gpio_config = emu_get_gpio_config();

  switch(gpio_config) {
    case GPIO_CONFIG_NAV_JOY:
     // Nav Buttons + Real Joys
     if (ReadDebounced(GPIO_CONFIG_0_MENU_INDEX) == BTN_PRESS) {
      emu_key_pressed(KEYCODE_F12);
      emu_key_released(KEYCODE_F12);
     }
     if (ReadDebounced(GPIO_CONFIG_0_MENU_BACK_INDEX) == BTN_PRESS) {
      emu_key_pressed(KEYCODE_Escape);
      emu_key_released(KEYCODE_Escape);
     }
     if (ReadDebounced(GPIO_CONFIG_0_MENU_UP_INDEX) == BTN_PRESS) {
      emu_key_pressed(KEYCODE_Up);
      emu_key_released(KEYCODE_Up);
     }
     if (ReadDebounced(GPIO_CONFIG_0_MENU_DOWN_INDEX) == BTN_PRESS) {
      emu_key_pressed(KEYCODE_Down);
      emu_key_released(KEYCODE_Down);
     }
     if (ReadDebounced(GPIO_CONFIG_0_MENU_LEFT_INDEX) == BTN_PRESS) {
      emu_key_pressed(KEYCODE_Left);
      emu_key_released(KEYCODE_Left);
     }
     if (ReadDebounced(GPIO_CONFIG_0_MENU_RIGHT_INDEX) == BTN_PRESS) {
      emu_key_pressed(KEYCODE_Right);
      emu_key_released(KEYCODE_Right);
     }
     if (ReadDebounced(GPIO_CONFIG_0_MENU_ENTER_INDEX) == BTN_PRESS) {
      emu_key_pressed(KEYCODE_Return);
      emu_key_released(KEYCODE_Return);
     }
     if (ReadDebounced(GPIO_CONFIG_0_MENU_VKBD_INDEX) == BTN_PRESS) {
      emu_quick_func_interrupt(BTN_ASSIGN_VKBD_TOGGLE);
     }
     ReadJoystick(0, GPIO_CONFIG_NAV_JOY);
     ReadJoystick(1, GPIO_CONFIG_NAV_JOY);
     break;
    case GPIO_CONFIG_KYB_JOY:
     // Real Kyb + Joys
     ScanKeyboard();
     ReadJoystick(0, GPIO_CONFIG_KYB_JOY);
     ReadJoystick(1, GPIO_CONFIG_KYB_JOY);
     break;
    case GPIO_CONFIG_WAVESHARE:
     // Waveshare Hat
     if (ReadDebounced(GPIO_CONFIG_2_WAVESHARE_START_INDEX) == BTN_PRESS) {
       emu_key_pressed(KEYCODE_F12);
       emu_key_released(KEYCODE_F12);
     }
     if (ReadDebounced(GPIO_CONFIG_2_WAVESHARE_TL_INDEX) == BTN_PRESS) {
       emu_key_pressed(KEYCODE_Escape);
       emu_key_released(KEYCODE_Escape);
     }
     if (ReadDebounced(GPIO_CONFIG_2_WAVESHARE_TR_INDEX) == BTN_PRESS) {
       emu_quick_func_interrupt(BTN_ASSIGN_WARP);
     }
     if (ReadDebounced(GPIO_CONFIG_2_WAVESHARE_X_INDEX) == BTN_PRESS) {
       emu_quick_func_interrupt(BTN_ASSIGN_VKBD_TOGGLE);
     }
     if (ReadDebounced(GPIO_CONFIG_2_WAVESHARE_SELECT_INDEX) == BTN_PRESS) {
       emu_quick_func_interrupt(BTN_ASSIGN_STATUS_TOGGLE);
     }
     ReadJoystick(0, GPIO_CONFIG_WAVESHARE);
     break;
    case GPIO_CONFIG_USERPORT:
     SetupUserport();
     ReadWriteUserport();
     ReadJoystick(0, GPIO_CONFIG_USERPORT);
     ReadJoystick(1, GPIO_CONFIG_USERPORT);
     break;
    case GPIO_CONFIG_CUSTOM:
     ReadCustomGPIO();
     break;
    default:
     // Disabled
     break;
  }
}

// Reset the state of the GPIO pins.
// Needed when switching to and from GPIO_CONFIG_USERPORT
void CKernel::circle_reset_gpio(int gpio_config) {
  switch (gpio_config) {
    case GPIO_CONFIG_NAV_JOY:
    case GPIO_CONFIG_KYB_JOY:
    case GPIO_CONFIG_WAVESHARE:
    case GPIO_CONFIG_CUSTOM:
      // Joystick and keyboard settings require all ports
      // to be inputs
      for (int i = 0; i < NUM_GPIO_PINS; i++) {
        gpioPins[i]->SetMode(GPIOModeInputPullUp);
      }
      break;
    case GPIO_CONFIG_USERPORT:
      for (int i = 0; i < 5; i++) {
        config_3_joystickPins1[i]->SetMode(GPIOModeInputPullUp);
        config_3_joystickPins2[i]->SetMode(GPIOModeInputPullUp);
      }
      SetupUserport();
      break;
    default:
      // Disabled
      break;
  }
}

void CKernel::circle_lock_acquire() { m_Lock.Acquire(); }

void CKernel::circle_lock_release() { m_Lock.Release(); }

void CKernel::circle_boot_complete() {
  // NOTE: We init the sound device here to avoid a sound sync
  // issue if a cartridge is attached.  If this is done too
  // early, the sound data consumer is a bit further behind.
  if (!mViceSound) {
#ifdef ARM_ALLOW_MULTI_CORE
    circle_lock_acquire();
    if (mNumCoresComplete >= 2) {
       // Cores 1/2 are done initing sound tables before we tried to
       // start playback device.
       mViceSound = new ViceSound(&mVCHIQ, mViceOptions.GetAudioOut());
       mViceSound->Playback(vol_percent_to_vchiq(mVolume), mNumSoundChannels);
    } else {
       // Cores 1/2 are still initializing sound tables. We'll init
       // sound later.  This is to get around the crashing noise you
       // can get on boot if you have a cartridge attached.
       mNeedSoundInit = true;
    }
    circle_lock_release();
#else
    mViceSound = new ViceSound(&mVCHIQ, mViceOptions.GetAudioOut());
    mViceSound->Playback(vol_percent_to_vchiq(mVolume), mNumSoundChannels);
#endif
  }

  DisableBootStat();
}

int CKernel::circle_alloc_fbl(int layer, int pixelmode, uint8_t **pixels,
                              int width, int height, int *pitch) {
  return fbl[layer].Allocate(pixelmode, pixels, width, height, pitch);
}

int CKernel::circle_realloc_fbl(int layer, int shader) {
  return fbl[layer].ReAllocate(shader);
}

void CKernel::circle_free_fbl(int layer) {
  fbl[layer].Free();
}

void CKernel::circle_clear_fbl(int layer) {
  fbl[layer].Clear();
}

void CKernel::circle_show_fbl(int layer) {
  fbl[layer].Show();
}

void CKernel::circle_hide_fbl(int layer) {
  fbl[layer].Hide();
}

void CKernel::circle_frames_ready_fbl(int layer1, int layer2, int sync) {
  // If we're going to sync to vblank, indicate this frame data should go
  // to the offscreen resource.
  fbl[layer1].FrameReady(sync);
  if (layer2 >= 0) {
     fbl[layer2].FrameReady(sync);
  }
  // Flip the buffers and wait for vblank.
  FrameBufferLayer::SwapResources(sync,
      &fbl[layer1], layer2 >= 0 ? &fbl[layer2] : nullptr);
}

void CKernel::circle_set_palette_fbl(int layer, uint8_t index, uint16_t rgb565) {
  fbl[layer].SetPalette(index, rgb565);
}

void CKernel::circle_set_palette32_fbl(int layer, uint8_t index, uint32_t argb) {
  fbl[layer].SetPalette(index, argb);
}

void CKernel::circle_update_palette_fbl(int layer) {
  fbl[layer].UpdatePalette();
}

void CKernel::circle_set_stretch_fbl(int layer, double hstretch, double vstretch, int hintstr, int vintstr, int use_hintstr, int use_vintstr) {
  fbl[layer].SetStretch(hstretch, vstretch, hintstr, vintstr, use_hintstr, use_vintstr);
}

void CKernel::circle_set_center_offset(int layer, int cx, int cy) {
  fbl[layer].SetCenterOffset(cx, cy);
}

void CKernel::circle_set_src_rect_fbl(int layer, int x, int y, int w, int h) {
  fbl[layer].SetSrcRect(x,y,w,h);
}

void CKernel::circle_set_valign_fbl(int layer, int align, int padding) {
  fbl[layer].SetVerticalAlignment(align, padding);
}

void CKernel::circle_set_halign_fbl(int layer, int align, int padding) {
  fbl[layer].SetHorizontalAlignment(align, padding);
}

void CKernel::circle_set_padding_fbl(int layer, double lpad, double rpad, double tpad, double bpad) {
  fbl[layer].SetPadding(lpad, rpad, tpad, bpad);
}

void CKernel::circle_set_zlayer_fbl(int layer, int zlayer) {
  fbl[layer].SetLayer(zlayer);
}

int CKernel::circle_get_zlayer_fbl(int layer) {
  return fbl[layer].GetLayer();
}

void CKernel::circle_set_volume(int value) {
  // TODO: This is a race condition between two cores. Fix this.
  mVolume = value;
  if (mViceSound) {
     mViceSound->SetControl(vol_percent_to_vchiq(value),
                            mViceOptions.GetAudioOut());
  }
}

int CKernel::circle_get_model() {
  return mMachineInfo.GetModelMajor();
}

unsigned CKernel::circle_get_arm_clock() {
  return mMachineInfo.GetClockRate(CLOCK_ID_ARM);
}

int CKernel::circle_gpio_enabled() {
  // When DPI is enabled, GPIO scanning must be disabled.
  return mViceOptions.DPIEnabled() == 0;
}

int CKernel::circle_gpio_outputs_enabled() {
  return !mViceOptions.DPIEnabled() && mViceOptions.GPIOOutputsEnabled();
}

// Called by cores 1 and 2 after they are done initializing
// sid tables.  Used to know whether volume should be set to
// 0 or requested initial volume after boot.
void CKernel::circle_kernel_core_init_complete(int core) {
  circle_lock_acquire();
  mNumCoresComplete++;
  circle_lock_release();
}

void CKernel::circle_get_fbl_dimensions(int layer,
                               int *display_w, int *display_h,
                               int *fb_w, int *fb_h,
                               int *src_w, int *src_h,
                               int *dst_w, int *dst_h) {
  return fbl[layer].GetDimensions(display_w, display_h,
                                  fb_w, fb_h,
                                  src_w, src_h,
                                  dst_w, dst_h);
}

void CKernel::circle_get_scaling_params(int display,
                                        int *fbw, int *fbh,
                                        int *sx, int *sy) {
  mViceOptions.GetScalingParams(display, fbw, fbh, sx, sy);
}

void CKernel::circle_set_interpolation(int enable) {
  FrameBufferLayer::SetInterpolation(enable);
}

void CKernel::circle_set_use_shader(int enable) {
	// Only the main display (layer 0) ever gets a shader.
  fbl[0].SetUsesShader(enable);
}

void CKernel::circle_set_shader_params(int curvature,
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
                int bilinear_interpolation) {
  // Only the main display (layer 0) ever gets a shader.
  fbl[0].SetShaderParams(curvature,
			curvature_x,
			curvature_y,
			mask,
			mask_brightness,
			gamma,
			fake_gamma,
			scanlines,
			multisample,
			scanline_weight,
			scanline_gap_brightness,
			bloom_factor,
			input_gamma,
			output_gamma,
			sharper,
                        bilinear_interpolation);
}
