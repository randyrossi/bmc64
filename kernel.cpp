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

#define GPIO_JOY_1_UP 17
#define GPIO_JOY_1_DOWN 18
#define GPIO_JOY_1_LEFT 27
#define GPIO_JOY_1_RIGHT 22
#define GPIO_JOY_1_FIRE 23

#define GPIO_JOY_2_UP 5
#define GPIO_JOY_2_DOWN 6
#define GPIO_JOY_2_LEFT 12
#define GPIO_JOY_2_RIGHT 13
#define GPIO_JOY_2_FIRE 19

#define GPIO_UI 16

// Used as indices into the hardwires joystick arrays
#define JOY_UP 0
#define JOY_DOWN 1
#define JOY_LEFT 2
#define JOY_RIGHT 3
#define JOY_FIRE 4

static bool key_states[MAX_KEY_CODES];
static unsigned char mod_states;

static raspi_key_handler raspi_press_handler = nullptr;
static raspi_key_handler raspi_release_handler = nullptr;

extern "C" {
  ssize_t circle_serial_write (int fd, const void * buf, size_t count) {
    if (static_kernel == NULL) return 0;
    return static_kernel->vice_write(fd, buf, count);
  }

  int circle_get_machine_timing() {
     return static_kernel->circle_get_machine_timing();
  }

  // circle hooks, these will disappear when we move away from circle
  uint8_t* circle_get_fb() {
     return static_kernel->circle_get_fb();
  }

  int circle_get_fb_pitch() {
     return static_kernel->circle_get_fb_pitch();
  }

  void circle_sleep(long delay) {
     return static_kernel->circle_sleep(delay);
  }

  void circle_set_palette(uint8_t index, uint16_t rgb565) {
     return static_kernel->circle_set_palette(index, rgb565);
  }

  void circle_update_palette() {
     return static_kernel->circle_update_palette();
  }

  int circle_get_display_w() {
     return static_kernel->circle_get_display_w();
  }

  int circle_get_display_h() {
     return static_kernel->circle_get_display_h();
  }

  unsigned long circle_get_ticks() {
     return static_kernel->circle_get_ticks();
  }

  void circle_set_fb_y(int loc) {
     static_kernel->circle_set_fb_y(loc);
  }

  void circle_wait_vsync() {
     static_kernel->circle_wait_vsync();
  }

  int circle_sound_bufferspace() {
     return static_kernel->circle_sound_bufferspace();
  }

  int circle_sound_init(const char *param, int *speed, 
                        int *fragsize, int *fragnr, int *channels) {
     return static_kernel->circle_sound_init(param, speed, 
                                             fragsize, fragnr, channels);
  }

  int circle_sound_write(int16_t *pbuf, size_t nr) {
     return static_kernel->circle_sound_write(pbuf, nr);
  }

  void circle_sound_close(void) {
     static_kernel->circle_sound_close();
  }

  int circle_sound_suspend(void) {
     return static_kernel->circle_sound_suspend();
  }

  int circle_sound_resume(void) {
     return static_kernel->circle_sound_resume();
  }

  void circle_yield(void) {
     static_kernel->circle_yield();
  }

  void circle_kbd_init(raspi_key_handler press_handler,
                             raspi_key_handler release_handler) {
     static_kernel->circle_kbd_init(press_handler, release_handler);
  }

  void circle_joy_init() {
     static_kernel->circle_joy_init();
  }

  void circle_poll_joysticks(int port) {
     static_kernel->circle_poll_joysticks(port);
  }

  void circle_check_gpio() {
     static_kernel->circle_check_gpio();
  }
};

bool CKernel::uiShift = false;

CKernel::CKernel (void) : ViceStdioApp("vice"),
                          mVCHIQ (&mMemory, &mInterrupt),
                          mGPIOManager (&mInterrupt)
{
  static_kernel = this;
  mod_states = 0;
  memset(key_states, 0, MAX_KEY_CODES * sizeof(bool));
}

