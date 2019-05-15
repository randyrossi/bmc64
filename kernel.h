//
// kernel.h
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

#ifndef _kernel_h
#define _kernel_h

#include "viceapp.h"
#include <setjmp.h>

#include "vicescreen.h"
#include "vicesound.h"
#include <circle/actled.h>
#include <circle/cputhrottle.h>
#include <circle/devicenameservice.h>
#include <circle/exceptionhandler.h>
#include <circle/input/mouse.h>
#include <circle/interrupt.h>
#include <circle/logger.h>
#include <circle/serial.h>
#include <circle/spinlock.h>
#include <circle/timer.h>
#include <circle/types.h>
#include <circle/usb/usbgamepad.h>
#include <circle/usb/usbkeyboard.h>
#include <circle/usertimer.h>
#include <stdint.h>
#include <vc4/vchiq/vchiqdevice.h>

extern "C" {

#include "third_party/vice-3.3/src/arch/raspi/circle.h"
#include "third_party/vice-3.3/src/arch/raspi/keycodes.h"
#include "third_party/vice-3.3/src/main.h"
}

class CKernel : public ViceStdioApp {
public:
  CKernel(void);

  bool Initialize(void) override;
  TShutdownMode Run(void);

  static void MouseStatusHandler(unsigned nButtons, int nPosX, int nPosY);
  static void KeyStatusHandlerRaw(unsigned char ucModifiers,
                                  const unsigned char RawKeys[6]);
  static void GamePadStatusHandler(unsigned nDeviceIndex,
                                   const TGamePadState *pState);

  ssize_t vice_write(int fd, const void *buf, size_t count);
  int circle_get_machine_timing();
  uint8_t *circle_get_fb();
  int circle_get_fb_pitch();
  void circle_sleep(long delay);
  void circle_set_palette(uint8_t index, uint16_t rgb565);
  void circle_update_palette();
  int circle_get_display_w();
  int circle_get_display_h();
  unsigned long circle_get_ticks();
  void circle_set_fb_y(int loc);
  void circle_wait_vsync();

  int circle_sound_init(const char *param, int *speed, int *fragsize,
                        int *fragnr, int *channels);
  int circle_sound_write(int16_t *pbuf, size_t nr);
  void circle_sound_close(void);
  int circle_sound_suspend(void);
  int circle_sound_resume(void);
  int circle_sound_bufferspace(void);
  void circle_yield(void);
  void circle_check_gpio();
  void circle_lock_acquire();
  void circle_lock_release();
  void circle_boot_complete();
  int circle_cycles_per_second();

private:
  void InitSound();
  void SetupUSBKeyboard();
  void SetupUSBMouse();
  int GetGpioPinState(int pinIndex);
  void ScanKeyboard();
  void ReadJoystick(int device);
  void ReadJoystickOld(int device);

  static bool uiShift;

  CScheduler mScheduler;
  CVCHIQDevice mVCHIQ;
  ViceSound *mViceSound;
  CCPUThrottle mCPUThrottle;
  CSpinLock m_Lock;

  int gpio_debounce_state[NUM_GPIO_PINS];
};

#endif
