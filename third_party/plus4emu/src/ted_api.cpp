
// plus4emu -- portable Commodore Plus/4 emulator
// Copyright (C) 2003-2017 Istvan Varga <istvanv@users.sourceforge.net>
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
#include "fileio.hpp"
#include "cpu.hpp"
#include "ted.hpp"
#include "system.hpp"

#include <cmath>

static const float brightnessToYTable[9] = {
  2.00f,  2.42f,  2.60f,  2.70f,  2.90f,  3.35f,  3.75f,  4.10f,  4.80f
};

static const float colorPhaseTablePAL[16] = {
    0.0f,    0.0f,  103.0f,  283.0f,   53.0f,  240.0f,  347.0f,  167.0f,
  130.0f,  148.0f,  195.0f,   83.0f,  265.0f,  323.0f,    3.0f,  213.0f
};

static const float colorPhaseTableNTSC[16] = {
    0.0f,    0.0f,  103.0f,  283.0f,   53.0f,  240.0f,  347.0f,  167.0f,
  125.0f,  148.0f,  195.0f,   83.0f,  265.0f,  323.0f,   23.0f,  213.0f
};

namespace Plus4 {

  void TED7360::convertPixelToYUV(uint8_t color, bool isNTSC,
                                  float& y, float& u, float& v)
  {
    const float yMin = 0.033f;
    const float yMax = 0.956f;
    uint8_t c = color & 0x0F;
    uint8_t b = (color & 0x70) >> 4;
    y = brightnessToYTable[(c != 0 ? (b + 1) : 0)] - brightnessToYTable[0];
    y = y * (yMax - yMin) / (brightnessToYTable[8] - brightnessToYTable[0]);
    y = y + yMin;
    u = 0.0f;
    v = 0.0f;
    if (c > 1) {
      float   phs = 0.0f;
      if (!isNTSC)
        phs = colorPhaseTablePAL[c];
      else
        phs = colorPhaseTableNTSC[c];
      phs *= (3.14159265f / 180.0f);
      u = float(std::cos(phs)) * 0.19f;
      v = float(std::sin(phs)) * 0.19f;
    }
  }

  void TED7360::setCPUClockMultiplier(int clk)
  {
    if (clk < 1)
      cpu_clock_multiplier = 1;
    else if (clk > 100)
      cpu_clock_multiplier = 100;
    else
      cpu_clock_multiplier = clk;
  }

  // set the state of the specified key
  // valid key numbers are:
  //
  //     0: Del          1: Return       2: £            3: Help
  //     4: F1           5: F2           6: F3           7: @
  //     8: 3            9: W           10: A           11: 4
  //    12: Z           13: S           14: E           15: Shift
  //    16: 5           17: R           18: D           19: 6
  //    20: C           21: F           22: T           23: X
  //    24: 7           25: Y           26: G           27: 8
  //    28: B           29: H           30: U           31: V
  //    32: 9           33: I           34: J           35: 0
  //    36: M           37: K           38: O           39: N
  //    40: Down        41: P           42: L           43: Up
  //    44: .           45: :           46: -           47: ,
  //    48: Left        49: *           50: ;           51: Right
  //    52: Esc         53: =           54: +           55: /
  //    56: 1           57: Home        58: Ctrl        59: 2
  //    60: Space       61: C=          62: Q           63: Stop
  //
  //    72: Joy2 Up     73: Joy2 Down   74: Joy2 Left   75: Joy2 Right
  //    79: Joy2 Fire
  //    80: Joy1 Up     81: Joy1 Down   82: Joy1 Left   83: Joy1 Right
  //    86: Joy1 Fire

  void TED7360::setKeyState(int keyNum, bool isPressed)
  {
    int     ndx = (keyNum & 0x78) >> 3;
    int     mask = 1 << (keyNum & 7);

    if (isPressed)
      keyboard_matrix[ndx] &= ((uint8_t) mask ^ (uint8_t) 0xFF);
    else
      keyboard_matrix[ndx] |= (uint8_t) mask;
  }

