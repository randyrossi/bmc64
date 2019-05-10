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

#include <stdio.h>
#include <string.h>
#include <math.h>
#include <stdlib.h>
#include <errno.h>

#include <circle/gpiopin.h>

CKernel* static_kernel = NULL;

#define MAX_KEY_CODES 128

// Used as indices into the hardwires joystick arrays
#define JOY_UP 0
#define JOY_DOWN 1
#define JOY_LEFT 2
#define JOY_RIGHT 3
#define JOY_FIRE 4

// Usb key states
static bool key_states[MAX_KEY_CODES];
static unsigned char mod_states;

// Real C64 keyboard matrix states
static bool kbdMatrixStates[8][8];

extern "C" {
  int circle_get_machine_timing() {
     return static_kernel->circle_get_machine_timing();
  }

  // circle hooks, these will disappear when we move away from circle
  uint8_t* circle_get_fb() {
     // Frame buffer guaranteed to be ready before vice can call this
     // so this is okay.
     return static_kernel->circle_get_fb();
  }

  int circle_get_fb_pitch() {
     // Frame buffer guaranteed to be ready before vice can call this.
     return static_kernel->circle_get_fb_pitch();
  }

  void circle_sleep(long delay) {
     // Timer guaranteed to be ready before vice can call this.
     return static_kernel->circle_sleep(delay);
  }

  void circle_set_palette(uint8_t index, uint16_t rgb565) {
     // Screen guaranteed to be ready before vice can call this.
     return static_kernel->circle_set_palette(index, rgb565);
  }

  void circle_update_palette() {
     // Screen guaranteed to be ready before vice can call this.
     return static_kernel->circle_update_palette();
  }

  int circle_get_display_w() {
     // Screen guaranteed to be ready before vice can call this.
     return static_kernel->circle_get_display_w();
  }

  int circle_get_display_h() {
     // Screen guaranteed to be ready before vice can call this.
     return static_kernel->circle_get_display_h();
  }

  unsigned long circle_get_ticks() {
     // Timer guaranteed to be ready before vice can call this.
     return static_kernel->circle_get_ticks();
  }

  void circle_set_fb_y(int loc) {
     // Screen guaranteed to be ready before vice can call this.
     static_kernel->circle_set_fb_y(loc);
  }

  void circle_wait_vsync() {
     // Screen guaranteed to be ready before vice can call this.
     static_kernel->circle_wait_vsync();
  }

  int circle_sound_bufferspace() {
     // Sound init will happen before this so this is okay
     return static_kernel->circle_sound_bufferspace();
  }

  int circle_sound_init(const char *param, int *speed,
                        int *fragsize, int *fragnr, int *channels) {
     // VCHIQ is guaranteed to have been constructed but not necessarily
     // initialized so we defer its initialization until this method is
     // called by vice.
     return static_kernel->circle_sound_init(param, speed,
                                             fragsize, fragnr, channels);
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
};

bool CKernel::uiShift = false;

CKernel::CKernel (void) : ViceStdioApp("vice"),
                          mVCHIQ (&mMemory, &mInterrupt),
                          mViceSound(nullptr)
{
  static_kernel = this;
  mod_states = 0;
  memset(key_states, 0, MAX_KEY_CODES * sizeof(bool));

  // Only used for pins that are used as buttons. See viceapp.h.
  for (int i =0 ;i < NUM_GPIO_PINS; i++) {
     gpio_debounce_state[i] = BTN_UP;
  }

  for (int i=0;i<8;i++) {
    for (int j=0;j<8;j++) {
       kbdMatrixStates[i][j] = HIGH;
    }
  }
}

bool CKernel::Initialize(void) {
   if (!ViceStdioApp::Initialize()) {
     return false;
   }

   return true;
}

// KEEP THIS IN SYNC WITH kbd.c
static void handle_button_function(bool is_ui, int button_func) {
   // KEEP THIS IN SYNC WITH kbd.c
   switch (button_func) {
      case BTN_ASSIGN_MENU:
         circle_key_pressed(KEYCODE_F12);
         circle_key_released(KEYCODE_F12);
         break;
      case BTN_ASSIGN_WARP:
      case BTN_ASSIGN_SWAP_PORTS:
      case BTN_ASSIGN_STATUS_TOGGLE:
      case BTN_ASSIGN_TAPE_MENU:
      case BTN_ASSIGN_CART_MENU:
      case BTN_ASSIGN_CART_FREEZE:
      case BTN_ASSIGN_RESET_HARD:
      case BTN_ASSIGN_RESET_SOFT:
         circle_emu_quick_func_interrupt(button_func);
         break;
      case BTN_ASSIGN_FIRE:
         // Only need to handle ui fire here.
         if (is_ui) {
            circle_ui_key_interrupt(KEYCODE_Return, 1);
            circle_ui_key_interrupt(KEYCODE_Return, 0);
         }
         break;
      // Don't try to handle dirs here even for ui. We want
      // repeat to work for those and button func is only
      // discovered on the down press anyway.
      default:
         break;
   }
}

// Interrupt handler. Make this quick.
void CKernel::GamePadStatusHandler (unsigned nDeviceIndex,
                                    const TGamePadState *pState) {

   static int dpad_to_joy[8] = {0x01, 0x09, 0x08, 0x0a, 0x02, 0x06, 0x04, 0x05};

   static unsigned int prev_buttons[2] = {0, 0};
   static int prev_dpad[2] = {8, 8};
   static int prev_axes_dirs[2][12] = {
      {0, 0, 0, 0 },
      {0, 0, 0, 0 }
   };

   if (nDeviceIndex >= 2) return;

   if (menu_wants_raw_usb()) {
      // Send the raw usb data and we're done.
      int axes[16];
      for (int i=0;i<pState->naxes;i++) { axes[i] = pState->axes[i].value; }
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
   circle_usb_pref(nDeviceIndex, &usb_pref, &axis_x, &axis_y, &thresh_x, &thresh_y);

   int max_index = axis_x;
   if (axis_y > max_index) max_index = axis_y;

   if (usb_pref == USB_PREF_HAT && pState->nhats > 0) {
	   int dpad = pState->hats[0];
	   bool has_changed =
			(prev_buttons[nDeviceIndex] != b) ||
			(prev_dpad[nDeviceIndex] != dpad);
	   if (has_changed) {
              int old_dpad = prev_dpad[nDeviceIndex];
              prev_buttons[nDeviceIndex] = b;
              prev_dpad[nDeviceIndex] = dpad;

              // If the UI is activated, route to the menu.
              int button_func = circle_button_function(nDeviceIndex, b);
              if (ui_activated) {
                 if (dpad == 0 && old_dpad != 0) {
                    circle_ui_key_interrupt(KEYCODE_Up, 1);
                 }
                 else if (dpad != 0 && old_dpad == 0) {
                    circle_ui_key_interrupt(KEYCODE_Up, 0);
                 }
                 if (dpad == 4 && old_dpad != 4) {
                    circle_ui_key_interrupt(KEYCODE_Down, 1);
                 }
                 else if (dpad != 4 && old_dpad == 4) {
                    circle_ui_key_interrupt(KEYCODE_Down, 0);
                 }
                 if (dpad == 6 && old_dpad != 6) {
                    circle_ui_key_interrupt(KEYCODE_Left, 1);
                 }
                 else if (dpad != 6 && old_dpad == 6) {
                    circle_ui_key_interrupt(KEYCODE_Left, 0);
                 }
                 if (dpad == 2 && old_dpad != 2) {
                    circle_ui_key_interrupt(KEYCODE_Right, 1);
                 }
                 else if (dpad != 2 && old_dpad == 2) {
                    circle_ui_key_interrupt(KEYCODE_Right, 0);
                 }
                 handle_button_function(true, button_func);
                 return;
              }

              handle_button_function(false, button_func);

              int value = 0;
              if (dpad < 8) value |= dpad_to_joy[dpad];
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
	   bool has_changed =
		   (prev_buttons[nDeviceIndex] != b) ||
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
              int button_func = circle_button_function(nDeviceIndex, b);
              if (ui_activated) {
                 if (a_up && !prev_a_up) {
                    circle_ui_key_interrupt(KEYCODE_Up, 1);
                 }
                 else if (!a_up && prev_a_up) {
                    circle_ui_key_interrupt(KEYCODE_Up, 0);
                 }
                 if (a_down && !prev_a_down) {
                    circle_ui_key_interrupt(KEYCODE_Down, 1);
                 }
                 else if (!a_down && prev_a_down) {
                    circle_ui_key_interrupt(KEYCODE_Down, 0);
                 }
                 if (a_left && !prev_a_left) {
                    circle_ui_key_interrupt(KEYCODE_Left, 1);
                 }
                 else if (!a_left && prev_a_left) {
                    circle_ui_key_interrupt(KEYCODE_Left, 0);
                 }
                 if (a_right && !prev_a_right) {
                    circle_ui_key_interrupt(KEYCODE_Right, 1);
                 }
                 else if (!a_right && prev_a_right) {
                    circle_ui_key_interrupt(KEYCODE_Right, 0);
                 }
                 handle_button_function(true, button_func);
                 return;
              }

              handle_button_function(false, button_func);

              int value = 0;
              if (a_left) value |= 0x4;
              if (a_right) value |= 0x8;
              if (a_up) value |= 0x1;
              if (a_down) value |= 0x2;
              value |= circle_add_button_values(nDeviceIndex, b);
              circle_joy_usb(nDeviceIndex, value);
	   }
   }
}

void CKernel::SetupUSBKeyboard() {
   CUSBKeyboardDevice *pKeyboard = (CUSBKeyboardDevice *)
     mDeviceNameService.GetDevice ("ukbd1", FALSE);
   if (pKeyboard) {
      pKeyboard->RegisterKeyStatusHandlerRaw (KeyStatusHandlerRaw);
   }
}

void CKernel::SetupUSBMouse() {
   CMouseDevice *pMouse = (CMouseDevice *)
     mDeviceNameService.GetDevice ("mouse1", FALSE);
   if (pMouse) {
      pMouse->RegisterStatusHandler (MouseStatusHandler);
   }
}

ViceApp::TShutdownMode CKernel::Run (void)
{
  SetupUSBKeyboard();
  SetupUSBMouse();

  circle_set_demo_mode(mViceOptions.GetDemoMode());

  unsigned num_pads = 0;
  int num_buttons[2] = {0,0};
  int num_axes[2] = {0,0};
  int num_hats[2] = {0,0};
  while (num_pads < 2) {
    CString DeviceName;
    DeviceName.Format("upad%u", num_pads + 1);

    CUSBGamePadDevice* game_pad =
      (CUSBGamePadDevice *) mDeviceNameService.GetDevice (DeviceName, FALSE);

    if (game_pad == 0) { break; }

    const TGamePadState *pState = game_pad->GetInitialState ();
    assert (pState != 0);

    num_axes[num_pads]= pState->naxes;
    num_hats[num_pads]= pState->nhats;
    num_buttons[num_pads]= pState->nbuttons;

    game_pad->RegisterStatusHandler (GamePadStatusHandler);
    num_pads++;
  }

  // Tell vice what we found
  joy_set_gamepad_info(num_pads, num_buttons, num_axes, num_hats);

  // This core will do nothing but service interrupts from
  // usb or gpio.
  printf ("Core 0 idle\n");

  asm("dsb\n\t"
      "1: wfi\n\t"
      "b 1b\n\t"
  );

  return ShutdownHalt;
}

// For a real C64 keyboard + joystick ports
void CKernel::ScanKeyboardAndJoysticks() {

  // When we're using a real C64 keyboard, ports cannot be assigned.
  // The events must go to the 'native' port for that bank.
  ReadJoysticks(0, false);
  ReadJoysticks(1, false);

  // Keyboard scan
  for (int kbdPA=0;kbdPA<8;kbdPA++) {
    gpioPins[kbdPA]->SetMode(GPIOModeOutput);
    gpioPins[kbdPA]->Write(LOW);
    for (int kbdPB=0;kbdPB<8;kbdPB++) {
      // Read PBi line
      int val = gpioPins[kbdPB+8]->Read();
      if (val == LOW && kbdMatrixStates[kbdPB][kbdPA] == HIGH) {
         circle_keyboard_set_latch_keyarr(kbdPA, kbdPB, 1);
      } else if (val == HIGH && kbdMatrixStates[kbdPB][kbdPA] == LOW) {
         circle_keyboard_set_latch_keyarr(kbdPA, kbdPB, 0);
      }
      kbdMatrixStates[kbdPB][kbdPA] = val;
    }
    gpioPins[kbdPA]->SetMode(GPIOModeInputPullUp);
  }
}

ssize_t CKernel::vice_write (int fd, const void * buf, size_t count) {
  return mSerial.Write(buf, count);
}

int CKernel::circle_get_machine_timing () {
   // See circle.h for valid values
   return mViceOptions.GetMachineTiming();
}

uint8_t* CKernel::circle_get_fb () {
  return (uint8_t*)mScreen.GetBuffer();
}

int CKernel::circle_get_fb_pitch () {
   return mScreen.GetPitch();
}

void CKernel::circle_sleep(long delay) {
   mTimer.SimpleusDelay(delay);
}

void CKernel::circle_set_palette(uint8_t index, uint16_t rgb565) {
   mScreen.SetPalette(index, rgb565);
}

void CKernel::circle_update_palette() {
   mScreen.UpdatePalette();
}

int CKernel::circle_get_display_w() {
   return mScreen.GetWidth();
}

int CKernel::circle_get_display_h() {
   return mScreen.GetHeight();
}

unsigned long CKernel::circle_get_ticks() {
   return mTimer.GetClockTicks();
}

void CKernel::circle_set_fb_y(int loc) {
  mScreen.SetVirtualOffset(0, loc);
}

void CKernel::circle_wait_vsync() {
  mScreen.WaitForVerticalSync();
}

// Called from VICE: Core 1
int CKernel::circle_sound_init(const char *param, int *speed,
                               int *fragsize, int *fragnr, int *channels) {
  *speed = SAMPLE_RATE;
  *fragsize = FRAG_SIZE;
  *fragnr = NUM_FRAGS;
  // We force mono.
  *channels = 1;

  if (!mViceSound) {
     // After parallelizing circle + vice init routine, we have to init vchiq
     // here on core 1 instead of the circle init on core 0.  Not sure why it
     // fails but the device is not found otherwise.
     if (!mVCHIQ.Initialize()) {
       printf ("ERROR: Could not init VCHIQ\n");
       return 0;
     }

     mViceSound = new ViceSound(&mVCHIQ,  mViceOptions.GetAudioOut());
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

int CKernel::circle_sound_suspend(void) {
  return 0;
}

int CKernel::circle_sound_resume(void) {
  return 0;
}

int CKernel::circle_sound_bufferspace(void) {
  if (mViceSound) {
     return mViceSound->BufferSpaceBytes();
  }
  return 0;
}

void CKernel::circle_yield(void) {
  CScheduler::Get()->Yield();
}

void CKernel::MouseStatusHandler (unsigned nButtons, int deltaX, int deltaY)
{
   static unsigned int prev_buttons = {0};

   circle_mouse_move(deltaX, deltaY);

   if ((prev_buttons & MOUSE_BUTTON_LEFT) &&
          !(nButtons & MOUSE_BUTTON_LEFT)) {
      circle_button_left(0);
   } else if (!(prev_buttons & MOUSE_BUTTON_LEFT) &&
          (nButtons & MOUSE_BUTTON_LEFT)) {
      circle_button_left(1);
   }
   if ((prev_buttons & MOUSE_BUTTON_RIGHT) &&
          !(nButtons & MOUSE_BUTTON_RIGHT)) {
      circle_button_right(0);
   } else if (!(prev_buttons & MOUSE_BUTTON_RIGHT) &&
          (nButtons & MOUSE_BUTTON_RIGHT)) {
      circle_button_right(1);
   }
   prev_buttons = nButtons;
}

void CKernel::KeyStatusHandlerRaw (unsigned char ucModifiers,
                                   const unsigned char RawKeys[6]) {

   bool new_states[MAX_KEY_CODES];
   memset(new_states, 0, MAX_KEY_CODES * sizeof(bool));

   // Compare previous to present and handle press/release that come from
   // modifier keys.
   int v = 1;
   for (int i=0;i<8;i++) {
      if ((ucModifiers & v) && !(mod_states & v)) {
         switch (i) {
           case 0: // LeftControl
              circle_key_pressed(KEYCODE_LeftControl);
              break;
           case 1: // LeftShift
              if (circle_ui_activated()) {
                 uiShift = true;
              }
              circle_key_pressed(KEYCODE_LeftShift);
              break;
           case 5: // RightShift
              if (circle_ui_activated()) {
                 uiShift = true;
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
                 uiShift = false;
              }
              circle_key_released(KEYCODE_LeftShift);
              break;
           case 5: // RightShift
              if (circle_ui_activated()) {
                 uiShift = false;
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
      v=v*2;
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
              if (uiShift && i == KEYCODE_Right) {
                circle_key_released(KEYCODE_Left);
              } else if (uiShift && i == KEYCODE_Down) {
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
              if (uiShift && i == KEYCODE_Right) {
                circle_key_pressed(KEYCODE_Left);
              } else if (uiShift && i == KEYCODE_Down) {
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

// This debounces the menu pin
int CKernel::GetGpioPinState(int pinIndex) {
   CGPIOPin* pin = gpioPins[pinIndex];
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
// Otherwise, just scans gpio joysticks (which are swappable).
void CKernel::circle_check_gpio()
{
   // Do any pins that have special functions.
   // TODO: Make these assignable like usb buttons and hotkeys.
   if (GetGpioPinState(GPIO_MENU_INDEX) == BTN_PRESS) {
      circle_key_pressed(KEYCODE_F12);
      circle_key_released(KEYCODE_F12);
   }

   // Now do keyboard and/or joysticks.
   if (circle_use_real_keyboard()) {
      ScanKeyboardAndJoysticks();
   } else {
      ReadJoysticks(0, true);
      ReadJoysticks(1, true);
   }
}

// If assignable is true, means we will route the event to whatever
// port the user assigned the bank of pins to mean. Otherwise, the event
// must go to the 'native' port that bank of pins belongs to (when using
// real c64 keyboards).  Note 'device' is overloaded here.  The argument
// passed in is the index of the gpio bank we are about to poll.  While
// joydevs[x].device is the type of device the user assigned to the control
// port. So when gpio joys are assignable, we find which port the event
// should go to based on this info.  If the joydev device type is not
// one of the valid GPIO options (taking assignable into consideration),
// then no events are sent.
void CKernel::ReadJoysticks(int device, bool assignable)
{
  static int js_prev_0[5] = { HIGH,HIGH,HIGH,HIGH,HIGH };
  static int js_prev_1[5] = { HIGH,HIGH,HIGH,HIGH,HIGH };

  int* js_prev;
  CGPIOPin** js_pins;
  int port = joydevs[device].port;
  int ui_activated = circle_ui_activated();

  // If ui is activated, don't bail if port assignment can't be done
  // since the event will always go to the ui. We want the joystick to
  // function in the ui even if the control port is not assigned to be
  // gpio.
  if (device == 0) {
     js_prev = js_prev_0;
     js_pins = joystickPins1;
     if (assignable) {
        if (joydevs[0].device == JOYDEV_GPIO_0) {
           port = joydevs[0].port;
        } else if (joydevs[1].device == JOYDEV_GPIO_0) {
           port = joydevs[1].port;
        } else if (!ui_activated) {
           return;
        }
     } else if (joydevs[0].device != JOYDEV_GPIO_0 && !ui_activated) {
        return;
     }
  } else {
     js_prev = js_prev_1;
     js_pins = joystickPins2;
     if (assignable) {
        if (joydevs[0].device == JOYDEV_GPIO_1) {
           port = joydevs[0].port;
        } else if (joydevs[1].device == JOYDEV_GPIO_1) {
           port = joydevs[1].port;
        } else if (!ui_activated) {
           return;
        }
     } else if (joydevs[1].device != JOYDEV_GPIO_1 && !ui_activated) {
        return;
     }
  }

  int js_up = js_pins[JOY_UP]->Read();
  int js_down = js_pins[JOY_DOWN]->Read();
  int js_left = js_pins[JOY_LEFT]->Read();
  int js_right = js_pins[JOY_RIGHT]->Read();
  int js_fire = js_pins[JOY_FIRE]->Read();

  if (js_up == LOW && js_prev[JOY_UP] != LOW) {
     if (ui_activated)
        circle_ui_key_interrupt(KEYCODE_Up, 1);
     else {
        circle_emu_joy_interrupt(PENDING_EMU_JOY_TYPE_OR,
           port, 0x01);
     }
  }
  else if (js_up != LOW && js_prev[JOY_UP] == LOW) {
     if (ui_activated)
        circle_ui_key_interrupt(KEYCODE_Up, 0);
     else {
        circle_emu_joy_interrupt(PENDING_EMU_JOY_TYPE_AND,
           port, ~0x01);
     }
  }
  if (js_down == LOW && js_prev[JOY_DOWN] != LOW) {
     if (ui_activated)
        circle_ui_key_interrupt(KEYCODE_Down, 1);
     else {
        circle_emu_joy_interrupt(PENDING_EMU_JOY_TYPE_OR,
           port, 0x02);
     }
  }
  else if (js_down != LOW && js_prev[JOY_DOWN] == LOW) {
     if (ui_activated)
        circle_ui_key_interrupt(KEYCODE_Down, 0);
     else {
        circle_emu_joy_interrupt(PENDING_EMU_JOY_TYPE_AND,
           port, ~0x02);
     }
  }
  if (js_left == LOW && js_prev[JOY_LEFT] != LOW) {
     if (ui_activated)
        circle_ui_key_interrupt(KEYCODE_Left, 1);
     else {
        circle_emu_joy_interrupt(PENDING_EMU_JOY_TYPE_OR,
           port, 0x04);
     }
  }
  else if (js_left != LOW && js_prev[JOY_LEFT] == LOW) {
     if (ui_activated)
        circle_ui_key_interrupt(KEYCODE_Left, 0);
     else {
        circle_emu_joy_interrupt(PENDING_EMU_JOY_TYPE_AND,
           port, ~0x04);
     }
  }
  if (js_right == LOW && js_prev[JOY_RIGHT] != LOW) {
     if (ui_activated)
        circle_ui_key_interrupt(KEYCODE_Right, 1);
     else {
        circle_emu_joy_interrupt(PENDING_EMU_JOY_TYPE_OR,
           port, 0x08);
     }
  }
  else if (js_right != LOW && js_prev[JOY_RIGHT] == LOW) {
     if (ui_activated)
        circle_ui_key_interrupt(KEYCODE_Right, 0);
     else {
        circle_emu_joy_interrupt(PENDING_EMU_JOY_TYPE_AND,
           port, ~0x08);
     }
  }
  if (js_fire == LOW && js_prev[JOY_FIRE] != LOW) {
     if (ui_activated)
        circle_ui_key_interrupt(KEYCODE_Return, 1);
     else {
        circle_emu_joy_interrupt(PENDING_EMU_JOY_TYPE_OR,
           port, 0x10);
     }
  }
  else if (js_fire != LOW && js_prev[JOY_FIRE] == LOW) {
     if (ui_activated)
        circle_ui_key_interrupt(KEYCODE_Return, 0);
     else {
        circle_emu_joy_interrupt(PENDING_EMU_JOY_TYPE_AND,
           port, ~0x10);
     }
  }

  js_prev[JOY_UP] = js_up;
  js_prev[JOY_DOWN] = js_down;
  js_prev[JOY_LEFT] = js_left;
  js_prev[JOY_RIGHT] = js_right;
  js_prev[JOY_FIRE] = js_fire;
}

void CKernel::circle_lock_acquire() {
  m_Lock.Acquire();
}

void CKernel::circle_lock_release() {
  m_Lock.Release();
}

void CKernel::circle_boot_complete() {
  DisableBootStat();
}

// 1025700 60hz NTSC hdmi
// 1022730 59.826hz NTSC composite
// 982800 50hz for hdmi
// 985248 50.125hz for composite
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
      return mViceOptions.GetCyclesPerRefresh();
   } else if (circle_get_machine_timing() == MACHINE_TIMING_PAL_HDMI) {
      // 50hz
      return 982800;
   } else if (circle_get_machine_timing() == MACHINE_TIMING_PAL_COMPOSITE) {
      // Actual C64's PAL Composite frequency is 50.125 but the Pi's vertical
      // sync frequency on composite is 50.0816. See c64.h for how this is
      // calculated.  This keep audio buffer to a minimum using ReSid.
      return 984404;
   } else if (circle_get_machine_timing() == MACHINE_TIMING_PAL_CUSTOM) {
      return mViceOptions.GetCyclesPerRefresh();
   } else {
      return 982800;
   }
}
