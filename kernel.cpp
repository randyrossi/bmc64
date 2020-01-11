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
 {KEYCODE_Backspace,  KEYCODE_3,         KEYCODE_5, KEYCODE_7, KEYCODE_9, KEYCODE_Down,         KEYCODE_Left,         KEYCODE_1},
 {KEYCODE_Return,     KEYCODE_w,         KEYCODE_r, KEYCODE_y, KEYCODE_i, KEYCODE_p,            KEYCODE_Dash,         KEYCODE_BackQuote},
 {KEYCODE_BackSlash,  KEYCODE_a,         KEYCODE_d, KEYCODE_g, KEYCODE_j, KEYCODE_l,            KEYCODE_SingleQuote,  KEYCODE_Tab},
 {KEYCODE_F7,         KEYCODE_4,         KEYCODE_6, KEYCODE_8, KEYCODE_0, KEYCODE_Up,           KEYCODE_Right,        KEYCODE_2},
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

void circle_set_aspect_fbl(int layer, double aspect) {
  static_kernel->circle_set_aspect_fbl(layer, aspect);
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

int circle_gpio_enabled() {
  return static_kernel->circle_gpio_enabled();
}
};

CKernel::CKernel(void)
    : ViceStdioApp("vice"), mViceSound(nullptr),
      mNumJoy(emu_get_num_joysticks()),
      mInitialVolume(100) {
  static_kernel = this;
  mod_states = 0;
  memset(key_states, 0, MAX_KEY_CODES * sizeof(bool));

  // Only used for pins that are used as buttons. See viceapp.h.
  for (int i = 0; i < NUM_GPIO_PINS; i++) {
    gpio_debounce_state[i] = BTN_UP;
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
}

bool CKernel::Initialize(void) {
  if (!ViceStdioApp::Initialize()) {
    return false;
  }

  return true;
}

// KEEP THIS IN SYNC WITH kbd.c
static void handle_button_function(bool is_ui, int device, unsigned buttons) {
  int button_num = 0;

  int button_func;
  int is_press;

  while (emu_button_function(device, button_num, buttons,
                             &button_func, &is_press) >= 0) {
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
     case BTN_ASSIGN_RESET_HARD:
     case BTN_ASSIGN_RESET_SOFT:
     case BTN_ASSIGN_ACTIVE_DISPLAY:
     case BTN_ASSIGN_PIP_LOCATION:
     case BTN_ASSIGN_PIP_SWAP:
     case BTN_ASSIGN_40_80_COLUMN:
     case BTN_ASSIGN_VKBD_TOGGLE:
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
     case BTN_ASSIGN_FIRE:
       // Only need to handle ui fire here. Actual joy fire is
       // handled in circle_add_usb_values.
       if (is_ui) {
         emu_ui_key_interrupt(KEYCODE_Return, is_press);
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
     // Only do direction button assignments for UI, joy is handled
     // in circle_add_usb_values seperately.
     case BTN_ASSIGN_UP:
       if (is_ui) {
         emu_ui_key_interrupt(KEYCODE_Up, is_press);
       }
       break;
     case BTN_ASSIGN_DOWN:
       if (is_ui) {
         emu_ui_key_interrupt(KEYCODE_Down, is_press);
       }
       break;
     case BTN_ASSIGN_LEFT:
       if (is_ui) {
         emu_ui_key_interrupt(KEYCODE_Left, is_press);
       }
       break;
     case BTN_ASSIGN_RIGHT:
       if (is_ui) {
         emu_ui_key_interrupt(KEYCODE_Right, is_press);
       }
       break;
     default:
       break;
   }

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

  if (usb_pref == USB_PREF_HAT && pState->nhats > 0) {
    int dpad = pState->hats[0];
    bool has_changed =
        (prev_buttons[nDeviceIndex] != b) || (prev_dpad[nDeviceIndex] != dpad);
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

ViceApp::TShutdownMode CKernel::Run(void) {
  SetupUSBKeyboard();
  SetupUSBMouse();

  emu_set_demo_mode(mViceOptions.GetDemoMode());

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

  // For restore, there is no public API that triggers it so we will
  // pass the keycode that will.
  int restore = gpioPins[GPIO_KBD_RESTORE_INDEX]->Read();
#if defined(RASPI_PLUS4) | defined(RASPI_PLUS4EMU)
  if (restore == LOW && kbdRestoreState == HIGH) {
     emu_key_pressed(KEYCODE_Home);
  } else if (restore == HIGH && kbdRestoreState == LOW) {
     emu_key_released(KEYCODE_Home);
  }
#else
  if (restore == LOW && kbdRestoreState == HIGH) {
     emu_key_pressed(KEYCODE_PageUp);
  } else if (restore == HIGH && kbdRestoreState == LOW) {
     emu_key_released(KEYCODE_PageUp);
  }
#endif
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

// Configure CIA2 port B from DDR
// Only configured for C64 for now.
void CKernel::SetupUserport() {
  uint8_t ddr = circle_get_userport_ddr();
  for (int i = 0; i < 8; i++) {
    uint8_t bit_pos = 1<<i;
    uint8_t ddr_value = ddr & bit_pos;
    config_3_userportPins[i]->SetMode(ddr_value ? GPIOModeOutput : GPIOModeInputPullUp);
  }
}

// Read input pins and send to output pins
// Only configured for C64 for now.
void CKernel::ReadWriteUserport() {
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

int CKernel::circle_get_machine_timing() {
  // See circle.h for valid values
  return mViceOptions.GetMachineTiming();
}

void CKernel::circle_sleep(long delay) { mTimer.SimpleusDelay(delay); }

unsigned long CKernel::circle_get_ticks() { return mTimer.GetClockTicks(); }

// Called from VICE: Core 1
int CKernel::circle_sound_init(const char *param, int *speed, int *fragsize,
                               int *fragnr, int *channels) {
  *speed = SAMPLE_RATE;
  *fragsize = FRAG_SIZE;
  *fragnr = NUM_FRAGS;
  // We force mono.
  *channels = 1;

  if (!mViceSound) {
    mViceSound = new ViceSound(&mVCHIQ, mViceOptions.GetAudioOut());
    mViceSound->Playback(vol_percent_to_vchiq(mInitialVolume));
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
    return mViceSound->BufferSpaceBytes();
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
  if (pin->Read() == LOW) {
    if (gpio_debounce_state[pinIndex] == BTN_PRESS) {
      gpio_debounce_state[pinIndex] = BTN_DOWN;
    }
    if (gpio_debounce_state[pinIndex] == BTN_UP) {
      circle_sleep(5);
      if (pin->Read() == LOW) {
        gpio_debounce_state[pinIndex] = BTN_PRESS;
      }
    }
  } else {
    if (gpio_debounce_state[pinIndex] == BTN_RELEASE) {
      gpio_debounce_state[pinIndex] = BTN_UP;
    }
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
     // CIA2 port B
     SetupUserport();
     ReadWriteUserport();
     ReadJoystick(0, GPIO_CONFIG_USERPORT);
     ReadJoystick(1, GPIO_CONFIG_USERPORT);
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

void CKernel::circle_boot_complete() { DisableBootStat(); }

#if defined(RASPI_PLUS4) | defined(RASPI_PLUS4EMU)
int CKernel::circle_cycles_per_second() {
  int timing = circle_get_machine_timing();
  if (timing == MACHINE_TIMING_NTSC_HDMI || timing == MACHINE_TIMING_NTSC_DPI) {
    // 60hz
    return 1792080;
  } else if (circle_get_machine_timing() == MACHINE_TIMING_NTSC_COMPOSITE) {
    // Actual C64's NTSC Composite frequency is 59.826 but the Pi's vertical
    // sync frequency on composite is 60.053. See c64.h for how this is
    // calculated. This keeps audio buffer to a minimum using ReSid.
    return 1793672;
  } else if (timing == MACHINE_TIMING_NTSC_CUSTOM_HDMI || timing == MACHINE_TIMING_NTSC_CUSTOM_DPI) {
    return mViceOptions.GetCyclesPerSecond();
  } else if (timing == MACHINE_TIMING_PAL_HDMI) {
    // 50hz
    return 1778400;
  } else if (timing == MACHINE_TIMING_PAL_COMPOSITE) {
    // Actual C64's PAL Composite frequency is 50.125 but the Pi's vertical
    // sync frequency on composite is 50.0816. See c64.h for how this is
    // calculated.  This keep audio buffer to a minimum using ReSid.
    return 1781245;
  } else if (timing == MACHINE_TIMING_PAL_CUSTOM_HDMI || timing == MACHINE_TIMING_PAL_CUSTOM_DPI) {
    return mViceOptions.GetCyclesPerSecond();
  } else {
    return 1778400;
  }
}
#elif defined(RASPI_VIC20)
int CKernel::circle_cycles_per_second() {
  int timing = circle_get_machine_timing();
  if (timing == MACHINE_TIMING_NTSC_HDMI) {
    // 60hz
    return 1017900;
  } else if (timing == MACHINE_TIMING_NTSC_COMPOSITE) {
    // Actual C64's NTSC Composite frequency is 59.826 but the Pi's vertical
    // sync frequency on composite is 60.053. See c64.h for how this is
    // calculated. This keeps audio buffer to a minimum using ReSid.
    return 1018804;
  } else if (timing == MACHINE_TIMING_NTSC_CUSTOM_HDMI || timing == MACHINE_TIMING_NTSC_CUSTOM_DPI) {
    return mViceOptions.GetCyclesPerSecond();
  } else if (timing == MACHINE_TIMING_PAL_HDMI) {
    // 50hz
    return 1107600;
  } else if (timing == MACHINE_TIMING_PAL_COMPOSITE) {
    // Actual C64's PAL Composite frequency is 50.125 but the Pi's vertical
    // sync frequency on composite is 50.0816. See c64.h for how this is
    // calculated.  This keep audio buffer to a minimum using ReSid.
    return 1109372;
  } else if (timing == MACHINE_TIMING_PAL_CUSTOM_HDMI || timing == MACHINE_TIMING_PAL_CUSTOM_DPI) {
    return mViceOptions.GetCyclesPerSecond();
  } else {
    return 1017900;
  }
}
#elif defined(RASPI_C64) | defined(RASPI_C128)
int CKernel::circle_cycles_per_second() {
  int timing = circle_get_machine_timing();
  if (timing == MACHINE_TIMING_NTSC_HDMI) {
    // 60hz
    return 1025700;
  } else if (timing == MACHINE_TIMING_NTSC_COMPOSITE) {
    // Actual C64's NTSC Composite frequency is 59.826 but the Pi's vertical
    // sync frequency on composite is 60.053. See c64.h for how this is
    // calculated. This keeps audio buffer to a minimum using ReSid.
    return 1026611;
  } else if (timing == MACHINE_TIMING_NTSC_CUSTOM_HDMI || timing == MACHINE_TIMING_NTSC_CUSTOM_DPI) {
    return mViceOptions.GetCyclesPerSecond();
  } else if (timing == MACHINE_TIMING_PAL_HDMI) {
    // 50hz
    return 982800;
  } else if (timing == MACHINE_TIMING_PAL_COMPOSITE) {
    // Actual C64's PAL Composite frequency is 50.125 but the Pi's vertical
    // sync frequency on composite is 50.0816. See c64.h for how this is
    // calculated.  This keep audio buffer to a minimum using ReSid.
    return 984404;
  } else if (timing == MACHINE_TIMING_PAL_CUSTOM_HDMI || timing == MACHINE_TIMING_PAL_CUSTOM_DPI) {
    return mViceOptions.GetCyclesPerSecond();
  } else {
    return 982800;
  }
}
#elif defined(RASPI_PET)
int CKernel::circle_cycles_per_second() {
  int timing = circle_get_machine_timing();
  if (timing == MACHINE_TIMING_NTSC_HDMI) {
    // 60hz
    return 1013760;
  } else if (timing == MACHINE_TIMING_NTSC_COMPOSITE) {
    // Actual C64's NTSC Composite frequency is 59.826 but the Pi's vertical
    // sync frequency on composite is 60.053. See c64.h for how this is
    // calculated. This keeps audio buffer to a minimum using ReSid.
    return 1014661;
  } else if (timing == MACHINE_TIMING_NTSC_CUSTOM_HDMI || timing == MACHINE_TIMING_NTSC_CUSTOM_DPI) {
    return mViceOptions.GetCyclesPerSecond();
  } else if (timing == MACHINE_TIMING_PAL_HDMI) {
    // 50hz
    return 1001600;
  } else if (timing == MACHINE_TIMING_PAL_COMPOSITE) {
    // Actual C64's PAL Composite frequency is 50.125 but the Pi's vertical
    // sync frequency on composite is 50.0816. See c64.h for how this is
    // calculated.  This keep audio buffer to a minimum using ReSid.
    return 1003202;
  } else if (timing == MACHINE_TIMING_PAL_CUSTOM_HDMI || timing == MACHINE_TIMING_PAL_CUSTOM_DPI) {
    return mViceOptions.GetCyclesPerSecond();
  } else {
    return 1000000;
  }
}
#else
  #error Unknown RASPI_ variant
#endif

int CKernel::circle_alloc_fbl(int layer, int pixelmode, uint8_t **pixels,
                              int width, int height, int *pitch) {
  return fbl[layer].Allocate(pixelmode, pixels, width, height, pitch);
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
  if (sync) {
     // Flip the buffers and wait for vblank.
     FrameBufferLayer::SwapResources(
         &fbl[layer1], layer2 >= 0 ? &fbl[layer2] : nullptr);
  }
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

void CKernel::circle_set_aspect_fbl(int layer, double aspect) {
  fbl[layer].SetAspect(aspect);
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
  if (mViceSound) {
     mViceSound->SetControl(vol_percent_to_vchiq(value),
                            mViceOptions.GetAudioOut());
  } else {
     mInitialVolume = value;
  }
}

int CKernel::circle_get_model() {
  return mMachineInfo.GetModelMajor();
}

int CKernel::circle_gpio_enabled() {
  // When DPI is enabled, GPIO scanning must be disabled.
  return mViceOptions.DPIEnabled() == 0;
}
