
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
#include "gui.hpp"
#include "disasm.hpp"
#include "debugger.hpp"

Plus4EmuGUI_LuaScript::~Plus4EmuGUI_LuaScript()
{
}

void Plus4EmuGUI_LuaScript::errorCallback(const char *msg)
{
  if (msg == (char *) 0 || msg[0] == '\0')
    msg = "Lua script error";
  debugWindow.gui.errorMessage(msg);
}

void Plus4EmuGUI_LuaScript::messageCallback(const char *msg)
{
  if (!msg)
    msg = "";
  debugWindow.monitor_->printMessage(msg);
}

Plus4EmuGUI_ScrollableOutput::~Plus4EmuGUI_ScrollableOutput()
{
}

int Plus4EmuGUI_ScrollableOutput::handle(int evt)
{
  if (evt == FL_MOUSEWHEEL) {
    int     tmp = Fl::event_dy();
    if (tmp > 0) {
      if (downWidget)
        downWidget->do_callback();
    }
    else if (tmp < 0) {
      if (upWidget)
        upWidget->do_callback();
    }
    return 1;
  }
  return Fl_Multiline_Output::handle(evt);
}

// ----------------------------------------------------------------------------

Plus4EmuGUI_DebugWindow::Plus4EmuGUI_DebugWindow(Plus4EmuGUI& gui_)
  : gui(gui_),
    luaScript(*this, gui_.vm)
{
  for (size_t i = 0; i < sizeof(windowTitle); i++)
    windowTitle[i] = '\0';
  std::strcpy(&(windowTitle[0]), "plus4emu debugger");
  savedWindowPositionX = 32;
  savedWindowPositionY = 32;
  focusWidget = (Fl_Widget *) 0;
  prvTab = (Fl_Widget *) 0;
  memoryDumpStartAddress = 0x00000000U;
  memoryDumpEndAddress = 0x0000007FU;
  memoryDumpViewAddress = 0x00000000U;
  memoryDumpAddrDec = 0x003FFFC0U;
  memoryDumpAddrInc = 0x00000040U;
  memoryDumpCPUAddressMode = true;
  memoryDumpASCIIFileFormat = false;
  disassemblyStartAddress = 0x00000000U;
  disassemblyViewAddress = 0x00000000U;
  disassemblyNextAddress = 0x00000000U;
  for (int i = 0; i < 6; i++)
    breakPointLists[i] = "";
  tmpBuffer.reserve(896);
  bpEditBuffer = new Fl_Text_Buffer();
  scriptEditBuffer = new Fl_Text_Buffer();
  createDebugWindow();
  window->label(&(windowTitle[0]));
  memoryDumpDisplay->upWidget = memoryDumpPrvPageButton;
  memoryDumpDisplay->downWidget = memoryDumpNxtPageButton;
  disassemblyDisplay->upWidget = disassemblyPrvPageButton;
  disassemblyDisplay->downWidget = disassemblyNxtPageButton;
  for (int i = 0; i < 32; i++)
    prvTEDRegisterState[i] = 0x00;
}

Plus4EmuGUI_DebugWindow::~Plus4EmuGUI_DebugWindow()
{
  delete window;
  delete bpEditBuffer;
  delete scriptEditBuffer;
}

void Plus4EmuGUI_DebugWindow::show()
{
  this->activate();
  monitor_->closeTraceFile();
  updateWindow();
  if (!window->shown()) {
    window->resize(savedWindowPositionX, savedWindowPositionY, 960, 720);
    if (focusWidget != (Fl_Widget *) 0 && focusWidget != monitor_)
      focusWidget->take_focus();
    else
      stepButton->take_focus();
  }
  window->label(&(windowTitle[0]));
  window->show();
}

bool Plus4EmuGUI_DebugWindow::shown() const
{
  return bool(window->shown());
}

void Plus4EmuGUI_DebugWindow::hide()
{
  this->deactivate(1000000.0);
  if (window->shown()) {
    savedWindowPositionX = window->x();
    savedWindowPositionY = window->y();
  }
  window->hide();
  std::strcpy(&(windowTitle[0]), "plus4emu debugger");
}

void Plus4EmuGUI_DebugWindow::activate()
{
  Fl::remove_timeout(&hideWindowCallback, (void *) this);
  debugTabs->clear_output();
  mainTab->clear_output();
  monitorTab->clear_output();
  stepIntoButton->clear_output();
  returnButton->clear_output();
  stepToButton->clear_output();
  stepToAddressValuator->clear_output();
  stepOverButton->clear_output();
  stepButton->clear_output();
  continueButton->clear_output();
}