bool CKernel::Initialize(void) {
   if (!ViceStdioApp::Initialize()) {
     return false;
   }

   if (!mVCHIQ.Initialize()) {
     return false;
   }

   if (!mGPIOManager.Initialize()) {
     return false;
   }

   return true;
}

// Interrupt handler. Make this quick.
void CKernel::GamePadStatusHandler (unsigned nDeviceIndex,
                                    const TGamePadState *pState) {

   static int dpad_to_joy[8] = {0x01, 0x09, 0x08, 0x0a, 0x02, 0x06, 0x04, 0x05};

   static unsigned int prev_buttons[2] = {0, 0};
   static int prev_dpad[2] = {8, 8};
   static int prev_axes[2][4] = { {0, 0, 0, 0 }, {0, 0, 0, 0 } };

   if (nDeviceIndex >= 2) return;

   unsigned b = pState->buttons;

   // usb_pref is the value of the usb pref menu item
   // 0 = analog
   // 1 = dpad (hat)
   int usb_pref;
   int axis_x;
   int axis_y;
   circle_usb_pref(nDeviceIndex, &usb_pref, &axis_x, &axis_y);

   int max_index = axis_x;
   if (axis_y > max_index) max_index = axis_y;

   if (usb_pref == 1 && pState->nhats > 0) {
	   int dpad = pState->hats[0];
	   bool has_changed =
			(prev_buttons[nDeviceIndex] != b) ||
			(prev_dpad[nDeviceIndex] != dpad);
	   if (has_changed) {

              // If the UI is activated, route to the menu.
              if (circle_ui_activated()) {
                 if (dpad == 0) {
                    circle_ui_key_interrupt(KEYCODE_Up);
                 }
                 else if (dpad == 4) {
                    circle_ui_key_interrupt(KEYCODE_Down);
                 }
                 else if (dpad == 6) {
                    circle_ui_key_interrupt(KEYCODE_Left);
                 }
                 else if (dpad == 2) {
                    circle_ui_key_interrupt(KEYCODE_Right);
                 }
                 else if (b) {
                    circle_ui_key_interrupt(KEYCODE_Return);
                 }
                 return;
              }

	      int value = 0;
              if (dpad < 8) value |= dpad_to_joy[dpad];
              if (b) value |= 0x10;

              circle_joy_usb(nDeviceIndex, value);

              prev_buttons[nDeviceIndex] = b;
              prev_dpad[nDeviceIndex] = dpad;
	   }
   } else if (usb_pref == 0 && pState->naxes > max_index) {
	   int x = pState->axes[axis_x].value;
	   int y = pState->axes[axis_y].value;
	   // TODO: Do this just once at init
	   int minx = pState->axes[axis_x].minimum;
	   int maxx = pState->axes[axis_x].maximum;
	   int miny = pState->axes[axis_y].minimum;
	   int maxy = pState->axes[axis_y].maximum;
	   int tx = (maxx - minx) / 4;
	   int mx = (maxx + minx) / 2;
	   int ty = (maxy - miny) / 4;
	   int my = (maxy + miny) / 2;
	   bool has_changed =
		   (prev_buttons[nDeviceIndex] != b) ||
		   (prev_axes[nDeviceIndex][axis_x] != x) ||
		   (prev_axes[nDeviceIndex][axis_y] != y);
	   if (has_changed) {

              // If the UI is activated, route to the menu.
              if (circle_ui_activated()) {
                 if (y < my - ty) {
                    circle_ui_key_interrupt(KEYCODE_Up);
                 }
                 else if (y > my + ty) {
                    circle_ui_key_interrupt(KEYCODE_Down);
                 }
                 else if (x < mx - tx) {
                    circle_ui_key_interrupt(KEYCODE_Left);
                 }
                 else if (x > mx + tx) {
                    circle_ui_key_interrupt(KEYCODE_Right);
                 }
                 else if (b) {
                    circle_ui_key_interrupt(KEYCODE_Return);
                 }
                 return;
              }

              int value = 0;
              if (x < mx - tx) value |= 0x4;
              if (x > mx + tx) value |= 0x8;
              if (y < my - ty) value |= 0x1;
              if (y > my + ty) value |= 0x2;
              if (b) value |= 0x10;

              circle_joy_usb(nDeviceIndex, value);

              prev_axes[nDeviceIndex][axis_x] = x;
              prev_axes[nDeviceIndex][axis_y] = y;
              prev_buttons[nDeviceIndex] = b;
	   }
   }
}