  void TED7360::setCallback(PLUS4EMU_REGPARM1 void (*func)(void *userData),
                            void *userData_, int flags_)
  {
    if (!func)
      return;
    flags_ = flags_ & 3;
    int     ndx = -1;
    for (size_t i = 0; i < (sizeof(callbacks) / sizeof(TEDCallback)); i++) {
      if (callbacks[i].func == func && callbacks[i].userData == userData_) {
        ndx = int(i);
        break;
      }
    }
    if (ndx >= 0) {
      TEDCallback *prv = (TEDCallback *) 0;
      TEDCallback *p = firstCallback0;
      while (p) {
        if (p == &(callbacks[ndx])) {
          if (prv)
            prv->nxt0 = p->nxt0;
          else
            firstCallback0 = p->nxt0;
          break;
        }
        prv = p;
        p = p->nxt0;
      }
      prv = (TEDCallback *) 0;
      p = firstCallback1;
      while (p) {
        if (p == &(callbacks[ndx])) {
          if (prv)
            prv->nxt1 = p->nxt1;
          else
            firstCallback1 = p->nxt1;
          break;
        }
        prv = p;
        p = p->nxt1;
      }
      if (flags_ == 0) {
        callbacks[ndx].func = (PLUS4EMU_REGPARM1 void (*)(void *)) 0;
        callbacks[ndx].userData = (void *) 0;
        callbacks[ndx].nxt0 = (TEDCallback *) 0;
        callbacks[ndx].nxt1 = (TEDCallback *) 0;
      }
    }
    if (flags_ == 0)
      return;
    if (ndx < 0) {
      for (size_t i = 0; i < (sizeof(callbacks) / sizeof(TEDCallback)); i++) {
        if (!(callbacks[i].func)) {
          ndx = int(i);
          break;
        }
      }
      if (ndx < 0)
        throw Plus4Emu::Exception("TED7360: too many callbacks");
    }
    callbacks[ndx].func = func;
    callbacks[ndx].userData = userData_;
    callbacks[ndx].nxt0 = (TEDCallback *) 0;
    callbacks[ndx].nxt1 = (TEDCallback *) 0;
    if (flags_ & 1) {
      TEDCallback *prv = (TEDCallback *) 0;
      TEDCallback *p = firstCallback0;
      while (p) {
        prv = p;
        p = p->nxt0;
      }
      p = &(callbacks[ndx]);
      if (prv)
        prv->nxt0 = p;
      else
        firstCallback0 = p;
    }
    if (flags_ & 2) {
      TEDCallback *prv = (TEDCallback *) 0;
      TEDCallback *p = firstCallback1;
      while (p) {
        prv = p;
        p = p->nxt1;
      }
      p = &(callbacks[ndx]);
      if (prv)
        prv->nxt1 = p;
      else
        firstCallback1 = p;
    }
  }

  bool TED7360::checkLightPen(int xPos, int yPos) const
  {
    if (savedVideoLine != yPos)
      return false;
    if (yPos < 0 || yPos > 311 || xPos < 0 || xPos > 455)
      return false;
    if (videoColumn != uint8_t(xPos < 452 ? ((xPos + 4) >> 2) : 0))
      return false;
    if (video_buf[prv_video_buf_pos] & 0x02) {
      if (video_buf[prv_video_buf_pos + 1 + (xPos & 3)] & 0x0F)
        return true;
    }
    else {
      if (video_buf[prv_video_buf_pos + 1] & 0x0F)
        return true;
    }
    return false;
  }

  // --------------------------------------------------------------------------

  class ChunkType_TED7360Snapshot : public Plus4Emu::File::ChunkTypeHandler {
   private:
    TED7360&  ref;
   public:
    ChunkType_TED7360Snapshot(TED7360& ref_)
      : Plus4Emu::File::ChunkTypeHandler(),
        ref(ref_)
    {
    }
    virtual ~ChunkType_TED7360Snapshot()
    {
    }
    virtual Plus4Emu::File::ChunkType getChunkType() const
    {
      return Plus4Emu::File::PLUS4EMU_CHUNKTYPE_TED_STATE;
    }
    virtual void processChunk(Plus4Emu::File::Buffer& buf)
    {
      ref.loadState(buf);
    }
  };

  class ChunkType_Plus4Program : public Plus4Emu::File::ChunkTypeHandler {
   private:
    TED7360&  ref;
   public:
    ChunkType_Plus4Program(TED7360& ref_)
      : Plus4Emu::File::ChunkTypeHandler(),
        ref(ref_)
    {
    }
    virtual ~ChunkType_Plus4Program()
    {
    }
    virtual Plus4Emu::File::ChunkType getChunkType() const
    {
      return Plus4Emu::File::PLUS4EMU_CHUNKTYPE_PLUS4_PRG;
    }
    virtual void processChunk(Plus4Emu::File::Buffer& buf)
    {
      ref.loadProgram(buf);
    }
  };

