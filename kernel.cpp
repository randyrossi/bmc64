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

// Usb key states
static bool key_states[MAX_KEY_CODES];
static unsigned char mod_states;
static bool uiLeftShift = false;
static bool uiRightShift = false;

// Real C64 keyboard matrix states
static bool kbdMatrixStates[8][8];
// These are only for translating row/col scans into equivalent USB codes
// for the ui.
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

int circle_alloc_fb2(int layer, uint8_t **pixels,
                     int width, int height, int *pitch) {
  return static_kernel->circle_alloc_fb2(layer, pixels, width, height, pitch);
}

void circle_free_fb2(int layer) {
  static_kernel->circle_free_fb2(layer);
}

void circle_clear_fb2(int layer) {
  static_kernel->circle_clear_fb2(layer);
}

void circle_show_fb2(int layer) {
  static_kernel->circle_show_fb2(layer);
}

void circle_hide_fb2(int layer) {
  static_kernel->circle_hide_fb2(layer);
}

void circle_frames_ready_fb2(int layer1, int layer2, int sync) {
  static_kernel->circle_frames_ready_fb2(layer1, layer2, sync);
}

void circle_set_palette_fb2(int layer, uint8_t index, uint16_t rgb565) {
  static_kernel->circle_set_palette_fb2(layer, index, rgb565);
}

void circle_update_palette_fb2(int layer) {
  static_kernel->circle_update_palette_fb2(layer);
}

void circle_set_aspect_fb2(int layer, double aspect) {
  static_kernel->circle_set_aspect_fb2(layer, aspect);
}

void circle_set_src_rect_fb2(int layer, int x, int y, int w, int h) {
  static_kernel->circle_set_src_rect_fb2(layer, x,y,w,h);
}

void circle_set_valign_fb2(int layer, int align, int padding) {
  static_kernel->circle_set_valign_fb2(layer, align, padding);
}

void circle_set_halign_fb2(int layer, int align, int padding) {
  static_kernel->circle_set_halign_fb2(layer, align, padding);
}

void circle_set_padding_fb2(int layer, double lpad, double rpad, double tpad, double bpad) {
  static_kernel->circle_set_padding_fb2(layer, lpad, rpad, tpad, bpad);
}

void circle_set_zlayer_fb2(int layer, int zlayer) {
  static_kernel->circle_set_zlayer_fb2(layer, zlayer);
}

int circle_get_zlayer_fb2(int layer) {
  return static_kernel->circle_get_zlayer_fb2(layer);
}

};


