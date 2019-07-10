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

#include "fb2.h"

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
  void circle_sleep(long delay);
  unsigned long circle_get_ticks();
  void circle_wait_vsync();

  uint8_t *circle_get_fb1();
  int circle_get_fb1_pitch();
  int circle_get_fb1_w();
  int circle_get_fb1_h();
  void circle_set_fb1_palette(uint8_t index, uint16_t rgb565);
  void circle_update_fb1_palette();
  void circle_set_fb1_y(int loc);

  // New FB2 stuff to replace the default frame buffer
  // TODO: Use an index here instead of 'fb2' convention
  int circle_alloc_fb2(uint8_t **pixels, int width, int height, int *pitch);
  void circle_free_fb2();
  void circle_clear_fb2();
  void circle_show_fb2();
  void circle_hide_fb2();
  void circle_frame_ready_fb2();

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
  int ReadDebounced(int pinIndex);
  void ScanKeyboard();
  void ReadJoystick(int device, bool usePcb);

  ViceSound *mViceSound;
  CCPUThrottle mCPUThrottle;
  CSpinLock m_Lock;
  int mNumJoy;

  int gpio_debounce_state[NUM_GPIO_PINS];

  // TODO: Have three of these eventually. VIC, VDC and UI
  FrameBuffer2 fb2;
};

#endif