bool Plus4EmuGUI_DebugWindow::active() const
{
  if (!window->shown())
    return false;
  return (!continueButton->output());
}

void Plus4EmuGUI_DebugWindow::deactivate(double tt)
{
  Fl::remove_timeout(&hideWindowCallback, (void *) this);
  if (tt <= 0.0) {
    this->hide();
    return;
  }
  mainTab->set_output();
  monitorTab->set_output();
  debugTabs->set_output();
  stepIntoButton->set_output();
  returnButton->set_output();
  stepToButton->set_output();
  stepToAddressValuator->set_output();
  stepOverButton->set_output();
  stepButton->set_output();
  continueButton->set_output();
  if (gui.debugWindowOpenFlag) {
    gui.debugWindowOpenFlag = false;
    gui.unlockVMThread();
  }
  if (tt < 1000.0)
    Fl::add_timeout(tt, &hideWindowCallback, (void *) this);
}

bool Plus4EmuGUI_DebugWindow::breakPoint(int debugContext_,
                                         int type, uint16_t addr, uint8_t value)
{
  if ((type == 0 || type == 3) && monitor_->getIsTraceOn()) {
    monitor_->writeTraceFile(debugContext_, addr);
    if (type == 3)
      return false;
  }
  setDebugContext(debugContext_);
  debugContextValuator->value(debugContext_);
  switch (type) {
  case 0:
  case 3:
    try {
      gui.vm.disassembleInstruction(tmpBuffer, addr, true);
      if (tmpBuffer.length() > 21 && tmpBuffer.length() <= 40) {
        std::sprintf(&(windowTitle[0]), "Break at PC=%04X: %s",
                     (unsigned int) (addr & 0xFFFF), (tmpBuffer.c_str() + 21));
        tmpBuffer.clear();
        break;
      }
    }
    catch (...) {
    }
    tmpBuffer.clear();
  case 1:
    std::sprintf(&(windowTitle[0]),
                 "Break on reading %02X from memory address %04X",
                 (unsigned int) (value & 0xFF), (unsigned int) (addr & 0xFFFF));
    break;
  case 2:
    std::sprintf(&(windowTitle[0]),
                 "Break on writing %02X to memory address %04X",
                 (unsigned int) (value & 0xFF), (unsigned int) (addr & 0xFFFF));
    break;
  case 4:
    std::sprintf(&(windowTitle[0]),
                 "Break at TED video position %04X:%02X",
                 (unsigned int) ((addr >> 7) & 0x01FF),
                 (unsigned int) ((addr & 0x7F) << 1));
    break;
  default:
    std::sprintf(&(windowTitle[0]), "Break");
  }
  disassemblyViewAddress = uint32_t(gui.vm.getProgramCounter() & 0xFFFF);
  if (focusWidget == monitor_)
    monitor_->breakMessage(&(windowTitle[0]));
  return true;
}