  void TED7360::saveState(Plus4Emu::File::Buffer& buf)
  {
    buf.setPosition(0);
    buf.writeUInt32(0x01000005);        // version number
    uint8_t   romBitmap = 0;
    for (uint8_t i = 0; i < 8; i++) {
      // find non-empty ROM segments
      romBitmap = romBitmap >> 1;
      if (segmentTable[i] != (uint8_t *) 0)
        romBitmap = romBitmap | uint8_t(0x80);
    }
    buf.writeByte(romBitmap);
    buf.writeByte(ramSegments);
    // save RAM segments
    for (size_t i = 0x08; i <= 0xFF; i++) {
      if (segmentTable[i] != (uint8_t *) 0) {
        for (size_t j = 0; j < 16384; j++)
          buf.writeByte(segmentTable[i][j]);
      }
    }
    // save ROM segments
    for (size_t i = 0x00; i < 0x08; i++) {
      if (segmentTable[i] != (uint8_t *) 0) {
        for (size_t j = 0; j < 16384; j++)
          buf.writeByte(segmentTable[i][j]);
      }
    }
    // save I/O and TED registers
    buf.writeByte(ioRegister_0000);
    buf.writeByte(ioRegister_0001);
    for (uint8_t i = 0x00; i <= 0x19; i++)
      buf.writeByte(readMemoryCPU(uint16_t(0xFF00) | uint16_t(i)));
    for (uint8_t i = 0x1A; i <= 0x1F; i++)
      buf.writeByte(tedRegisters[i]);
    // save memory paging
    buf.writeByte(hannesRegister);
    buf.writeByte((uint8_t(cpuMemoryReadMap) & uint8_t(0x80))
                  | (uint8_t(cpuMemoryReadMap >> 11) & uint8_t(0x0F)));
    // save internal registers
    buf.writeByte((4 - cycle_count) & 3);
    buf.writeByte(videoColumn);
    buf.writeUInt32(uint32_t(videoLine));
    buf.writeByte(uint8_t(characterLine));
    buf.writeUInt32(uint32_t(characterPosition));
    buf.writeUInt32(uint32_t(savedCharacterPosition));
    buf.writeUInt32(uint32_t(characterPositionReload));
    buf.writeByte(uint8_t(characterColumn));
    buf.writeUInt32(uint32_t(dmaPosition | (dmaBaseAddr & 0x0400)));
    buf.writeUInt32(uint32_t(dmaPositionReload));
    buf.writeByte(flashState);
    buf.writeBoolean(renderWindow);
    buf.writeBoolean(incrementingCharacterLine);
    buf.writeByte(bitmapAddressDisableFlags);
    buf.writeBoolean(displayWindow);
    buf.writeBoolean(displayActive);
    buf.writeByte(videoOutputFlags);
    buf.writeBoolean(timer1_run);
    buf.writeBoolean(timer2_run);
    buf.writeBoolean(timer3_run);
    buf.writeUInt32(uint32_t(timer1_state));
    buf.writeUInt32(uint32_t(timer1_reload_value));
    buf.writeUInt32(uint32_t(timer2_state));
    buf.writeUInt32(uint32_t(timer3_state));
    buf.writeUInt32((((uint32_t(soundChannel1Cnt) - 1U) ^ 0x03FFU) - 1U)
                    & 0x03FFU);
    buf.writeUInt32((((uint32_t(soundChannel2Cnt) - 1U) ^ 0x03FFU) - 1U)
                    & 0x03FFU);
    buf.writeBoolean(prvSoundChannel1Overflow);
    buf.writeBoolean(prvSoundChannel2Overflow);
    buf.writeUInt32(soundChannel1Decay);
    buf.writeUInt32(soundChannel2Decay);
    buf.writeByte(soundChannel1State);
    buf.writeByte(soundChannel2State);
    buf.writeByte(soundChannel2NoiseState);
    buf.writeByte(prvCycleCount);
    buf.writeBoolean(videoShiftRegisterEnabled);
    buf.writeByte(shiftRegisterCharacter.bitmap_());
    buf.writeByte(shiftRegisterCharacter.attr_());
    buf.writeByte(shiftRegisterCharacter.char_());
    buf.writeByte(shiftRegisterCharacter.flags_() & uint8_t(0xF8));
    buf.writeByte(currentCharacter.attr_());
    buf.writeByte(currentCharacter.char_());
    buf.writeByte(currentCharacter.bitmap_());
    buf.writeByte(currentCharacter.flags_() & uint8_t(0xF8));
    buf.writeByte(nextCharacter.attr_());
    buf.writeByte(nextCharacter.char_());
    buf.writeByte(nextCharacter.bitmap_());
    buf.writeByte(nextCharacter.flags_() & uint8_t(0xF8));
    buf.writeBoolean(dmaEnabled);
    buf.writeUInt32(uint32_t(savedVideoLineDelay1));
    buf.writeByte(singleClockModeFlags & uint8_t(0x83));
    buf.writeBoolean(externalFetchSingleClockFlag);
    buf.writeByte(dmaFlags);
    buf.writeBoolean(incrementingDMAPosition);
    buf.writeBoolean(incrementingCharacterPosition);
    buf.writeBoolean(cpuHaltedFlag);
    buf.writeUInt32(uint32_t(delayedEvents0));
    buf.writeUInt32(uint32_t(delayedEvents1));
    buf.writeUInt32(uint32_t(savedVideoLine));
    buf.writeBoolean(prvVideoInterruptState);
    buf.writeByte(prvCharacterLine);
    buf.writeByte(vsyncFlags);
    buf.writeByte(dataBusState);
    buf.writeByte(dramRefreshAddrL);
    buf.writeUInt32(uint32_t(keyboard_row_select_mask));
    for (int i = 0; i < 16; i++)
      buf.writeByte(keyboard_matrix[i]);
    buf.writeByte(user_port_state);
  }