CKernel::CKernel(void)
    : ViceStdioApp("vice"), mViceSound(nullptr),
      mNumJoy(circle_num_joysticks()) {
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

  fb2[FB_LAYER_VIC].SetLayer(0);
  fb2[FB_LAYER_VIC].SetTransparency(false);

  fb2[FB_LAYER_VDC].SetLayer(1);
  fb2[FB_LAYER_VDC].SetTransparency(false);

  fb2[FB_LAYER_STATUS].SetLayer(2);
  fb2[FB_LAYER_STATUS].SetTransparency(true);

  fb2[FB_LAYER_UI].SetLayer(3);
  fb2[FB_LAYER_UI].SetTransparency(true);
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

  while (circle_button_function(device, button_num, buttons,
                                &button_func, &is_press) >= 0) {
   // KEEP THIS IN SYNC WITH kbd.c
   switch (button_func) {
     case BTN_ASSIGN_MENU:
       if (is_press) {
          circle_key_pressed(KEYCODE_F12);
       } else {
          circle_key_released(KEYCODE_F12);
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
       if (is_press) {
          circle_emu_quick_func_interrupt(button_func);
       }
       break;
     case BTN_ASSIGN_CUSTOM_KEY_1:
     case BTN_ASSIGN_CUSTOM_KEY_2:
     case BTN_ASSIGN_CUSTOM_KEY_3:
     case BTN_ASSIGN_CUSTOM_KEY_4:
     case BTN_ASSIGN_CUSTOM_KEY_5:
     case BTN_ASSIGN_CUSTOM_KEY_6:
        if (is_press) {
           circle_key_pressed(
               circle_key_binding(button_func - BTN_ASSIGN_CUSTOM_KEY_1));
        } else {
           circle_key_released(
               circle_key_binding(button_func - BTN_ASSIGN_CUSTOM_KEY_1));
        }
        break;
     case BTN_ASSIGN_FIRE:
       // Only need to handle ui fire here. Actual joy fire is
       // handled in circle_add_usb_values.
       if (is_ui) {
         circle_ui_key_interrupt(KEYCODE_Return, is_press);
       }
       break;
     case BTN_ASSIGN_RUN_STOP_BACK:
       if (is_ui) {
         circle_ui_key_interrupt(KEYCODE_Escape, is_press);
       } else {
         if (is_press) {
            circle_key_pressed(KEYCODE_Escape);
         } else {
            circle_key_released(KEYCODE_Escape);
         }
       }
       break;
     // Only do direction button assignments for UI, joy is handled
     // in circle_add_usb_values seperately.
     case BTN_ASSIGN_UP:
       if (is_ui) {
         circle_ui_key_interrupt(KEYCODE_Up, is_press);
       }
       break;
     case BTN_ASSIGN_DOWN:
       if (is_ui) {
         circle_ui_key_interrupt(KEYCODE_Down, is_press);
       }
       break;
     case BTN_ASSIGN_LEFT:
       if (is_ui) {
         circle_ui_key_interrupt(KEYCODE_Left, is_press);
       }
       break;
     case BTN_ASSIGN_RIGHT:
       if (is_ui) {
         circle_ui_key_interrupt(KEYCODE_Right, is_press);
       }
       break;
     default:
       break;
   }

   button_num++;
  }
}

// Interrupt handler. Make this quick.
void CKernel::GamePadStatusHandler(unsigned nDeviceIndex,
                                   const TGamePadState *pState) {

  static int dpad_to_joy[8] = {0x01, 0x09, 0x08, 0x0a, 0x02, 0x06, 0x04, 0x05};

  static unsigned int prev_buttons[2] = {0, 0};
  static int prev_dpad[2] = {8, 8};
  static int prev_axes_dirs[2][12] = {{0, 0, 0, 0}, {0, 0, 0, 0}};

  if (nDeviceIndex >= 2)
    return;

  if (menu_wants_raw_usb()) {
    // Send the raw usb data and we're done.
    int axes[16];
    for (int i = 0; i < pState->naxes; i++) {
      axes[i] = pState->axes[i].value;
    }
    menu_raw_usb(nDeviceIndex, pState->buttons, pState->hats, axes);
    return;
  }

  int ui_activated = circle_ui_activated();

  unsigned b = pState->buttons;

  // usb_pref is the value of the usb pref menu item
  int usb_pref;
  int axis_x;
  int axis_y;
  float thresh_x;
  float thresh_y;
  circle_usb_pref(nDeviceIndex, &usb_pref, &axis_x, &axis_y, &thresh_x,
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
          circle_ui_key_interrupt(KEYCODE_Up, 1);
        } else if (dpad != 0 && old_dpad == 0) {
          circle_ui_key_interrupt(KEYCODE_Up, 0);
        }
        if (dpad == 4 && old_dpad != 4) {
          circle_ui_key_interrupt(KEYCODE_Down, 1);
        } else if (dpad != 4 && old_dpad == 4) {
          circle_ui_key_interrupt(KEYCODE_Down, 0);
        }
        if (dpad == 6 && old_dpad != 6) {
          circle_ui_key_interrupt(KEYCODE_Left, 1);
        } else if (dpad != 6 && old_dpad == 6) {
          circle_ui_key_interrupt(KEYCODE_Left, 0);
        }
        if (dpad == 2 && old_dpad != 2) {
          circle_ui_key_interrupt(KEYCODE_Right, 1);
        } else if (dpad != 2 && old_dpad == 2) {
          circle_ui_key_interrupt(KEYCODE_Right, 0);
        }
        handle_button_function(true, nDeviceIndex, b);
        return;
      }

      handle_button_function(false, nDeviceIndex, b);

      int value = 0;
      if (dpad < 8)
        value |= dpad_to_joy[dpad];
      value |= circle_add_button_values(nDeviceIndex, b);
      circle_joy_usb(nDeviceIndex, value);
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
          circle_ui_key_interrupt(KEYCODE_Up, 1);
        } else if (!a_up && prev_a_up) {
          circle_ui_key_interrupt(KEYCODE_Up, 0);
        }
        if (a_down && !prev_a_down) {
          circle_ui_key_interrupt(KEYCODE_Down, 1);
        } else if (!a_down && prev_a_down) {
          circle_ui_key_interrupt(KEYCODE_Down, 0);
        }
        if (a_left && !prev_a_left) {
          circle_ui_key_interrupt(KEYCODE_Left, 1);
        } else if (!a_left && prev_a_left) {
          circle_ui_key_interrupt(KEYCODE_Left, 0);
        }
        if (a_right && !prev_a_right) {
          circle_ui_key_interrupt(KEYCODE_Right, 1);
        } else if (!a_right && prev_a_right) {
          circle_ui_key_interrupt(KEYCODE_Right, 0);
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
      value |= circle_add_button_values(nDeviceIndex, b);
      circle_joy_usb(nDeviceIndex, value);
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

  circle_set_demo_mode(mViceOptions.GetDemoMode());

  unsigned num_pads = 0;
  int num_buttons[2] = {0, 0};
  int num_axes[2] = {0, 0};
  int num_hats[2] = {0, 0};
  while (num_pads < 2) {
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

  // Tell vice what we found
  joy_set_gamepad_info(num_pads, num_buttons, num_axes, num_hats);

  // This core will do nothing but service interrupts from
  // usb or gpio.
  printf("Core 0 idle\n");

  asm("dsb\n\t"
      "1: wfi\n\t"
      "b 1b\n\t");

  return ShutdownHalt;
}

void CKernel::ScanKeyboard() {
  int ui_activated = circle_ui_activated();

  // For restore, there is no public API that triggers it so we will
  // pass the keycode that will.
  int restore = gpioPins[GPIO_KBD_RESTORE_INDEX]->Read();
  if (restore == LOW && kbdRestoreState == HIGH) {
     circle_key_pressed(KEYCODE_PageUp);
  } else if (restore == HIGH && kbdRestoreState == LOW) {
     circle_key_released(KEYCODE_PageUp);
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
             circle_key_pressed(KEYCODE_Left);
          } else if (keycode == KEYCODE_Down && (uiLeftShift || uiRightShift)) {
             circle_key_pressed(KEYCODE_Up);
          } else {
             circle_key_pressed(keycode);
          }
        } else if (val == HIGH && kbdMatrixStates[kbdPA][kbdPB] == LOW) {
          if (keycode == KEYCODE_LeftShift) {
             uiLeftShift = false;
          } else if (keycode == KEYCODE_RightShift) {
             uiRightShift = false;
          }
          if (keycode == KEYCODE_Right && (uiLeftShift || uiRightShift)) {
             circle_key_released(KEYCODE_Left);
          } else if (keycode == KEYCODE_Down && (uiLeftShift || uiRightShift)) {
             circle_key_released(KEYCODE_Up);
          } else {
             circle_key_released(keycode);
          }
        }
      } else {
        // TODO: Need to watch out for key combos here.  Hook into
        // the handle functions directly in kbd.c so we can invoke the
        // same hotkey funcs.
        if (val == LOW && kbdMatrixStates[kbdPA][kbdPB] == HIGH) {
          circle_key_pressed(keycode);
        } else if (val == HIGH && kbdMatrixStates[kbdPA][kbdPB] == LOW) {
          circle_key_released(keycode);
        }
      }
      kbdMatrixStates[kbdPA][kbdPB] = val;
    }
    gpioPins[kbdPA]->SetMode(GPIOModeInputPullUp);
  }
}