void Plus4EmuGUI_DebugWindow::updateWindow()
{
  try {
    gui.vm.listCPURegisters(tmpBuffer);
    cpuRegisterDisplay->value(tmpBuffer.c_str());
    tmpBuffer.clear();
    {
      char    tmpBuf[64];
      char    *s = &(tmpBuf[0]);
      std::sprintf(s, "0000-3FFF: %02X\n4000-7FFF: %02X\n"
                      "8000-BFFF: %02X\nC000-FFFF: %02X",
                   (unsigned int) gui.vm.getMemoryPage(0),
                   (unsigned int) gui.vm.getMemoryPage(1),
                   (unsigned int) gui.vm.getMemoryPage(2),
                   (unsigned int) gui.vm.getMemoryPage(3));
      memoryPagingDisplay->value(s);
      dumpMemory(tmpBuffer, 0x0010FF00U, 0x0010FF1FU, 0x0010FFFFU, true, false);
      tmpBuffer[0] = ' ';
      tmpBuffer[1] = ' ';
      tmpBuffer[41] = ' ';
      tmpBuffer[42] = ' ';
      tmpBuffer[82] = ' ';
      tmpBuffer[83] = ' ';
      tmpBuffer[123] = ' ';
      tmpBuffer[124] = ' ';
      for (int i = 0; i < 32; i++) {
        uint8_t tmp = gui.vm.readMemory(0x0010FF00U | (unsigned int) i, false);
        if (tmp != prvTEDRegisterState[i]) {
          if (prvTEDRegisterState[0x1F] != 0x00) {
            tmpBuffer[((i >> 3) * 41) + (((i & 4) >> 2) * 17) + ((i & 3) << 2)
                      + 8] = '*';
          }
          prvTEDRegisterState[i] = tmp;
        }
      }
      int     xPos = 0;
      int     yPos = 0;
      gui.vm.getVideoPosition(xPos, yPos);
      std::sprintf(s, "\n\n  Horizontal position:  %04X\n"
                      "  Vertical position:    %04X",
                   (unsigned int) xPos, (unsigned int) yPos);
      tmpBuffer += s;
      tedRegisterDisplay->value(tmpBuffer.c_str());
      tmpBuffer.clear();
    }
    uint32_t  tmp = gui.vm.getStackPointer();
    uint32_t  startAddr = (((tmp - 1U) & 0xFFU) + 0x00F5U) & 0xFFF8U;
    uint32_t  endAddr = startAddr + 0x002FU;
    dumpMemory(tmpBuffer, startAddr, endAddr, tmp, true, true);
    stackMemoryDumpDisplay->value(tmpBuffer.c_str());
    tmpBuffer.clear();
    updateMemoryDumpDisplay();
    updateDisassemblyDisplay();
    bpPriorityThresholdValuator->value(
        double(gui.config.debug.bpPriorityThreshold));
    breakOnInvalidOpcodeValuator->value(
        gui.config.debug.breakOnInvalidOpcode ? 1 : 0);
  }
  catch (std::exception& e) {
    gui.errorMessage(e.what());
  }
}

char * Plus4EmuGUI_DebugWindow::printHexNumber(char *bufp, uint32_t n,
                                               size_t spaceCnt1, size_t nDigits,
                                               size_t spaceCnt2)
{
  char    *s = bufp + (spaceCnt1 + nDigits);
  bufp = s;
  while (spaceCnt2-- > 0)
    *(bufp++) = ' ';
  *bufp = '\0';
  while (nDigits-- > 0) {
    char    c = char(n & 0x0FU);
    *(--s) = c + (c < char(10) ? '0' : ('A' - char(10)));
    n = n >> 4;
  }
  while (spaceCnt1-- > 0) {
    if (n > 0U) {
      char    c = char(n & 0x0FU);
      *(--s) = c + (c < char(10) ? '0' : ('A' - char(10)));
      n = n >> 4;
    }
    else {
      *(--s) = ' ';
    }
  }
  return bufp;
}

void Plus4EmuGUI_DebugWindow::dumpMemory(std::string& buf,
                                         uint32_t startAddr, uint32_t endAddr,
                                         uint32_t cursorAddr, bool showCursor,
                                         bool isCPUAddress)
{
  try {
    char      tmpBuf[48];
    uint8_t   tmpBuf2[8];
    buf.clear();
    uint32_t  addrMask = uint32_t(isCPUAddress ? 0x0000FFFFU : 0x003FFFFFU);
    int       cnt;
    startAddr &= addrMask;
    endAddr &= addrMask;
    cursorAddr &= addrMask;
    do {
      char    *bufp =
          printHexNumber(&(tmpBuf[0]), startAddr, (isCPUAddress ? 2 : 0),
                         (isCPUAddress ? 4 : 6), (showCursor ? 0 : 1));
      cnt = 0;
      while (true) {
        tmpBuf2[cnt] = gui.vm.readMemory(startAddr, isCPUAddress);
        if (showCursor) {
          bufp = printHexNumber(bufp, tmpBuf2[cnt], ((cnt & 3) == 0 ? 3 : 2),
                                2, 0);
          if (startAddr == cursorAddr)
            *(bufp - 3) = '*';
        }
        else {
          bufp = printHexNumber(bufp, tmpBuf2[cnt], 1, 2, 0);
          if (cnt >= 7) {
            bufp[0] = ' ';
            bufp[1] = ':';
            for (int i = 2; i < 10; i++) {
              bufp[i] = char(tmpBuf2[i - 2] & 0x7F);
              if (bufp[i] < char(0x20) || bufp[i] == char(0x7F))
                bufp[i] = '.';
            }
            bufp[10] = '\0';
            bufp = bufp + 10;
          }
        }
        if (startAddr == endAddr)
          break;
        startAddr = (startAddr + 1U) & addrMask;
        if (++cnt >= 8) {
          bufp[0] = '\n';
          bufp[1] = '\0';
          break;
        }
      }
      buf += &(tmpBuf[0]);
    } while (cnt >= 8);
  }
  catch (std::exception& e) {
    buf.clear();
    gui.errorMessage(e.what());
  }
}