  void TED7360::saveState(Plus4Emu::File& f)
  {
    {
      Plus4Emu::File::Buffer  buf;
      this->saveState(buf);
      f.addChunk(Plus4Emu::File::PLUS4EMU_CHUNKTYPE_TED_STATE, buf);
    }
    M7501::saveState(f);
  }

  void TED7360::loadState(Plus4Emu::File::Buffer& buf)
  {
    buf.setPosition(0);
    // check version number
    unsigned int  version = buf.readUInt32();
    if (!(version >= 0x01000000 && version <= 0x01000005)) {
      buf.setPosition(buf.getDataSize());
      throw Plus4Emu::Exception("incompatible Plus/4 snapshot format");
    }
    try {
      this->reset(true);
      // load saved state
      uint8_t   romBitmap = buf.readByte();
      ramSegments = buf.readByte();
      if (!(ramSegments == 1 || ramSegments == 2 || ramSegments == 4 ||
            ramSegments == 16 || ramSegments == 64))
        throw Plus4Emu::Exception("incompatible Plus/4 snapshot data");
      // load RAM segments
      setRAMSize(size_t(ramSegments) << 4);
      for (size_t i = 0x08; i <= 0xFF; i++) {
        if (segmentTable[i] != (uint8_t *) 0) {
          for (size_t j = 0; j < 16384; j++)
            segmentTable[i][j] = buf.readByte();
        }
      }
      // load ROM segments
      for (uint8_t i = 0x00; i < 0x08; i++) {
        if (!(romBitmap & (uint8_t(1) << i)))
          loadROM(int(i >> 1), int(i & 1) << 14, 0, (uint8_t *) 0);
        else {
          uint8_t tmp = 0;
          loadROM(int(i >> 1), int(i & 1) << 14, 1, &tmp);
          for (size_t j = 0; j < 16384; j++)
            segmentTable[i][j] = buf.readByte();
        }
      }
      // load I/O and TED registers
      ioRegister_0000 = buf.readByte();
      ioRegister_0001 = buf.readByte();
      writeMemory(0x0000, ioRegister_0000);
      writeMemory(0x0001, ioRegister_0001);
      for (uint8_t i = 0x00; i <= 0x1F; i++) {
        uint8_t c = buf.readByte();
        if (i == 0x06 || i == 0x07 || (i >= 0x0A && i <= 0x19))
          writeMemory(uint16_t(0xFF00) | uint16_t(i), c);
        else
          tedRegisters[i] = c;
      }
      tedRegisters[0x09] &= uint8_t(0x5E);
      updateInterruptFlag();
      delayedEvents0 = 0U;
      delayedEvents1 = 0U;
      // load memory paging
      hannesRegister = buf.readByte();
      uint8_t romSelect_ = buf.readByte() & uint8_t(0x8F);
      // update internal registers according to the new RAM image loaded
      write_register_FD16(this, 0xFD16, hannesRegister);
      if (romSelect_ & uint8_t(0x80))
        write_register_FF3E(this, 0xFF3E, 0x00);
      else
        write_register_FF3F(this, 0xFF3F, 0x00);
      write_register_FDDx(this, uint16_t(0xFDD0) | uint16_t(romSelect_), 0x00);
      // load remaining internal registers from snapshot data
      if (version < 0x01000003)
        (void) buf.readUInt32();        // was tedRegisterWriteMask
      cycle_count = buf.readByte() & 0x03;
      cycle_count = (4 - cycle_count) & 3;
      videoColumn = buf.readByte() & 0x7F;
      if (version < 0x01000002) {
        videoColumn =
            uint8_t(videoColumn != 113 ? ((videoColumn + 1) & 0x7F) : 0);
      }
      videoLine = int(buf.readUInt32() & 0x01FF);
      characterLine = buf.readByte() & 7;
      characterPosition = int(buf.readUInt32() & 0x03FF);
      if (version >= 0x01000003)
        savedCharacterPosition = int(buf.readUInt32() & 0x03FF);
      else
        savedCharacterPosition = characterPosition;
      characterPositionReload = int(buf.readUInt32() & 0x03FF);
      characterColumn = buf.readByte() & 0x3F;
      dmaPosition = int(buf.readUInt32() & 0x07FF);
      dmaBaseAddr = (dmaBaseAddr & 0xF800) | (dmaPosition & 0x0400);
      dmaPosition = dmaPosition & 0x03FF;
      dmaPositionReload = int(buf.readUInt32() & 0x03FF);
      flashState = uint8_t(buf.readByte() == 0x00 ? 0x00 : 0xFF);
      renderWindow = buf.readBoolean();
      incrementingCharacterLine = buf.readBoolean();
      bitmapAddressDisableFlags = buf.readByte() & 0x03;
      displayWindow = buf.readBoolean();
      if (version < 0x01000003)
        (void) buf.readBoolean();       // was renderingDisplay
      displayActive = buf.readBoolean();
      if (version >= 0x01000002) {
        videoOutputFlags = uint8_t((videoOutputFlags & 0x01)
                                   | (buf.readByte() & 0xFC));
      }
      else {
        uint8_t tmp = buf.readByte();
        videoOutputFlags = uint8_t((videoOutputFlags & 0x01)
                                   | ((tmp & 0x01) << 5) | ((tmp & 0x02) << 3));
      }
      timer1_run = buf.readBoolean();
      timer2_run = buf.readBoolean();
      timer3_run = buf.readBoolean();
      timer1_state = int(buf.readUInt32() & 0xFFFF);
      timer1_reload_value = int(buf.readUInt32() & 0xFFFF);
      timer2_state = int(buf.readUInt32() & 0xFFFF);
      timer3_state = int(buf.readUInt32() & 0xFFFF);
      soundChannel1Cnt = uint16_t((((buf.readUInt32() + 1U) ^ 0x03FFU)
                                   & 0x03FFU) + 1U);
      soundChannel2Cnt = uint16_t((((buf.readUInt32() + 1U) ^ 0x03FFU)
                                   & 0x03FFU) + 1U);
      if (version >= 0x01000004) {
        prvSoundChannel1Overflow = buf.readBoolean();
        prvSoundChannel2Overflow = buf.readBoolean();
        soundChannel1Decay = buf.readUInt32();
        soundChannel2Decay = buf.readUInt32();
      }
      else {
        prvSoundChannel1Overflow = (soundChannel1Reload == 0x0001);
        prvSoundChannel2Overflow = (soundChannel2Reload == 0x0001);
        soundChannel1Decay = soundDecayCycles;
        soundChannel2Decay = soundDecayCycles;
      }
      soundChannel1State = uint8_t(buf.readByte() == uint8_t(0) ? 0 : 1);
      soundChannel2State = uint8_t(buf.readByte() == uint8_t(0) ? 0 : 1);
      soundChannel2NoiseState = buf.readByte();
      if (version >= 0x01000005) {
        prvCycleCount = buf.readByte() & 0x03;
      }
      else {
        (void) buf.readByte();          // was soundChannel2NoiseOutput
        prvCycleCount = 3;
      }
      updateSoundOutput();
      videoShiftRegisterEnabled = buf.readBoolean();
      shiftRegisterCharacter.bitmap_() = buf.readByte();
      if (version == 0x01000000)
        (void) buf.readUInt32();        // was bitmapMShiftRegister
      if (version < 0x01000003)
        (void) buf.readByte();          // was horizontalScroll
      shiftRegisterCharacter.attr_() = buf.readByte();
      shiftRegisterCharacter.char_() = buf.readByte();
      if (version >= 0x01000003)
        shiftRegisterCharacter.flags_() = buf.readByte() & 0xF8;
      else
        shiftRegisterCharacter.flags_() = (buf.readBoolean() ? 0xF8 : 0x08);
      currentCharacter.attr_() = buf.readByte();
      currentCharacter.char_() = buf.readByte();
      currentCharacter.bitmap_() = buf.readByte();
      if (version >= 0x01000003)
        currentCharacter.flags_() = buf.readByte() & 0xF8;
      else
        currentCharacter.flags_() = (buf.readBoolean() ? 0xF8 : 0x08);
      nextCharacter.attr_() = buf.readByte();
      nextCharacter.char_() = buf.readByte();
      nextCharacter.bitmap_() = buf.readByte();
      if (version >= 0x01000003) {
        nextCharacter.flags_() = buf.readByte() & 0xF8;
        dmaEnabled = buf.readBoolean();
        savedVideoLineDelay1 = int(buf.readUInt32() & 0x01FF);
        singleClockModeFlags = buf.readByte() & 0x83;
        externalFetchSingleClockFlag = buf.readBoolean();
      }
      else {
        nextCharacter.flags_() = (buf.readBoolean() ? 0xF8 : 0x08);
        dmaEnabled = buf.readBoolean();
        singleClockModeFlags = buf.readByte();
        singleClockModeFlags = (singleClockModeFlags & 0x02)
                               | ((singleClockModeFlags & 0x01) << 7);
        (void) buf.readBoolean();       // was singleClockModeFlags
        uint8_t dmaCycleCounter = buf.readByte();
        cpuHaltedFlag = (dmaCycleCounter >= 7);
        if (dmaCycleCounter >= 2 && dmaCycleCounter <= 6)
          delayedEvents0.dmaCycle(dmaCycleCounter - 1);
        externalFetchSingleClockFlag = (videoColumn >= 101 || videoColumn < 75);
      }
      dmaFlags = buf.readByte() & 0x83;
      if (version < 0x01000003) {
        if (videoColumn >= 103 || videoColumn < 75)
          dmaFlags = dmaFlags | 0x80;
        else
          dmaFlags = dmaFlags & 0x7F;
      }
      incrementingDMAPosition = buf.readBoolean();
      if (version >= 0x01000003) {
        incrementingCharacterPosition = buf.readBoolean();
        cpuHaltedFlag = buf.readBoolean();
        delayedEvents0 = buf.readUInt32();
        delayedEvents1 = buf.readUInt32();
      }
      else {
        incrementingCharacterPosition =
            (videoColumn >= 111 || videoColumn < 75);
      }
      dmaActive = (cpuHaltedFlag || delayedEvents0.dmaStarted());
      delayedEvents0.setVerticalScroll();
      delayedEvents0.setHorizontalScroll();
      delayedEvents0.selectRenderer();
      delayedEvents0.setForceSingleClockFlag();
      for (uint8_t i = 0; i < 5; i++)
        delayedEvents0.setColorRegister(i);
      savedVideoLine = int(buf.readUInt32() & 0x01FF);
      if (version < 0x01000003) {
        savedVideoLineDelay1 = savedVideoLine;
        if (videoColumn == 99 || videoColumn == 100) {
          if (savedVideoLineDelay1 > 0)
            savedVideoLineDelay1--;
          else
            savedVideoLineDelay1 = ((tedRegisters[0x07] & 0x40) ? 261 : 311);
        }
      }
      prvVideoInterruptState = buf.readBoolean();
      if (version < 0x01000002)
        checkVideoInterrupt();
      prvCharacterLine = buf.readByte() & 7;
      if (version >= 0x01000002)
        vsyncFlags = buf.readByte() & 0xC0;
      else
        (void) buf.readByte();  // was invertColorPhaseFlags in old versions
      dataBusState = buf.readByte();
      if (version >= 0x01000003)
        dramRefreshAddrL = buf.readByte();
      keyboard_row_select_mask = int(buf.readUInt32() & 0xFFFF);
      for (int i = 0; i < 16; i++)
        keyboard_matrix[i] = buf.readByte();
      user_port_state = buf.readByte();
      updateVideoMode();
      if (buf.getPosition() != buf.getDataSize())
        throw Plus4Emu::Exception("trailing garbage at end of "
                                  "Plus/4 snapshot data");
    }
    catch (...) {
      for (int i = 0; i < 16; i++)
        keyboard_matrix[i] = 0xFF;
      try {
        this->reset(true);
      }
      catch (...) {
      }
      throw;
    }
  }