// Read joystick state. 
// If usePcb is true, PCB layout for joysticks is used (where
// selector is used to drive pins low instead of GND).  Otherwise,
// the non-pcb way of reading joysticks is used.
void CKernel::ReadJoystick(int device, bool usePcb) {
  static int js_prev_0[5] = {HIGH, HIGH, HIGH, HIGH, HIGH};
  static int js_prev_1[5] = {HIGH, HIGH, HIGH, HIGH, HIGH};

  int *js_prev;
  CGPIOPin **js_pins;
  CGPIOPin *js_selector = NULL;
  int port = 0;
  int ui_activated = circle_ui_activated();

  // If ui is activated, don't bail if port assignment can't be done
  // since the event will always go to the ui. We want the joystick to
  // function in the ui even if the control port is not assigned to be
  // gpio.
  if (device == 0) {
    js_prev = js_prev_0;
    if (usePcb) {
       js_selector = gpioPins[GPIO_JS1_SELECT_INDEX];
       js_pins = joystickPins1;
    } else {
       js_pins = noPCBJoystickPins1;
    }

    if (joydevs[0].device == JOYDEV_GPIO_0) {
      port = joydevs[0].port;
    } else if (joydevs[1].device == JOYDEV_GPIO_0) {
      port = joydevs[1].port;
    } else if (!ui_activated) {
      return;
    }
  } else {
    js_prev = js_prev_1;
    if (usePcb) {
       js_selector = gpioPins[GPIO_JS2_SELECT_INDEX];
       js_pins = joystickPins2;
    } else {
       js_pins = noPCBJoystickPins2;
    }

    if (joydevs[0].device == JOYDEV_GPIO_1) {
      port = joydevs[0].port;
    } else if (joydevs[1].device == JOYDEV_GPIO_1) {
      port = joydevs[1].port;
    } else if (!ui_activated) {
      return;
    }
  }

  if (usePcb) {
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

  if (ui_activated) {
    if (js_up == LOW && js_prev[JOY_UP] != LOW) {
      circle_ui_key_interrupt(KEYCODE_Up, 1);
    } else if (js_up != LOW && js_prev[JOY_UP] == LOW) {
      circle_ui_key_interrupt(KEYCODE_Up, 0);
    }

    if (js_down == LOW && js_prev[JOY_DOWN] != LOW) {
      circle_ui_key_interrupt(KEYCODE_Down, 1);
    } else if (js_down != LOW && js_prev[JOY_DOWN] == LOW) {
      circle_ui_key_interrupt(KEYCODE_Down, 0);
    }

    if (js_left == LOW && js_prev[JOY_LEFT] != LOW) {
      circle_ui_key_interrupt(KEYCODE_Left, 1);
    } else if (js_left != LOW && js_prev[JOY_LEFT] == LOW) {
      circle_ui_key_interrupt(KEYCODE_Left, 0);
    }

    if (js_right == LOW && js_prev[JOY_RIGHT] != LOW) {
      circle_ui_key_interrupt(KEYCODE_Right, 1);
    } else if (js_right != LOW && js_prev[JOY_RIGHT] == LOW) {
      circle_ui_key_interrupt(KEYCODE_Right, 0);
    }

    if (js_fire == LOW && js_prev[JOY_FIRE] != LOW) {
      circle_ui_key_interrupt(KEYCODE_Return, 1);
    } else if (js_fire != LOW && js_prev[JOY_FIRE] == LOW) {
      circle_ui_key_interrupt(KEYCODE_Return, 0);
    }
    js_prev[JOY_UP] = js_up;
    js_prev[JOY_DOWN] = js_down;
    js_prev[JOY_LEFT] = js_left;
    js_prev[JOY_RIGHT] = js_right;
    js_prev[JOY_FIRE] = js_fire;
  } else {
    int val = 0;
    if (js_up == LOW) val |= 0x01;
    if (js_down == LOW) val |= 0x02;
    if (js_left == LOW) val |= 0x04;
    if (js_right == LOW) val |= 0x08;
    if (js_fire == LOW) val |= 0x10;
    circle_emu_joy_interrupt(PENDING_EMU_JOY_TYPE_ABSOLUTE, port, val);
  }

  if (usePcb) {
     js_selector->SetMode(GPIOModeInputPullUp);
  }
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
    mViceSound->Playback();
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
  return 0;
}

void CKernel::circle_yield(void) { CScheduler::Get()->Yield(); }

void CKernel::MouseStatusHandler(unsigned nButtons, int deltaX, int deltaY) {
  static unsigned int prev_buttons = {0};

  circle_mouse_move(deltaX, deltaY);

  if ((prev_buttons & MOUSE_BUTTON_LEFT) && !(nButtons & MOUSE_BUTTON_LEFT)) {
    circle_button_left(0);
  } else if (!(prev_buttons & MOUSE_BUTTON_LEFT) &&
             (nButtons & MOUSE_BUTTON_LEFT)) {
    circle_button_left(1);
  }
  if ((prev_buttons & MOUSE_BUTTON_RIGHT) && !(nButtons & MOUSE_BUTTON_RIGHT)) {
    circle_button_right(0);
  } else if (!(prev_buttons & MOUSE_BUTTON_RIGHT) &&
             (nButtons & MOUSE_BUTTON_RIGHT)) {
    circle_button_right(1);
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
        circle_key_pressed(KEYCODE_LeftControl);
        break;
      case 1: // LeftShift
        if (circle_ui_activated()) {
          uiLeftShift = true;
        }
        circle_key_pressed(KEYCODE_LeftShift);
        break;
      case 5: // RightShift
        if (circle_ui_activated()) {
          uiRightShift = true;
        }
        circle_key_pressed(KEYCODE_RightShift);
        break;
      case 3: // LeftAlt
        circle_key_pressed(KEYCODE_LeftAlt);
        break;
      default:
        break;
      }
    } else if (!(ucModifiers & v) && (mod_states & v)) {
      switch (i) {
      case 0: // LeftControl
        circle_key_released(KEYCODE_LeftControl);
        break;
      case 1: // LeftShift
        if (circle_ui_activated()) {
          uiLeftShift = false;
        }
        circle_key_released(KEYCODE_LeftShift);
        break;
      case 5: // RightShift
        if (circle_ui_activated()) {
          uiRightShift = false;
        }
        circle_key_released(KEYCODE_RightShift);
        break;
      case 3: // LeftAlt
        circle_key_released(KEYCODE_LeftAlt);
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
  int ui_activated = circle_ui_activated();
  for (unsigned i = 1; i < MAX_KEY_CODES; i++) {
    if (key_states[i] == true && new_states[i] == false) {
      if (ui_activated) {
        // We have to handle shift+left/right here or else our ui
        // isn't navigable by keyrah with real C64 board. Keep
        // key_states below managing the state of the original key,
        // not the translated one.
        if ((uiLeftShift || uiRightShift) && i == KEYCODE_Right) {
          circle_key_released(KEYCODE_Left);
        } else if ((uiLeftShift || uiRightShift) && i == KEYCODE_Down) {
          circle_key_released(KEYCODE_Up);
        } else {
          circle_key_released(i);
        }
      } else {
        circle_key_released(i);
      }
    } else if (key_states[i] == false && new_states[i] == true) {
      if (ui_activated) {
        // See above note on shift.
        if ((uiLeftShift || uiRightShift) && i == KEYCODE_Right) {
          circle_key_pressed(KEYCODE_Left);
        } else if ((uiLeftShift || uiRightShift) && i == KEYCODE_Down) {
          circle_key_pressed(KEYCODE_Up);
        } else {
          circle_key_pressed(i);
        }
      } else {
        circle_key_pressed(i);
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
  if (circle_use_pcb()) {
     ScanKeyboard();
     ReadJoystick(0, TRUE);
     ReadJoystick(1, TRUE);
  } else {
    if (ReadDebounced(GPIO_MENU_INDEX) == BTN_PRESS) {
      circle_key_pressed(KEYCODE_F12);
      circle_key_released(KEYCODE_F12);
    }
    ReadJoystick(0, FALSE);
    ReadJoystick(1, FALSE);
  }
}

void CKernel::circle_lock_acquire() { m_Lock.Acquire(); }

void CKernel::circle_lock_release() { m_Lock.Release(); }

void CKernel::circle_boot_complete() { DisableBootStat(); }

#if defined(RASPI_VIC20)
int CKernel::circle_cycles_per_second() {
  if (circle_get_machine_timing() == MACHINE_TIMING_NTSC_HDMI) {
    // 60hz
    return 1017900;
  } else if (circle_get_machine_timing() == MACHINE_TIMING_NTSC_COMPOSITE) {
    // Actual C64's NTSC Composite frequency is 59.826 but the Pi's vertical
    // sync frequency on composite is 60.053. See c64.h for how this is
    // calculated. This keeps audio buffer to a minimum using ReSid.
    return 1018804;
  } else if (circle_get_machine_timing() == MACHINE_TIMING_NTSC_CUSTOM) {
    return mViceOptions.GetCyclesPerSecond();
  } else if (circle_get_machine_timing() == MACHINE_TIMING_PAL_HDMI) {
    // 50hz
    return 1107600;
  } else if (circle_get_machine_timing() == MACHINE_TIMING_PAL_COMPOSITE) {
    // Actual C64's PAL Composite frequency is 50.125 but the Pi's vertical
    // sync frequency on composite is 50.0816. See c64.h for how this is
    // calculated.  This keep audio buffer to a minimum using ReSid.
    return 1109372;
  } else if (circle_get_machine_timing() == MACHINE_TIMING_PAL_CUSTOM) {
    return mViceOptions.GetCyclesPerSecond();
  } else {
    return 1017900;
  }
}
#else
int CKernel::circle_cycles_per_second() {
  if (circle_get_machine_timing() == MACHINE_TIMING_NTSC_HDMI) {
    // 60hz
    return 1025700;
  } else if (circle_get_machine_timing() == MACHINE_TIMING_NTSC_COMPOSITE) {
    // Actual C64's NTSC Composite frequency is 59.826 but the Pi's vertical
    // sync frequency on composite is 60.053. See c64.h for how this is
    // calculated. This keeps audio buffer to a minimum using ReSid.
    return 1026611;
  } else if (circle_get_machine_timing() == MACHINE_TIMING_NTSC_CUSTOM) {
    return mViceOptions.GetCyclesPerSecond();
  } else if (circle_get_machine_timing() == MACHINE_TIMING_PAL_HDMI) {
    // 50hz
    return 982800;
  } else if (circle_get_machine_timing() == MACHINE_TIMING_PAL_COMPOSITE) {
    // Actual C64's PAL Composite frequency is 50.125 but the Pi's vertical
    // sync frequency on composite is 50.0816. See c64.h for how this is
    // calculated.  This keep audio buffer to a minimum using ReSid.
    return 984404;
  } else if (circle_get_machine_timing() == MACHINE_TIMING_PAL_CUSTOM) {
    return mViceOptions.GetCyclesPerSecond();
  } else {
    return 982800;
  }
}
#endif

int CKernel::circle_alloc_fb2(int layer, uint8_t **pixels,
                              int width, int height, int *pitch) {
  return fb2[layer].Allocate(pixels, width, height, pitch);  
}

void CKernel::circle_free_fb2(int layer) {
  fb2[layer].Free();
}

void CKernel::circle_clear_fb2(int layer) {
  fb2[layer].Clear();
}

void CKernel::circle_show_fb2(int layer) {
  fb2[layer].Show();
}

void CKernel::circle_hide_fb2(int layer) {
  fb2[layer].Hide();
}

void CKernel::circle_frames_ready_fb2(int layer1, int layer2, int sync) {
  // If we're going to sync to vblank, indicate this frame data should go
  // to the offscreen resource.
  fb2[layer1].FrameReady(sync);
  if (layer2 >= 0) {
     fb2[layer2].FrameReady(sync);
  }
  if (sync) {
     // Flip the buffers and wait for vblank.
     FrameBuffer2::SwapResources(&fb2[layer1], layer2 >= 0 ? &fb2[layer2] : nullptr);
  }
}

void CKernel::circle_set_palette_fb2(int layer, uint8_t index, uint16_t rgb565) {
  fb2[layer].SetPalette(index, rgb565);
}

void CKernel::circle_update_palette_fb2(int layer) {
  fb2[layer].UpdatePalette();
}

void CKernel::circle_set_aspect_fb2(int layer, double aspect) {
  fb2[layer].SetAspect(aspect);
}

void CKernel::circle_set_src_rect_fb2(int layer, int x, int y, int w, int h) {
  fb2[layer].SetSrcRect(x,y,w,h);
}

void CKernel::circle_set_valign_fb2(int layer, int align, int padding) {
  fb2[layer].SetVerticalAlignment(align, padding);
}

void CKernel::circle_set_halign_fb2(int layer, int align, int padding) {
  fb2[layer].SetHorizontalAlignment(align, padding);
}

void CKernel::circle_set_padding_fb2(int layer, double lpad, double rpad, double tpad, double bpad) {
  fb2[layer].SetPadding(lpad, rpad, tpad, bpad);
}

void CKernel::circle_set_zlayer_fb2(int layer, int zlayer) {
  fb2[layer].SetLayer(zlayer);
}

int CKernel::circle_get_zlayer_fb2(int layer) {
  return fb2[layer].GetLayer();
}
