
// plus4emu -- portable Commodore Plus/4 emulator
// Copyright (C) 2003-2016 Istvan Varga <istvanv@users.sourceforge.net>
// https://github.com/istvan-v/plus4emu/
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

#include "plus4emu.hpp"
#include "cpu.hpp"
#include "ted.hpp"
#include "system.hpp"

static const uint8_t tedRegisterInit[32] = {
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1B, 0x08,
  0xFF, 0x04, 0xA0, 0x00, 0xFF, 0xFF, 0x00, 0x00,
  0x7C, 0x00, 0xC4, 0x01, 0x07, 0x80, 0x80, 0x80,
  0x80, 0x80, 0xFF, 0xE8, 0xFE, 0xE0, 0xC8, 0x80
};

namespace Plus4 {

  TED7360::TED7360() : M7501()
  {
    tedRegisters[0x07] = uint8_t(0x00);         // default to PAL mode
    for (int i = 0; i < int(sizeof(callbacks) / sizeof(TEDCallback)); i++) {
      callbacks[i].func = (PLUS4EMU_REGPARM1 void (*)(void *)) 0;
      callbacks[i].userData = (void *) 0;
      callbacks[i].nxt0 = (TEDCallback *) 0;
      callbacks[i].nxt1 = (TEDCallback *) 0;
    }
    firstCallback0 = (TEDCallback *) 0;
    firstCallback1 = (TEDCallback *) 0;
    // create initial memory map
    ramSegments = 0;
    ramPatternCode = 0UL;
    randomSeed = 0;
    Plus4Emu::setRandomSeed(randomSeed,
                            Plus4Emu::Timer::getRandomSeedFromTime());
    for (int i = 0; i < 256; i++)
      segmentTable[i] = (uint8_t *) 0;
    try {
      setRAMSize(64);
    }
    catch (...) {
      for (int i = 0; i < 256; i++) {
        if (segmentTable[i] != (uint8_t *) 0) {
          delete[] segmentTable[i];
          segmentTable[i] = (uint8_t *) 0;
        }
      }
      throw;
    }
    setMemoryCallbackUserData(this);
    for (uint16_t i = 0x0000; i <= 0x0FFF; i++) {
      setMemoryReadCallback(i, &read_memory_0000_to_0FFF);
      setMemoryWriteCallback(i, &write_memory_0000_to_0FFF);
    }
    for (uint16_t i = 0x1000; i <= 0x3FFF; i++) {
      setMemoryReadCallback(i, &read_memory_1000_to_3FFF);
      setMemoryWriteCallback(i, &write_memory_1000_to_3FFF);
    }
    for (uint16_t i = 0x4000; i <= 0x7FFF; i++) {
      setMemoryReadCallback(i, &read_memory_4000_to_7FFF);
      setMemoryWriteCallback(i, &write_memory_4000_to_7FFF);
    }
    for (uint16_t i = 0x8000; i <= 0xBFFF; i++) {
      setMemoryReadCallback(i, &read_memory_8000_to_BFFF);
      setMemoryWriteCallback(i, &write_memory_8000_to_BFFF);
    }
    for (uint16_t i = 0xC000; i <= 0xFBFF; i++) {
      setMemoryReadCallback(i, &read_memory_C000_to_FBFF);
      setMemoryWriteCallback(i, &write_memory_C000_to_FCFF);
    }
    for (uint16_t i = 0xFC00; i <= 0xFCFF; i++) {
      setMemoryReadCallback(i, &read_memory_FC00_to_FCFF);
      setMemoryWriteCallback(i, &write_memory_C000_to_FCFF);
    }
    for (uint16_t i = 0xFD00; i <= 0xFEFF; i++) {
      setMemoryReadCallback(i, &read_memory_FD00_to_FEFF);
      setMemoryWriteCallback(i, &write_memory_FD00_to_FEFF);
    }
    for (uint16_t i = 0xFF00; i <= 0xFF1F; i++) {
      setMemoryReadCallback(i, &read_register_FFxx);
      setMemoryWriteCallback(i, &write_register_FFxx);
    }
    for (uint32_t i = 0xFF20; i <= 0xFFFF; i++) {
      setMemoryReadCallback(uint16_t(i), &read_memory_FF00_to_FFFF);
      setMemoryWriteCallback(uint16_t(i), &write_memory_FF00_to_FFFF);
    }
    // TED register read
    setMemoryReadCallback(0x0000, &read_register_0000);
    setMemoryReadCallback(0x0001, &read_register_0001);
    for (uint16_t i = 0xFD00; i <= 0xFD0F; i++)
      setMemoryReadCallback(i, &read_register_FD0x);
    for (uint16_t i = 0xFD10; i <= 0xFD1F; i++)
      setMemoryReadCallback(i, &read_register_FD1x);
    setMemoryReadCallback(0xFD16, &read_register_FD16);
    for (uint16_t i = 0xFD30; i <= 0xFD3F; i++)
      setMemoryReadCallback(i, &read_register_FD3x);
    setMemoryReadCallback(0xFF00, &read_register_FF00);
    setMemoryReadCallback(0xFF01, &read_register_FF01);
    setMemoryReadCallback(0xFF02, &read_register_FF02);
    setMemoryReadCallback(0xFF03, &read_register_FF03);
    setMemoryReadCallback(0xFF04, &read_register_FF04);
    setMemoryReadCallback(0xFF05, &read_register_FF05);
    setMemoryReadCallback(0xFF06, &read_register_FF06);
    setMemoryReadCallback(0xFF09, &read_register_FF09);
    setMemoryReadCallback(0xFF0A, &read_register_FF0A);
    setMemoryReadCallback(0xFF0C, &read_register_FF0C);
    setMemoryReadCallback(0xFF10, &read_register_FF10);
    setMemoryReadCallback(0xFF12, &read_register_FF12);
    setMemoryReadCallback(0xFF13, &read_register_FF13);
    setMemoryReadCallback(0xFF14, &read_register_FF14);
    setMemoryReadCallback(0xFF1A, &read_register_FF1A);
    setMemoryReadCallback(0xFF1B, &read_register_FF1B);
    setMemoryReadCallback(0xFF1C, &read_register_FF1C);
    setMemoryReadCallback(0xFF1E, &read_register_FF1E);
    setMemoryReadCallback(0xFF1F, &read_register_FF1F);
    setMemoryReadCallback(0xFF3E, &read_register_FF3E_FF3F);
    setMemoryReadCallback(0xFF3F, &read_register_FF3E_FF3F);
    // TED register write
    setMemoryWriteCallback(0x0000, &write_register_0000);
    setMemoryWriteCallback(0x0001, &write_register_0001);
    for (uint16_t i = 0xFD10; i <= 0xFD1F; i++)
      setMemoryWriteCallback(i, &write_register_FD1x);
    setMemoryWriteCallback(0xFD16, &write_register_FD16);
    for (uint16_t i = 0xFD30; i <= 0xFD3F; i++)
      setMemoryWriteCallback(i, &write_register_FD3x);
    for (uint16_t i = 0xFDD0; i <= 0xFDDF; i++)
      setMemoryWriteCallback(i, &write_register_FDDx);
    setMemoryWriteCallback(0xFF00, &write_register_FF00);
    setMemoryWriteCallback(0xFF01, &write_register_FF01);
    setMemoryWriteCallback(0xFF02, &write_register_FF02);
    setMemoryWriteCallback(0xFF03, &write_register_FF03);
    setMemoryWriteCallback(0xFF04, &write_register_FF04);
    setMemoryWriteCallback(0xFF05, &write_register_FF05);
    setMemoryWriteCallback(0xFF06, &write_register_FF06);
    setMemoryWriteCallback(0xFF07, &write_register_FF07);
    setMemoryWriteCallback(0xFF08, &write_register_FF08);
    setMemoryWriteCallback(0xFF09, &write_register_FF09);
    setMemoryWriteCallback(0xFF0A, &write_register_FF0A);
    setMemoryWriteCallback(0xFF0B, &write_register_FF0B);
    setMemoryWriteCallback(0xFF0C, &write_register_FF0C);
    setMemoryWriteCallback(0xFF0D, &write_register_FF0D);
    setMemoryWriteCallback(0xFF0E, &write_register_FF0E);
    setMemoryWriteCallback(0xFF0F, &write_register_FF0F);
    setMemoryWriteCallback(0xFF10, &write_register_FF10);
    setMemoryWriteCallback(0xFF11, &write_register_FF11);
    setMemoryWriteCallback(0xFF12, &write_register_FF12);
    setMemoryWriteCallback(0xFF13, &write_register_FF13);
    setMemoryWriteCallback(0xFF14, &write_register_FF14);
    setMemoryWriteCallback(0xFF15, &write_register_FF15_to_FF19);
    setMemoryWriteCallback(0xFF16, &write_register_FF15_to_FF19);
    setMemoryWriteCallback(0xFF17, &write_register_FF15_to_FF19);
    setMemoryWriteCallback(0xFF18, &write_register_FF15_to_FF19);
    setMemoryWriteCallback(0xFF19, &write_register_FF15_to_FF19);
    setMemoryWriteCallback(0xFF1A, &write_register_FF1A);
    setMemoryWriteCallback(0xFF1B, &write_register_FF1B);
    setMemoryWriteCallback(0xFF1C, &write_register_FF1C);
    setMemoryWriteCallback(0xFF1D, &write_register_FF1D);
    setMemoryWriteCallback(0xFF1E, &write_register_FF1E);
    setMemoryWriteCallback(0xFF1F, &write_register_FF1F);
    setMemoryWriteCallback(0xFF3E, &write_register_FF3E);
    setMemoryWriteCallback(0xFF3F, &write_register_FF3F);
    // initialize external ports
    user_port_state = uint8_t(0xFF);
    tape_read_state = false;
    tape_button_state = false;
    // set internal TED registers
    this->initRegisters();
    cpu_clock_multiplier = 1;
    for (int i = 0; i < 16; i++)                // keyboard matrix
      keyboard_matrix[i] = uint8_t(0xFF);
  }