  void TED7360::saveProgram(Plus4Emu::File::Buffer& buf)
  {
    uint16_t  startAddr, endAddr, len;
    startAddr = uint16_t(readMemoryCPU(0x002B))
                | (uint16_t(readMemoryCPU(0x002C)) << 8);
    endAddr = uint16_t(readMemoryCPU(0x002D))
              | (uint16_t(readMemoryCPU(0x002E)) << 8);
    len = (endAddr > startAddr ? (endAddr - startAddr) : uint16_t(0));
    buf.writeUInt32(startAddr);
    buf.writeUInt32(len);
    while (len) {
      buf.writeByte(readMemoryCPU(startAddr, true));
      startAddr = (startAddr + 1) & 0xFFFF;
      len--;
    }
  }

  void TED7360::saveProgram(Plus4Emu::File& f)
  {
    Plus4Emu::File::Buffer  buf;
    this->saveProgram(buf);
    f.addChunk(Plus4Emu::File::PLUS4EMU_CHUNKTYPE_PLUS4_PRG, buf);
  }

  void TED7360::saveProgram(const char *fileName)
  {
    if (fileName == (char *) 0 || fileName[0] == '\0')
      throw Plus4Emu::Exception("invalid plus4 program file name");
    std::FILE *f = Plus4Emu::fileOpen(fileName, "wb");
    if (!f)
      throw Plus4Emu::Exception("error opening plus4 program file");
    uint16_t  startAddr, endAddr, len;
    startAddr = uint16_t(readMemoryCPU(0x002B))
                | (uint16_t(readMemoryCPU(0x002C)) << 8);
    endAddr = uint16_t(readMemoryCPU(0x002D))
              | (uint16_t(readMemoryCPU(0x002E)) << 8);
    len = (endAddr > startAddr ? (endAddr - startAddr) : uint16_t(0));
    bool  err = true;
    if (std::fputc(int(startAddr & 0xFF), f) != EOF) {
      if (std::fputc(int((startAddr >> 8) & 0xFF), f) != EOF) {
        while (len) {
          int   c = readMemoryCPU(startAddr, true);
          if (std::fputc(c, f) == EOF)
            break;
          startAddr = (startAddr + 1) & 0xFFFF;
          len--;
        }
        err = (len != 0);
      }
    }
    if (std::fclose(f) != 0)
      err = true;
    if (err)
      throw Plus4Emu::Exception("error writing plus4 program file "
                                "-- is the disk full ?");
  }