bool CKernel::StartupChecksOk() {
   FILE *fp = fopen("chargen","r");
   if (fp == NULL) return false;
   fclose(fp);

   fp = fopen("basic","r");
   if (fp == NULL) return false;
   fclose(fp);

   fp = fopen("kernal","r");
   if (fp == NULL) return false;
   fclose(fp);

   return true;
}

ViceApp::TShutdownMode CKernel::Run (void)
{
  mLogger.Write ("vice", LogNotice, "VICE");

  printf ("Starting vice\n");

  // Call Vice's main_program

  // Use -soundsync 0 option for 'flexible'
  // sound sync.

  // Use -refresh 1 option to turn off the 'auto'
  // refresh which screws up badly after some time.
  // The vertical blank really messes up vice's
  // algorithm that decides to skip frames. Might
  // want to go back to using the open gl hook.
  // See arch/raspi/videoarch.c

  char timing_option[8];
  if (circle_get_machine_timing() == MACHINE_TIMING_NTSC) {
     strcpy(timing_option, "-ntsc");
  } else {
     strcpy(timing_option, "-pal");
  }

  unsigned num_pads = 0;
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

    game_pad->RegisterStatusHandler (GamePadStatusHandler);
    num_pads++;
  }

  // Tell vice what we found
  joy_set_gamepad_info(num_pads, num_axes, num_hats);

  if (!StartupChecksOk()) {
     mScreen.SetHideConsole(false);
     mLogger.Write ("", LogNotice, "KERNAL, CHARGEN or BASIC MISSING");
     mLogger.Write ("", LogNotice, "Please read documentation.");
     for (;;) {
        circle_sleep(1000000);
     }
  } else {
    int argc = 11;
    char *argv[] = {
      (char*)"vice",
      timing_option,
      (char*)"-sounddev",
      (char*)"raspi",
      (char*)"-soundoutput",
      (char*)"1",
      (char*)"-soundsync",
      (char*)"0",
      (char*)"-refresh",
      (char*)"1",
      // Unless we disable the video cache, vsync is messed up
      (char*)"+VICIIvcache",
    };
    main_program(argc, argv);
  }

  return ShutdownHalt;
}

ssize_t CKernel::vice_write (int fd, const void * buf, size_t count) {
  return mSerial.Write(buf, count);
}