void Plus4EmuGUI_DebugWindow::updateMemoryDumpDisplay()
{
  try {
    uint32_t  addrMask =
        uint32_t(memoryDumpCPUAddressMode ? 0x00FFFFU : 0x3FFFFFU);
    memoryDumpStartAddress &= addrMask;
    memoryDumpEndAddress &= addrMask;
    memoryDumpViewAddress &= addrMask;
    const char  *fmt = (memoryDumpCPUAddressMode ? "%04X" : "%06X");
    char  tmpBuf[8];
    std::sprintf(&(tmpBuf[0]), fmt, (unsigned int) memoryDumpStartAddress);
    memoryDumpStartAddressValuator->value(&(tmpBuf[0]));
    std::sprintf(&(tmpBuf[0]), fmt, (unsigned int) memoryDumpEndAddress);
    memoryDumpEndAddressValuator->value(&(tmpBuf[0]));
    dumpMemory(tmpBuffer, memoryDumpViewAddress, memoryDumpViewAddress + 0x87U,
               0U, false, memoryDumpCPUAddressMode);
    memoryDumpDisplay->value(tmpBuffer.c_str());
    tmpBuffer.clear();
  }
  catch (std::exception& e) {
    gui.errorMessage(e.what());
  }
}

long Plus4EmuGUI_DebugWindow::parseHexNumber(uint32_t& value, const char *s)
{
  long  cnt = 0L;
  if (!s)
    return 0L;
  while (*s == ' ' || *s == '\t') {
    s++;
    cnt++;
  }
  if (*s == '\0')
    return 0L;
  if (*s == '\r' || *s == '\n')
    return (-(cnt + 1L));
  uint32_t  tmpVal = 0U;
  while (true) {
    char  c = *s;
    if (c == ' ' || c == '\t' || c == '\r' || c == '\n' || c == '\0') {
      value = tmpVal;
      return cnt;
    }
    tmpVal = (tmpVal << 4) & 0xFFFFFFFFU;
    if (c >= '0' && c <= '9')
      tmpVal += uint32_t(c - '0');
    else if (c >= 'A' && c <= 'F')
      tmpVal += uint32_t((c - 'A') + 10);
    else if (c >= 'a' && c <= 'f')
      tmpVal += uint32_t((c - 'a') + 10);
    else {
      gui.errorMessage("invalid number format");
      return 0L;
    }
    s++;
    cnt++;
  }
  return 0L;
}

void Plus4EmuGUI_DebugWindow::parseMemoryDump(const char *s)
{
  uint32_t  addr = 0U;
  bool      haveAddress = false;
  try {
    while (true) {
      uint32_t  tmp = 0U;
      long      n = parseHexNumber(tmp, s);
      if (!n)           // end of string or error
        break;
      if (n < 0L) {     // end of line
        n = (-n);
        haveAddress = false;
        s = s + n;
      }
      else {
        s = s + n;
        if (!haveAddress) {
          addr = tmp & 0x3FFFFFU;
          haveAddress = true;
        }
        else {
          gui.vm.writeMemory(addr, uint8_t(tmp & 0xFFU),
                             memoryDumpCPUAddressMode);
          addr++;
        }
      }
    }
  }
  catch (std::exception& e) {
    gui.errorMessage(e.what());
  }
}

void Plus4EmuGUI_DebugWindow::updateDisassemblyDisplay()
{
  try {
    disassemblyStartAddress &= 0xFFFFU;
    disassemblyViewAddress &= 0xFFFFU;
    disassemblyNextAddress &= 0xFFFFU;
    char  tmpBuf[8];
    std::sprintf(&(tmpBuf[0]), "%04X", (unsigned int) disassemblyStartAddress);
    disassemblyStartAddressValuator->value(&(tmpBuf[0]));
    std::string tmp;
    tmp.reserve(48);
    tmpBuffer.clear();
    uint32_t  addr = disassemblySearchBack(2);
    uint32_t  pcAddr = uint32_t(gui.vm.getProgramCounter()) & 0xFFFFU;
    for (int i = 0; i < 23; i++) {
      if (i == 22)
        disassemblyNextAddress = addr;
      uint32_t  nxtAddr = gui.vm.disassembleInstruction(tmp, addr, true, 0);
      while (addr != nxtAddr) {
        if (addr == pcAddr)
          tmp[1] = '*';
        addr = (addr + 1U) & 0xFFFFU;
      }
      tmpBuffer += tmp;
      if (i != 22)
        tmpBuffer += '\n';
    }
    disassemblyDisplay->value(tmpBuffer.c_str());
    tmpBuffer.clear();
  }
  catch (std::exception& e) {
    gui.errorMessage(e.what());
  }
}