  void TED7360::loadProgram(Plus4Emu::File::Buffer& buf)
  {
    buf.setPosition(0);
    uint32_t  addr = buf.readUInt32();
    uint32_t  len = buf.readUInt32();
    if (addr >= 0x00010000U)
      throw Plus4Emu::Exception("invalid start address in plus4 program data");
    if (len >= 0x00010000U ||
        size_t(len) != (buf.getDataSize() - buf.getPosition()))
      throw Plus4Emu::Exception("invalid plus4 program length");
#if 0
    writeMemory(0x002B, uint8_t(addr & 0xFF));
    writeMemory(0x002C, uint8_t((addr >> 8) & 0xFF));
#endif
    while (len) {
      writeMemory(uint16_t(addr), buf.readByte());
      addr = (addr + 1) & 0xFFFF;
      len--;
    }
    writeMemory(0x002D, uint8_t(addr & 0xFF));
    writeMemory(0x002E, uint8_t((addr >> 8) & 0xFF));
    writeMemory(0x002F, uint8_t(addr & 0xFF));
    writeMemory(0x0030, uint8_t((addr >> 8) & 0xFF));
    writeMemory(0x0031, uint8_t(addr & 0xFF));
    writeMemory(0x0032, uint8_t((addr >> 8) & 0xFF));
    writeMemory(0x0033, readMemoryCPU(0x0037));
    writeMemory(0x0034, readMemoryCPU(0x0038));
#if 0
    writeMemory(0x0035, readMemoryCPU(0x0037));
    writeMemory(0x0036, readMemoryCPU(0x0038));
#endif
    writeMemory(0x009D, uint8_t(addr & 0xFF));
    writeMemory(0x009E, uint8_t((addr >> 8) & 0xFF));
  }