int CKernel::circle_get_machine_timing () {
   // Returns 0 for ntsc, 1 for pal
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

int CKernel::circle_sound_init(const char *param, int *speed, 
                               int *fragsize, int *fragnr, int *channels) {
  *speed = SAMPLE_RATE;
  *fragsize = FRAG_SIZE;
  *fragnr = NUM_FRAGS;
  // We force mono.
  *channels = 1;

  mViceSound = new ViceSound(&mVCHIQ,  VCHIQSoundDestinationAuto);
  mViceSound->Playback();
  return 0;
}

int CKernel::circle_sound_write(int16_t *pbuf, size_t nr) {
  return mViceSound->AddChunk(pbuf, nr);
}

void CKernel::circle_sound_close(void) {
  if (!mViceSound) return;

  mViceSound->CancelPlayback();
  delete mViceSound;
  mViceSound = nullptr;
}

int CKernel::circle_sound_suspend(void) {
  return 0;
}

int CKernel::circle_sound_resume(void) {
  return 0;
}

int CKernel::circle_sound_bufferspace(void) {
  return mViceSound->BufferSpaceBytes();
}

void CKernel::circle_yield(void) {
  CScheduler::Get()->Yield();
}

void CKernel::circle_kbd_init(raspi_key_handler press_handler,
                             raspi_key_handler release_handler) {
  raspi_press_handler = press_handler;
  raspi_release_handler = release_handler;

  pKeyboard =  (CUSBKeyboardDevice *) mDeviceNameService.GetDevice ("ukbd1",
                                                                    FALSE);
  pKeyboard->RegisterKeyStatusHandlerRaw (KeyStatusHandlerRaw);
}

void CKernel::circle_joy_init() {
  uiPin = new CGPIOPin(GPIO_UI, GPIOModeInputPullUp, &mGPIOManager);

  // Configure joystick pins now
  joystickPins1[JOY_UP] = new CGPIOPin(GPIO_JOY_1_UP, GPIOModeInputPullUp, &mGPIOManager);
  joystickPins1[JOY_DOWN] = new CGPIOPin(GPIO_JOY_1_DOWN, GPIOModeInputPullUp, &mGPIOManager);
  joystickPins1[JOY_LEFT] = new CGPIOPin(GPIO_JOY_1_LEFT, GPIOModeInputPullUp, &mGPIOManager);
  joystickPins1[JOY_RIGHT] = new CGPIOPin(GPIO_JOY_1_RIGHT, GPIOModeInputPullUp, &mGPIOManager);
  joystickPins1[JOY_FIRE] = new CGPIOPin(GPIO_JOY_1_FIRE, GPIOModeInputPullUp, &mGPIOManager);

  joystickPins2[JOY_UP] = new CGPIOPin(GPIO_JOY_2_UP, GPIOModeInputPullUp, &mGPIOManager);
  joystickPins2[JOY_DOWN] = new CGPIOPin(GPIO_JOY_2_DOWN, GPIOModeInputPullUp, &mGPIOManager);
  joystickPins2[JOY_LEFT] = new CGPIOPin(GPIO_JOY_2_LEFT, GPIOModeInputPullUp, &mGPIOManager);
  joystickPins2[JOY_RIGHT] = new CGPIOPin(GPIO_JOY_2_RIGHT, GPIOModeInputPullUp, &mGPIOManager);
  joystickPins2[JOY_FIRE] = new CGPIOPin(GPIO_JOY_2_FIRE, GPIOModeInputPullUp, &mGPIOManager);

  for (int i=0;i<5;i++) {
     joystickPins1[i]->ConnectInterrupt (InterruptStub, this);
     joystickPins1[i]->EnableInterrupt (GPIOInterruptOnRisingEdge);
     joystickPins1[i]->EnableInterrupt2 (GPIOInterruptOnFallingEdge);
     joystickPins2[i]->ConnectInterrupt (InterruptStub, this);
     joystickPins2[i]->EnableInterrupt (GPIOInterruptOnRisingEdge);
     joystickPins2[i]->EnableInterrupt2 (GPIOInterruptOnFallingEdge);
  }
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
              raspi_press_handler(KEYCODE_LeftControl);
              break;
           case 1: // LeftShift
              if (circle_ui_activated()) {
                 uiShift = true;
              } else {
                 raspi_press_handler(KEYCODE_LeftShift);
              }
              break;
           case 5: // RightShift
              if (circle_ui_activated()) {
                 uiShift = true;
              } else {
                 raspi_press_handler(KEYCODE_RightShift);
              }
              break;
           case 3: // LeftAlt
              raspi_press_handler(KEYCODE_LeftAlt);
              break;
           default:
              break;
         }
      } else if (!(ucModifiers & v) && (mod_states & v)) {
         switch (i) {
           case 0: // LeftControl
              raspi_release_handler(KEYCODE_LeftControl);
              break;
           case 1: // LeftShift
              if (circle_ui_activated()) {
                 uiShift = false;
              } else {
                 raspi_release_handler(KEYCODE_LeftShift);
              }
              break;
           case 5: // RightShift
              if (circle_ui_activated()) {
                 uiShift = false;
              } else {
                 raspi_release_handler(KEYCODE_RightShift);
              }
              break;
           case 3: // LeftAlt
              raspi_release_handler(KEYCODE_LeftAlt);
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
   for (unsigned i = 1; i < MAX_KEY_CODES; i++) {
      if (key_states[i] == true && new_states[i] == false) {
           if (circle_ui_activated()) {
              // We have to handle shift+left/right here or else our ui
              // isn't navigable by keyrah with real C64 board.
              if (uiShift && i == KEYCODE_Right) i = KEYCODE_Left;
              else if (uiShift && i == KEYCODE_Down) i = KEYCODE_Up;
           }
           raspi_release_handler(i);
      } else if (key_states[i] == false && new_states[i] == true) {
           raspi_press_handler(i);
      }
      key_states[i] = new_states[i];    
   }
}

// This checks whether GPIO16 has triggered the ui activation
// Not hooked to interrupt, only polled.
void CKernel::circle_check_gpio()
{
   if (uiPin->Read() == LOW) {
      circle_ui_key_interrupt(KEYCODE_F12);
   }
}

void CKernel::circle_poll_joysticks(int device)
{
  if (device == 0) {

     // If the UI is activated, route to the menu.
     if (circle_ui_activated()) {
        if (joystickPins1[JOY_UP]->Read() == LOW) {
           circle_ui_key_interrupt(KEYCODE_Up);
        }
        else if (joystickPins1[JOY_DOWN]->Read() == LOW) {
           circle_ui_key_interrupt(KEYCODE_Down);
        }
        else if (joystickPins1[JOY_LEFT]->Read() == LOW) {
           circle_ui_key_interrupt(KEYCODE_Left);
        }
        else if (joystickPins1[JOY_RIGHT]->Read() == LOW) {
           circle_ui_key_interrupt(KEYCODE_Right);
        }
        else if (joystickPins1[JOY_FIRE]->Read() == LOW) {
           circle_ui_key_interrupt(KEYCODE_Return);
        }
        return;
     }

     int value = 0;
     if (joystickPins1[JOY_UP]->Read() == LOW) {
        value |= 0x1;
     }
     if (joystickPins1[JOY_DOWN]->Read() == LOW) {
        value |= 0x2;
     }
     if (joystickPins1[JOY_LEFT]->Read() == LOW) {
        value |= 0x4;
     }
     if (joystickPins1[JOY_RIGHT]->Read() == LOW) {
        value |= 0x8;
     }
     if (joystickPins1[JOY_FIRE]->Read() == LOW) {
        value |= 0x10;
     }
     circle_joy_gpio(0, value);
  } else if (device == 1) {
     // If the UI is activated, route to the menu.
     if (circle_ui_activated()) {
        if (joystickPins2[JOY_UP]->Read() == LOW) {
           circle_ui_key_interrupt(KEYCODE_Up);
        }
        else if (joystickPins2[JOY_DOWN]->Read() == LOW) {
           circle_ui_key_interrupt(KEYCODE_Down);
        }
        else if (joystickPins2[JOY_LEFT]->Read() == LOW) {
           circle_ui_key_interrupt(KEYCODE_Left);
        }
        else if (joystickPins2[JOY_RIGHT]->Read() == LOW) {
           circle_ui_key_interrupt(KEYCODE_Right);
        }
        else if (joystickPins2[JOY_FIRE]->Read() == LOW) {
           circle_ui_key_interrupt(KEYCODE_Return);
        }
        return;
     }

     int value = 0;
     if (joystickPins2[JOY_UP]->Read() == LOW) {
        value |= 0x1;
     }
     if (joystickPins2[JOY_DOWN]->Read() == LOW) {
        value |= 0x2;
     }
     if (joystickPins2[JOY_LEFT]->Read() == LOW) {
        value |= 0x4;
     }
     if (joystickPins2[JOY_RIGHT]->Read() == LOW) {
        value |= 0x8;
     }
     if (joystickPins2[JOY_FIRE]->Read() == LOW) {
        value |= 0x10;
     }
     circle_joy_gpio(1, value);
  }
}

void CKernel::InterruptStub (void *pParam)
{
        CKernel *pThis = (CKernel *) pParam;
        assert (pThis != 0);

        if (circle_joy_need_gpio(0))
           pThis->circle_poll_joysticks(0);
        if (circle_joy_need_gpio(1))
           pThis->circle_poll_joysticks(1);
}