  void TED7360::initRegisters()
  {
    cycle_count = 0;
    videoColumn = 100;
    // initialize memory paging
    memoryReadMap = 0x06F8U;
    memoryWriteMap = 0x0678U;
    cpuMemoryReadMap = 0x06F8U;
    tedDMAReadMap = 0x07F8U;
    tedBitmapReadMap = 0x07F8U;
    // clear memory used by TED registers
    ioRegister_0000 = uint8_t(0x0F);
    ioRegister_0001 = uint8_t(0xC8);
    if (tedRegisters[0x07] & uint8_t(0x40)) {
      tedRegisters[0x07] = uint8_t(0x00);
      ntscModeChangeCallback(false);
    }
    for (uint8_t i = 0x00; i <= 0x1F; i++)
      tedRegisters[i] = tedRegisterInit[i];
    // set internal TED registers
    render_func = &render_char_std;
    current_render_func = &render_border;
    videoLine = 224;
    characterLine = 0;
    characterPosition = 0x0000;
    savedCharacterPosition = 0x0000;
    characterPositionReload = 0x0000;
    characterColumn = 0;
    dmaPosition = 0x03FF;
    dmaPositionReload = 0x03FF;
    dmaBaseAddr = 0x0000;
    bitmap_base_addr = 0x0000;
    charset_base_addr = 0x0000;
    cursor_position = 0x03FF;
    ted_disabled = false;
    flashState = 0x00;
    renderWindow = false;
    incrementingCharacterLine = false;
    bitmapAddressDisableFlags = 0x03;
    displayWindow = false;
    displayActive = false;
    videoOutputFlags = 0x30;
    vsyncFlags = 0x00;
    timer1_run = true;                          // timers
    timer2_run = true;
    timer3_run = true;
    timer1_state = 0;
    timer1_reload_value = 0;
    timer2_state = 0;
    timer3_state = 0;
    soundChannel1Cnt = 0x03FF;                  // sound generators
    soundChannel1Reload = 0x03FF;
    soundChannel2Cnt = 0x03FF;
    soundChannel2Reload = 0x03FF;
    prvSoundChannel1Overflow = false;
    prvSoundChannel2Overflow = false;
    soundChannel1Decay = soundDecayCycles;
    soundChannel2Decay = soundDecayCycles;
    soundChannel1State = uint8_t(1);
    soundChannel2State = uint8_t(1);
    soundChannel2NoiseState = uint8_t(0xFF);
    prvCycleCount = 0;
    soundFlags = 0x00;
    soundVolume = 0x00;
    soundOutput = 0x00;
    prvSoundOutput = 0x00;
    for (int i = 0; i < 64; i++) {              // video buffers
      attr_buf[i] = uint8_t(0);
      attr_buf_tmp[i] = uint8_t(0);
      char_buf[i] = uint8_t(0);
    }
    for (int i = 0; i < 464; i++)
      video_buf[i] = uint8_t(0x00);
    prv_video_buf_pos = 0;
    video_buf_pos = 0;
    videoShiftRegisterEnabled = false;
    videoMode = 0x00;
    characterMask = uint8_t(0x7F);
    for (int i = 0; i < 5; i++)
      colorRegisters[i] = uint8_t(0x80);
    horizontalScroll = 0;
    verticalScroll = 3;
    dmaEnabled = false;
    savedVideoLineDelay1 = 223;
    singleClockModeFlags = 0x00;
    externalFetchSingleClockFlag = true;
    dmaFlags = 0x00;
    dmaActive = false;
    incrementingDMAPosition = false;
    incrementingCharacterPosition = false;
    cpuHaltedFlag = false;
    singleClockCycleLength = 20;
    delayedEvents0 = 0U;
    delayedEvents1 = 0U;
    savedVideoLine = 224;
    videoInterruptLine = 0;
    prvVideoInterruptState = false;
    prvCharacterLine = 0;
    dataBusState = uint8_t(0xFF);
    dramRefreshAddrL = 0x00;
    keyboard_row_select_mask = 0xFFFF;
    tape_motor_state = false;
    tape_write_state = false;
    hannesRegister = uint8_t(0xFF);
  }