  uint16_t TED7360::readPRGFileHeader(std::FILE*& f, const char *fileName)
  {
    if (fileName == (char *) 0 || fileName[0] == '\0') {
      if (f) {
        std::fclose(f);
        f = (std::FILE *) 0;
      }
      throw Plus4Emu::Exception("invalid plus4 program file name");
    }
    if (!f) {
      f = Plus4Emu::fileOpen(fileName, "rb");
      if (!f)
        throw Plus4Emu::Exception("error opening plus4 program file");
    }
    uint16_t  addr = 0x0000;
    int       c = std::fgetc(f);
    bool      eofFlag = (c == EOF);
    if (!eofFlag) {
      addr = uint16_t(c & 0xFF);
      c = std::fgetc(f);
      eofFlag = (c == EOF);
      if (!eofFlag) {
        addr |= uint16_t((c & 0xFF) << 8);
        // check for P00 format
        if (addr == 0x3643) {           // "C6"
          if (Plus4Emu::checkFileNameExtension(fileName, ".p00")) {
            static const char *p00HeaderMagic = "C64File";
            int     i = 2;
            do {
              c = std::fgetc(f);
              eofFlag = (c == EOF);
            } while (!eofFlag && c == p00HeaderMagic[i] && ++i < 8);
            if (i == 8) {
              for ( ; i < 26 && !eofFlag; i++) {
                c = std::fgetc(f);
                eofFlag = (c == EOF);
              }
              if (!eofFlag) {
                c = std::fgetc(f);
                eofFlag = (c == EOF);
                if (!eofFlag) {
                  addr = uint16_t(c & 0xFF);
                  c = std::fgetc(f);
                  eofFlag = (c == EOF);
                  if (!eofFlag)
                    addr |= uint16_t((c & 0xFF) << 8);
                }
              }
            }
          }
        }
      }
    }
    if (eofFlag) {
      std::fclose(f);
      f = (std::FILE *) 0;
      throw Plus4Emu::Exception("unexpected end of plus4 program file");
    }
    return addr;
  }