uint32_t Plus4EmuGUI_DebugWindow::disassemblySearchBack(int insnCnt)
{
  uint32_t    addrTable[256];
  if (insnCnt > 64)
    insnCnt = 64;
  for (uint32_t offs = 21U; true; offs--) {
    int       addrCnt = 0;
    uint32_t  addr = disassemblyViewAddress - (uint32_t(insnCnt * 3) + offs);
    bool      doneFlag = false;
    addr = addr & 0xFFFFU;
    addrTable[addrCnt++] = addr;
    do {
      uint32_t  nxtAddr =
          Plus4::M7501Disassembler::getNextInstructionAddr(gui.vm, addr, true);
      while (addr != nxtAddr) {
        if (addr == disassemblyViewAddress)
          doneFlag = true;
        addr = (addr + 1U) & 0xFFFFU;
      }
      addrTable[addrCnt++] = addr;
    } while (!doneFlag);
    for (int i = 0; i < addrCnt; i++) {
      if (addrTable[i] == disassemblyViewAddress ||
          (offs == 3U && i == (addrCnt - 1))) {
        if (i >= insnCnt)
          return addrTable[i - insnCnt];
      }
    }
  }
  return disassemblyViewAddress;
}

void Plus4EmuGUI_DebugWindow::applyBreakPointList()
{
  const char  *buf = (char *) 0;
  try {
    buf = bpEditBuffer->text();
    if (!buf)
      throw std::bad_alloc();
    std::string bpListText(buf);
    std::free(const_cast<char *>(buf));
    buf = (char *) 0;
    Plus4Emu::BreakPointList  bpList(bpListText);
    gui.vm.clearBreakPoints();
    gui.vm.setBreakPoints(bpList);
  }
  catch (std::exception& e) {
    if (buf)
      std::free(const_cast<char *>(buf));
    gui.errorMessage(e.what());
  }
}

void Plus4EmuGUI_DebugWindow::setDebugContext(int n)
{
  n = (n >= 0 ? (n <= 5 ? n : 5) : 0);
  int   tmp = gui.vm.getDebugContext();
  if (n != tmp) {
    const char  *s = (char *) 0;
    try {
      s = bpEditBuffer->text();
      if (s) {
        breakPointLists[tmp] = s;
        std::free(const_cast<char *>(s));
        s = (char *) 0;
      }
      else
        breakPointLists[tmp] = "";
      gui.vm.setDebugContext(n);
      bpEditBuffer->text(breakPointLists[n].c_str());
    }
    catch (std::exception& e) {
      if (s)
        std::free(const_cast<char *>(s));
      gui.errorMessage(e.what());
    }
  }
}

void Plus4EmuGUI_DebugWindow::breakPointCallback(void *userData,
                                                 int debugContext_, int type,
                                                 uint16_t addr, uint8_t value)
{
  Plus4EmuGUI_DebugWindow&  debugWindow =
      *(reinterpret_cast<Plus4EmuGUI_DebugWindow *>(userData));
  if (!debugWindow.luaScript.runBreakPointCallback(debugContext_,
                                                   type, addr, value)) {
    return;
  }
  Plus4EmuGUI&  gui_ = debugWindow.gui;
  Fl::lock();
  if (gui_.exitFlag || !gui_.mainWindow->shown()) {
    Fl::unlock();
    return;
  }
  if (!debugWindow.breakPoint(debugContext_, type, addr, value)) {
    Fl::unlock();
    return;             // do not show debugger window if tracing
  }
  if (!debugWindow.active()) {
    gui_.debugWindowShowFlag = true;
    Fl::awake();
  }
  while (gui_.debugWindowShowFlag) {
    Fl::unlock();
    gui_.updateDisplay();
    Fl::lock();
  }
  while (true) {
    bool  tmp = debugWindow.active();
    Fl::unlock();
    if (!tmp)
      break;
    gui_.updateDisplay();
    Fl::lock();
  }
}

void Plus4EmuGUI_DebugWindow::hideWindowCallback(void *userData)
{
  reinterpret_cast< Plus4EmuGUI_DebugWindow * >(userData)->hide();
}