  void TED7360::reset(bool cold_reset)
  {
    if (cold_reset) {
      // reset TED registers
      this->initRegisters();
      // make sure that RAM size is detected correctly
      for (uint8_t j = 0xC0; j < 0xFF; j++) {
        if ((j & uint8_t(0x02)) != uint8_t(0) &&
            !(j == uint8_t(0xFE) && segmentTable[0xFC] == (uint8_t *) 0))
          continue;
        if (segmentTable[j] != (uint8_t *) 0) {
          for (uint16_t i = 0x3FFD; i > 0x3FF5; i--) {
            if (segmentTable[j][i] != readMemory(i | 0xC000))
              break;
            if (i == 0x3FF6) {
              segmentTable[j][i] =
                  (segmentTable[j][i] + uint8_t(1)) & uint8_t(0xFF);
            }
          }
        }
      }
      // force RAM testing
      writeMemory(0x0508, 0x00);
    }
    // reset ROM banking
    writeMemory(0xFDD0, 0x00);
    // reset CPU
    writeMemory(0x0000, 0x00);
    writeMemory(0x0001, 0x00);
    M7501::reset(cold_reset);
  }

  TED7360::~TED7360()
  {
    for (int i = 0; i < 256; i++) {
      if (segmentTable[i] != (uint8_t *) 0) {
        delete[] segmentTable[i];
        segmentTable[i] = (uint8_t *) 0;
      }
    }
  }

}       // namespace Plus4