  void TED7360::loadProgram(const char *fileName)
  {
    std::FILE *f = (std::FILE *) 0;
    uint16_t  addr = readPRGFileHeader(f, fileName);
#if 0
    writeMemory(0x002B, uint8_t(addr & 0xFF));
    writeMemory(0x002C, uint8_t((addr >> 8) & 0xFF));
#endif
    size_t  len = 0;
    while (true) {
      int     c = std::fgetc(f);
      if (c == EOF)
        break;
      if (++len > 0xFFFF) {
        std::fclose(f);
        throw Plus4Emu::Exception("plus4 program file has invalid length");
      }
      writeMemory(addr, uint8_t(c));
      addr = (addr + 1) & 0xFFFF;
    }
    std::fclose(f);
    writeMemory(0x002D, uint8_t(addr & 0xFF));
    writeMemory(0x002E, uint8_t((addr >> 8) & 0xFF));
    writeMemory(0x002F, uint8_t(addr & 0xFF));
    writeMemory(0x0030, uint8_t((addr >> 8) & 0xFF));
    writeMemory(0x0031, uint8_t(addr & 0xFF));
    writeMemory(0x0032, uint8_t((addr >> 8) & 0xFF));
    writeMemory(0x0033, readMemoryCPU(0x0037));
    writeMemory(0x0034, readMemoryCPU(0x0038));
#if 0
    writeMemory(0x0035, readMemoryCPU(0x0037));
    writeMemory(0x0036, readMemoryCPU(0x0038));
#endif
    writeMemory(0x009D, uint8_t(addr & 0xFF));
    writeMemory(0x009E, uint8_t((addr >> 8) & 0xFF));
  }

  void TED7360::registerChunkTypes(Plus4Emu::File& f)
  {
    ChunkType_TED7360Snapshot *p1 = (ChunkType_TED7360Snapshot *) 0;
    ChunkType_Plus4Program    *p2 = (ChunkType_Plus4Program *) 0;

    try {
      p1 = new ChunkType_TED7360Snapshot(*this);
      f.registerChunkType(p1);
      p1 = (ChunkType_TED7360Snapshot *) 0;
      p2 = new ChunkType_Plus4Program(*this);
      f.registerChunkType(p2);
      p2 = (ChunkType_Plus4Program *) 0;
    }
    catch (...) {
      if (p2)
        delete p2;
      if (p1)
        delete p1;
      throw;
    }
    M7501::registerChunkType(f);
  }

}       // namespace Plus4

