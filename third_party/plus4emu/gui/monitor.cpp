
// plus4emu -- portable Commodore Plus/4 emulator
// Copyright (C) 2003-2016 Istvan Varga <istvanv@users.sourceforge.net>
// http://sourceforge.net/projects/plus4emu/
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

#include "gui.hpp"
#include "monitor.hpp"
#include "ted.hpp"

#include <vector>

#define MONITOR_MAX_LINES   (160)

static const char *fileOpenErrorMessages[6] = {
  "Error opening file",
  "Invalid file name",
  "File not found",
  "File is not a regular file",
  "Permission denied",
  "File already exists"
};

// ----------------------------------------------------------------------------

void Plus4EmuGUIMonitor::tokenizeString(std::vector<std::string>& args,
                                        const char *s)
{
  args.resize(0);
  if (!s)
    return;
  std::string curToken = "";
  int         mode = 0;         // 0: skipping space, 1: token, 2: string
  while (*s != '\0') {
    if (mode == 0) {
      if (*s == ' ' || *s == '\t' || *s == '\r' || *s == '\n') {
        s++;
        continue;
      }
      mode = (*s != '"' ? 1 : 2);
    }
    if (mode == 1) {
      if (args.size() == 0 && curToken.length() > 0) {
        // allow no space between command and first hexadecimal argument
        if ((*s >= '0' && *s <= '9') || *s == '%' ||
            (*s >= 'A' && *s <= 'F') || (*s >= 'a' && *s <= 'f')) {
          args.push_back(curToken);
          curToken = "";
          continue;
        }
      }
      if ((*s >= 'A' && *s <= 'Z') || (*s >= '0' && *s <= '9') ||
          *s == '_' || (*s == '%' && curToken.empty()) || *s == '?') {
        curToken += (*s);
        s++;
        continue;
      }
      else if (*s >= 'a' && *s <= 'z') {
        // convert to upper case
        curToken += ((*s - 'a') + 'A');
        s++;
        continue;
      }
      else {
        if (curToken.length() > 0) {
          args.push_back(curToken);
          curToken = "";
        }
        if (*s == ' ' || *s == '\t' || *s == '\r' || *s == '\n') {
          mode = 0;
          s++;
          continue;
        }
        if (*s != '"') {
          curToken = (*s);
          args.push_back(curToken);
          curToken = "";
          mode = 0;
          s++;
          continue;
        }
        mode = 2;
      }
    }
    if (*s == '"' && curToken.length() > 0) {
      // closing quote character is not stored
      args.push_back(curToken);
      curToken = "";
      mode = 0;
    }
    else {
      curToken += (*s);
    }
    s++;
  }
  if (curToken.length() > 0)
    args.push_back(curToken);
}

static bool parseHexNumber(uint32_t& n, const char *s)
{
  n = 0U;
  if (!s)
    return false;
  while ((*s) == ' ' || (*s) == '\t' || (*s) == '\r' || (*s) == '\n')
    s++;
  uint32_t  k = 16U;            // assume hexadecimal format by default
  if ((*s) == '%') {
    k = 2U;
    s++;
  }
  else if ((*s) == 'O' || (*s) == 'o') {
    k = 8U;
    s++;
  }
  size_t  len = 0;
  while ((s[len] >= '0' && s[len] <= '9') ||
         (s[len] >= 'A' && s[len] <= 'F') ||
         (s[len] >= 'a' && s[len] <= 'f')) {
    len++;
  }
  if (len < 1)
    return false;
  if (s[len] != '\0') {
    const char  *t = &(s[len]);
    if (k == 16U) {
      if ((*t) == 'O' || (*t) == 'o') {
        k = 8U;
        t++;
      }
      else if ((*t) == 'L' || (*t) == 'l') {
        k = 10U;
        t++;
      }
      else if ((*t) == 'H' || (*t) == 'h') {
        t++;
      }
    }
    while ((*t) == ' ' || (*t) == '\t' || (*t) == '\r' || (*t) == '\n')
      t++;
    if ((*t) != '\0')
      return false;
  }
  for (size_t i = 0; i < len; i++) {
    uint32_t  tmp =
        uint32_t(s[i] - (s[i] <= '9' ? '0' : (s[i] <= 'Z' ? '7' : 'W')));
    if (tmp >= k)
      return false;
    n = (n * k) + tmp;
  }
  return true;
}

static uint32_t parseHexNumberEx(const char *s, uint32_t mask_ = 0xFFFFFFFFU)
{
  uint32_t  n = 0U;
  if (!parseHexNumber(n, s))
    throw Plus4Emu::Exception("invalid number format");
  return (n & mask_);
}

// ----------------------------------------------------------------------------

struct AssemblerOpcodeTableEntry {
  const char  *name;
  uint8_t     opcode;
};

// A: implied
// B: immediate
// C: zeropage
// D: zeropage, X
// E: zeropage, Y
// F: absolute / relative
// G: absolute, X
// H: absolute, Y
// I: (indirect)
// J: (indirect, X)
// K: (indirect), Y

static const AssemblerOpcodeTableEntry  assemblerOpcodeTable[221] = {
  { "???A", 0x02 },   { "ADCB", 0x69 },   { "ADCC", 0x65 },   { "ADCD", 0x75 },
  { "ADCF", 0x6D },   { "ADCG", 0x7D },   { "ADCH", 0x79 },   { "ADCJ", 0x61 },
  { "ADCK", 0x71 },   { "ANCB", 0x0B },   { "ANDB", 0x29 },   { "ANDC", 0x25 },
  { "ANDD", 0x35 },   { "ANDF", 0x2D },   { "ANDG", 0x3D },   { "ANDH", 0x39 },
  { "ANDJ", 0x21 },   { "ANDK", 0x31 },   { "ANEB", 0x8B },   { "ARRB", 0x6B },
  { "ASLA", 0x0A },   { "ASLC", 0x06 },   { "ASLD", 0x16 },   { "ASLF", 0x0E },
  { "ASLG", 0x1E },   { "ASRB", 0x4B },   { "BCCF", 0x90 },   { "BCSF", 0xB0 },
  { "BEQF", 0xF0 },   { "BITC", 0x24 },   { "BITF", 0x2C },   { "BMIF", 0x30 },
  { "BNEF", 0xD0 },   { "BPLF", 0x10 },   { "BRKA", 0x00 },   { "BVCF", 0x50 },
  { "BVSF", 0x70 },   { "CLCA", 0x18 },   { "CLDA", 0xD8 },   { "CLIA", 0x58 },
  { "CLVA", 0xB8 },   { "CMPB", 0xC9 },   { "CMPC", 0xC5 },   { "CMPD", 0xD5 },
  { "CMPF", 0xCD },   { "CMPG", 0xDD },   { "CMPH", 0xD9 },   { "CMPJ", 0xC1 },
  { "CMPK", 0xD1 },   { "CPXB", 0xE0 },   { "CPXC", 0xE4 },   { "CPXF", 0xEC },
  { "CPYB", 0xC0 },   { "CPYC", 0xC4 },   { "CPYF", 0xCC },   { "DCPC", 0xC7 },
  { "DCPD", 0xD7 },   { "DCPF", 0xCF },   { "DCPG", 0xDF },   { "DCPH", 0xDB },
  { "DCPJ", 0xC3 },   { "DCPK", 0xD3 },   { "DECC", 0xC6 },   { "DECD", 0xD6 },
  { "DECF", 0xCE },   { "DECG", 0xDE },   { "DEXA", 0xCA },   { "DEYA", 0x88 },
  { "EORB", 0x49 },   { "EORC", 0x45 },   { "EORD", 0x55 },   { "EORF", 0x4D },
  { "EORG", 0x5D },   { "EORH", 0x59 },   { "EORJ", 0x41 },   { "EORK", 0x51 },
  { "INCC", 0xE6 },   { "INCD", 0xF6 },   { "INCF", 0xEE },   { "INCG", 0xFE },
  { "INXA", 0xE8 },   { "INYA", 0xC8 },   { "ISBC", 0xE7 },   { "ISBD", 0xF7 },
  { "ISBF", 0xEF },   { "ISBG", 0xFF },   { "ISBH", 0xFB },   { "ISBJ", 0xE3 },
  { "ISBK", 0xF3 },   { "JMPF", 0x4C },   { "JMPI", 0x6C },   { "JSRF", 0x20 },
  { "LASH", 0xBB },   { "LAXC", 0xA7 },   { "LAXE", 0xB7 },   { "LAXF", 0xAF },
  { "LAXH", 0xBF },   { "LAXJ", 0xA3 },   { "LAXK", 0xB3 },   { "LDAB", 0xA9 },
  { "LDAC", 0xA5 },   { "LDAD", 0xB5 },   { "LDAF", 0xAD },   { "LDAG", 0xBD },
  { "LDAH", 0xB9 },   { "LDAJ", 0xA1 },   { "LDAK", 0xB1 },   { "LDXB", 0xA2 },
  { "LDXC", 0xA6 },   { "LDXE", 0xB6 },   { "LDXF", 0xAE },   { "LDXH", 0xBE },
  { "LDYB", 0xA0 },   { "LDYC", 0xA4 },   { "LDYD", 0xB4 },   { "LDYF", 0xAC },
  { "LDYG", 0xBC },   { "LSRA", 0x4A },   { "LSRC", 0x46 },   { "LSRD", 0x56 },
  { "LSRF", 0x4E },   { "LSRG", 0x5E },   { "LXAB", 0xAB },   { "NOPA", 0xEA },
  { "NOPB", 0x89 },   { "NOPC", 0x04 },   { "NOPD", 0x14 },   { "NOPF", 0x0C },
  { "NOPG", 0x1C },   { "ORAB", 0x09 },   { "ORAC", 0x05 },   { "ORAD", 0x15 },
  { "ORAF", 0x0D },   { "ORAG", 0x1D },   { "ORAH", 0x19 },   { "ORAJ", 0x01 },
  { "ORAK", 0x11 },   { "PHAA", 0x48 },   { "PHPA", 0x08 },   { "PLAA", 0x68 },
  { "PLPA", 0x28 },   { "RLAC", 0x27 },   { "RLAD", 0x37 },   { "RLAF", 0x2F },
  { "RLAG", 0x3F },   { "RLAH", 0x3B },   { "RLAJ", 0x23 },   { "RLAK", 0x33 },
  { "ROLA", 0x2A },   { "ROLC", 0x26 },   { "ROLD", 0x36 },   { "ROLF", 0x2E },
  { "ROLG", 0x3E },   { "RORA", 0x6A },   { "RORC", 0x66 },   { "RORD", 0x76 },
  { "RORF", 0x6E },   { "RORG", 0x7E },   { "RRAC", 0x67 },   { "RRAD", 0x77 },
  { "RRAF", 0x6F },   { "RRAG", 0x7F },   { "RRAH", 0x7B },   { "RRAJ", 0x63 },
  { "RRAK", 0x73 },   { "RTIA", 0x40 },   { "RTSA", 0x60 },   { "SAXC", 0x87 },
  { "SAXE", 0x97 },   { "SAXF", 0x8F },   { "SAXJ", 0x83 },   { "SBCB", 0xE9 },
  { "SBCC", 0xE5 },   { "SBCD", 0xF5 },   { "SBCF", 0xED },   { "SBCG", 0xFD },
  { "SBCH", 0xF9 },   { "SBCJ", 0xE1 },   { "SBCK", 0xF1 },   { "SBXB", 0xCB },
  { "SECA", 0x38 },   { "SEDA", 0xF8 },   { "SEIA", 0x78 },   { "SHAH", 0x9F },
  { "SHAK", 0x93 },   { "SHSH", 0x9B },   { "SHXH", 0x9E },   { "SHYG", 0x9C },
  { "SLOC", 0x07 },   { "SLOD", 0x17 },   { "SLOF", 0x0F },   { "SLOG", 0x1F },
  { "SLOH", 0x1B },   { "SLOJ", 0x03 },   { "SLOK", 0x13 },   { "SREC", 0x47 },
  { "SRED", 0x57 },   { "SREF", 0x4F },   { "SREG", 0x5F },   { "SREH", 0x5B },
  { "SREJ", 0x43 },   { "SREK", 0x53 },   { "STAC", 0x85 },   { "STAD", 0x95 },
  { "STAF", 0x8D },   { "STAG", 0x9D },   { "STAH", 0x99 },   { "STAJ", 0x81 },
  { "STAK", 0x91 },   { "STXC", 0x86 },   { "STXE", 0x96 },   { "STXF", 0x8E },
  { "STYC", 0x84 },   { "STYD", 0x94 },   { "STYF", 0x8C },   { "TAXA", 0xAA },
  { "TAYA", 0xA8 },   { "TSXA", 0xBA },   { "TXAA", 0x8A },   { "TXSA", 0x9A },
  { "TYAA", 0x98 }
};

static int searchAssemblerOpcodeTable(const char *s)
{
  size_t  l = 0;
  size_t  h = sizeof(assemblerOpcodeTable) / sizeof(AssemblerOpcodeTableEntry);
  while (h > l) {
    size_t  n = (l + h) >> 1;
    int     d = 0;
    for (int i = 0; i < 5; i++) {
      if (s[i] == '*' && i < 4)
        continue;
      if (s[i] < assemblerOpcodeTable[n].name[i]) {
        d = -1;
        break;
      }
      if (s[i] > assemblerOpcodeTable[n].name[i]) {
        d = 1;
        break;
      }
    }
    if (d == 0)
      return int(n);
    if (d < 0) {
      if (h == n)
        break;
      h = n;
    }
    else {
      if (l == n)
        break;
      l = n;
    }
  }
  return -1;
}

static int assembleInstruction(uint8_t *buf, uint32_t& addr,
                               const std::vector<std::string>& args)
{
  if (args.size() < 3)
    return 0;
  addr = 0U;
  if (!parseHexNumber(addr, args[1].c_str()))
    return 0;
  size_t    nameOffs = 2;
  char      tmpBuf[5];
  tmpBuf[3] = '*';
  tmpBuf[4] = '\0';
  while (nameOffs < args.size()) {
    if (args[nameOffs].length() == 3) {
      tmpBuf[0] = args[nameOffs][0];
      tmpBuf[1] = args[nameOffs][1];
      tmpBuf[2] = args[nameOffs][2];
      if (searchAssemblerOpcodeTable(&(tmpBuf[0])) >= 0)
        break;
    }
    if (args[nameOffs] != "*") {
      uint32_t  tmp = 0U;
      if (!parseHexNumber(tmp, args[nameOffs].c_str()))
        return 0;
    }
    nameOffs++;
  }
  if (nameOffs >= args.size())
    return 0;
  char      addressingMode = 'A';
  uint32_t  operand = 0U;
  size_t    offs = nameOffs + 1;
  if (offs < args.size()) {
    if (args[offs] == "(") {                    // indirect
      offs++;
      if (offs >= args.size())
        return 0;
      if (args[offs] == "$")
        offs++;
      if ((offs + 2) > args.size())
        return 0;
      if (!parseHexNumber(operand, args[offs].c_str()))
        return 0;
      offs++;
      if (args[offs] == ")") {
        if ((offs + 1) == args.size()) {
          addressingMode = 'I';
        }
        else if ((offs + 3) == args.size()) {   // (indirect), Y
          if (args[offs + 1] != ",")
            return 0;
          if (args[offs + 2] != "Y")
            return 0;
          addressingMode = 'K';
        }
        else
          return 0;
      }
      else if (args[offs] == ",") {             // (indirect, X)
        if ((offs + 3) != args.size())
          return 0;
        if (args[offs + 1] != "X")
          return 0;
        if (args[offs + 2] != ")")
          return 0;
        addressingMode = 'J';
      }
    }
    else if (args[offs] == "#") {               // immediate
      offs++;
      if (offs >= args.size())
        return 0;
      if (args[offs] == "$")
        offs++;
      if ((offs + 1) != args.size())
        return 0;
      if (!parseHexNumber(operand, args[offs].c_str()))
        return 0;
      addressingMode = 'B';
    }
    else {                              // zeropage, absolute, or relative
      if (args[offs] == "$")
        offs++;
      if (offs >= args.size())
        return 0;
      if (!parseHexNumber(operand, args[offs].c_str()))
        return 0;
      if (args[offs].length() > 2)
        addressingMode = 'F';
      else
        addressingMode = 'C';
      offs++;
      if (offs != args.size()) {                // indexed
        if ((offs + 2) != args.size())
          return 0;
        if (args[offs] != ",")
          return 0;
        addressingMode++;
        if (args[offs + 1] == "Y")
          addressingMode++;
        else if (args[offs + 1] != "X")
          return 0;
      }
    }
  }
  tmpBuf[3] = addressingMode;
  int     opcode = searchAssemblerOpcodeTable(&(tmpBuf[0]));
  if (opcode < 0) {
    if (addressingMode >= 'C' && addressingMode <= 'E') {
      // if no zeropage version is available, try absolute
      addressingMode += ('F' - 'C');
      tmpBuf[3] = addressingMode;
      opcode = searchAssemblerOpcodeTable(&(tmpBuf[0]));
    }
    if (opcode < 0)
      return 0;
  }
  opcode = assemblerOpcodeTable[opcode].opcode;
  if ((opcode & 0x1F) == 0x10) {                // branch instructions
    operand = (operand - (addr + 2U)) & 0xFFFFU;
    if ((operand & 0xFF80U) != 0x0000U && (operand & 0xFF80U) != 0xFF80U)
      return 0;
    operand = operand & 0xFFU;
    buf[0] = uint8_t(opcode);
    buf[1] = uint8_t(operand);
    return 2;
  }
  switch (addressingMode) {
  case 'A':
    buf[0] = uint8_t(opcode);
    return 1;
  case 'B':
  case 'C':
  case 'D':
  case 'E':
  case 'J':
  case 'K':
    if (operand >= 0x0100U)
      return 0;
    buf[0] = uint8_t(opcode);
    buf[1] = uint8_t(operand);
    return 2;
  case 'F':
  case 'G':
  case 'H':
  case 'I':
    if (operand >= 0x00010000U)
      return 0;
    buf[0] = uint8_t(opcode);
    buf[1] = uint8_t(operand & 0xFFU);
    buf[2] = uint8_t((operand >> 8) & 0xFFU);
    return 3;
  }
  return 0;
}

// ----------------------------------------------------------------------------

Plus4EmuGUIMonitor::Plus4EmuGUIMonitor(int xx, int yy, int ww, int hh,
                                       const char *ll)
  : Fl_Text_Editor(xx, yy, ww, hh, ll),
    buf_((Fl_Text_Buffer *) 0),
    debugWindow((Plus4EmuGUI_DebugWindow *) 0),
    gui((Plus4EmuGUI *) 0),
    assembleOffset(int32_t(0)),
    disassembleAddress(0U),
    disassembleOffset(int32_t(0)),
    memoryDumpAddress(0U),
    addressMask(0xFFFFU),
    cpuAddressMode(true),
    traceFile((std::FILE *) 0),
    traceInstructionsRemaining(0)
{
  buf_ = new Fl_Text_Buffer();
  buffer(buf_);
  add_key_binding(FL_Enter, FL_TEXT_EDITOR_ANY_STATE, &enterKeyCallback);
  insert_mode(0);
  scrollbar_align(FL_ALIGN_RIGHT);
}

Plus4EmuGUIMonitor::~Plus4EmuGUIMonitor()
{
  closeTraceFile();
  buffer((Fl_Text_Buffer *) 0);
  delete buf_;
}

void Plus4EmuGUIMonitor::command_assemble(const std::vector<std::string>& args)
{
  if (args.size() == 2) {
    (void) parseHexNumberEx(args[1].c_str());
    return;
  }
  uint8_t   opcodeBuf[4];
  uint32_t  addr = 0U;
  int       nBytes = assembleInstruction(&(opcodeBuf[0]), addr, args);
  if (nBytes < 1)
    throw Plus4Emu::Exception("assembler syntax error");
  uint32_t  writeAddr =
      (uint32_t(int32_t(addr) + assembleOffset)) & addressMask;
  for (int i = 0; i < nBytes; i++) {
    gui->vm.writeMemory((writeAddr + uint32_t(i)) & addressMask, opcodeBuf[i],
                        cpuAddressMode);
  }
  this->move_up();
  disassembleOffset = -assembleOffset;
  disassembleAddress = writeAddr;
  disassembleInstruction(true);
  addr = (addr + uint32_t(nBytes)) & addressMask;
  char    tmpBuf[16];
  if (cpuAddressMode)
    std::sprintf(&(tmpBuf[0]), "A   %04X  ", (unsigned int) addr);
  else
    std::sprintf(&(tmpBuf[0]), "A %06X  ", (unsigned int) addr);
  this->overstrike(&(tmpBuf[0]));
  show_insert_position();
}

void Plus4EmuGUIMonitor::command_disassemble(const std::vector<std::string>&
                                                 args)
{
  size_t    argOffs = 1;
  std::FILE *f = (std::FILE *) 0;
  if (args.size() > 1) {
    if (args[1].length() >= 1) {
      if (args[1][0] == '"') {
        argOffs++;
        std::string fileName(args[1].c_str() + 1);
        int   err = gui->vm.openFileInWorkingDirectory(f, fileName, "w", false);
        if (err != 0) {
          if (err >= -6 && err <= -2)
            printMessage(fileOpenErrorMessages[(-err) - 1]);
          else
            printMessage(fileOpenErrorMessages[0]);
          return;
        }
      }
    }
  }
  try {
    if (args.size() > (argOffs + 3))
      throw Plus4Emu::Exception("invalid number of disassemble arguments");
    uint32_t  startAddr = disassembleAddress & addressMask;
    if (args.size() > argOffs)
      startAddr = parseHexNumberEx(args[argOffs].c_str(), addressMask);
    disassembleAddress = startAddr;
    uint32_t  endAddr = (startAddr + 20U) & addressMask;
    if (args.size() > (argOffs + 1))
      endAddr = parseHexNumberEx(args[argOffs + 1].c_str(), addressMask);
    if (args.size() > (argOffs + 2)) {
      uint32_t  tmp = parseHexNumberEx(args[argOffs + 2].c_str(), addressMask);
      disassembleOffset = int32_t(tmp) - int32_t(startAddr);
      if (disassembleOffset > int32_t(addressMask >> 1))
        disassembleOffset -= int32_t(addressMask + 1U);
      else if (disassembleOffset < -(int32_t((addressMask >> 1) + 1U)))
        disassembleOffset += int32_t(addressMask + 1U);
    }
    std::string tmpBuf;
    if (!f) {
      // disassemble to screen
      while (((endAddr - disassembleAddress) & addressMask)
             > (MONITOR_MAX_LINES * 4U)) {
        uint32_t  nextAddr = gui->vm.disassembleInstruction(tmpBuf,
                                                            disassembleAddress,
                                                            cpuAddressMode,
                                                            disassembleOffset);
        disassembleAddress = nextAddr & addressMask;
      }
      while (true) {
        uint32_t  prvAddr = disassembleAddress;
        disassembleInstruction();
        while (prvAddr != disassembleAddress) {
          if (prvAddr == endAddr)
            return;
          prvAddr = (prvAddr + 1U) & addressMask;
        }
      }
    }
    else {
      // disassemble to file
      while (true) {
        uint32_t  prvAddr = disassembleAddress;
        uint32_t  nextAddr = gui->vm.disassembleInstruction(tmpBuf,
                                                            disassembleAddress,
                                                            cpuAddressMode,
                                                            disassembleOffset);
        int       n = std::fprintf(f, ". %s\n", tmpBuf.c_str());
        if (size_t(n) != (tmpBuf.length() + 3)) {
          printMessage("Error writing file");
          break;
        }
        disassembleAddress = nextAddr & addressMask;
        while (prvAddr != disassembleAddress) {
          if (prvAddr == endAddr) {
            int     err = std::fflush(f);
            std::fclose(f);
            f = (std::FILE *) 0;
            if (err != 0)
              printMessage("Error writing file");
            return;
          }
          prvAddr = (prvAddr + 1U) & addressMask;
        }
      }
    }
  }
  catch (...) {
    if (f)
      std::fclose(f);
    throw;
  }
  if (f)
    std::fclose(f);
}

void Plus4EmuGUIMonitor::command_memoryDump(const std::vector<std::string>&
                                                args)
{
  if (args.size() > 3)
    throw Plus4Emu::Exception("invalid number of memory dump arguments");
  uint32_t  startAddr = memoryDumpAddress & addressMask;
  if (args.size() > 1)
    startAddr = parseHexNumberEx(args[1].c_str(), addressMask);
  memoryDumpAddress = startAddr;
  uint32_t  endAddr = (startAddr + 95U) & addressMask;
  if (args.size() > 2)
    endAddr = parseHexNumberEx(args[2].c_str(), addressMask);
  while (((endAddr - memoryDumpAddress) & addressMask)
         > (MONITOR_MAX_LINES * 8U)) {
    memoryDumpAddress = (memoryDumpAddress + 8U) & addressMask;
  }
  while (true) {
    uint32_t  prvAddr = memoryDumpAddress;
    memoryDump();
    while (prvAddr != memoryDumpAddress) {
      if (prvAddr == endAddr)
        return;
      prvAddr = (prvAddr + 1U) & addressMask;
    }
  }
}

void Plus4EmuGUIMonitor::command_memoryModify(const std::vector<std::string>&
                                                  args)
{
  if (args.size() < 2)
    throw Plus4Emu::Exception("insufficient arguments for memory modify");
  uint32_t  addr = parseHexNumberEx(args[1].c_str(), addressMask);
  memoryDumpAddress = addr;
  for (size_t i = 2; i < args.size(); i++) {
    if (args[i] == ":")
      break;
    uint32_t  value = parseHexNumberEx(args[i].c_str());
    if (value >= 0x0100U)
      throw Plus4Emu::Exception("byte value is out of range");
    gui->vm.writeMemory(addr, uint8_t(value), cpuAddressMode);
    addr = (addr + 1U) & addressMask;
  }
  this->move_up();
  memoryDump();
}

void Plus4EmuGUIMonitor::command_printRegisters(const std::vector<std::string>&
                                                    args)
{
  if (args.size() != 1)
    throw Plus4Emu::Exception("too many arguments");
  printMessage("  PC  SR AC XR YR SP");
  printCPURegisters();
}

void Plus4EmuGUIMonitor::command_setRegisters(const std::vector<std::string>&
                                                  args)
{
  if (args.size() < 2 || args.size() > 7)
    throw Plus4Emu::Exception("invalid number of arguments");
  Plus4::M7501Registers r;
  reinterpret_cast<Plus4::Plus4VM *>(&(gui->vm))->getCPURegisters(r);
  uint32_t  tmp = parseHexNumberEx(args[1].c_str());
  if (tmp > 0xFFFFU)
    throw Plus4Emu::Exception("address is out of range");
  r.reg_PC = uint16_t(tmp);
  if (args.size() >= 3) {
    tmp = parseHexNumberEx(args[2].c_str());
    if (tmp > 0xFFU)
      throw Plus4Emu::Exception("byte value is out of range");
    r.reg_SR = uint8_t(tmp);
  }
  if (args.size() >= 4) {
    tmp = parseHexNumberEx(args[3].c_str());
    if (tmp > 0xFFU)
      throw Plus4Emu::Exception("byte value is out of range");
    r.reg_AC = uint8_t(tmp);
  }
  if (args.size() >= 5) {
    tmp = parseHexNumberEx(args[4].c_str());
    if (tmp > 0xFFU)
      throw Plus4Emu::Exception("byte value is out of range");
    r.reg_XR = uint8_t(tmp);
  }
  if (args.size() >= 6) {
    tmp = parseHexNumberEx(args[5].c_str());
    if (tmp > 0xFFU)
      throw Plus4Emu::Exception("byte value is out of range");
    r.reg_YR = uint8_t(tmp);
  }
  if (args.size() >= 7) {
    tmp = parseHexNumberEx(args[6].c_str());
    if (tmp > 0xFFU)
      throw Plus4Emu::Exception("byte value is out of range");
    r.reg_SP = uint8_t(tmp);
  }
  reinterpret_cast<Plus4::Plus4VM *>(&(gui->vm))->setCPURegisters(r);
  this->move_up();
  printCPURegisters();
}

void Plus4EmuGUIMonitor::command_go(const std::vector<std::string>& args)
{
  if (args.size() > 2)
    throw Plus4Emu::Exception("too many arguments");
  if (args.size() > 1) {
    Plus4::M7501Registers r;
    reinterpret_cast<Plus4::Plus4VM *>(&(gui->vm))->getCPURegisters(r);
    uint32_t  tmp = parseHexNumberEx(args[1].c_str());
    if (tmp > 0xFFFFU)
      throw Plus4Emu::Exception("address is out of range");
    r.reg_PC = uint16_t(tmp);
    reinterpret_cast<Plus4::Plus4VM *>(&(gui->vm))->setCPURegisters(r);
  }
  debugWindow->focusWidget = this;
  gui->vm.setSingleStepMode(0);
  debugWindow->deactivate();
}

void Plus4EmuGUIMonitor::command_searchPattern(const std::vector<std::string>&
                                                   args)
{
  if (args.size() < 4)
    throw Plus4Emu::Exception("insufficient arguments for search");
  uint32_t  startAddr = parseHexNumberEx(args[1].c_str());
  uint32_t  endAddr = parseHexNumberEx(args[2].c_str());
  if ((startAddr | endAddr) > addressMask)
    throw Plus4Emu::Exception("address is out of range");
  size_t    matchCnt = 0;
  uint32_t  matchAddrs[64];
  while (true) {
    int32_t   nextAddr = searchPattern(args, 3, args.size() - 3,
                                       startAddr, endAddr, cpuAddressMode);
    if (nextAddr < 0) {
      startAddr = endAddr;
      break;
    }
    startAddr = uint32_t(nextAddr) & addressMask;
    matchAddrs[matchCnt++] = startAddr;
    if (matchCnt >= 64 || startAddr == endAddr)
      break;
    startAddr = (startAddr + 1U) & addressMask;
  }
  char    tmpBuf[64];
  int     bufPos = 0;
  for (size_t i = 0; i < matchCnt; i++) {
    int     n = std::sprintf(&(tmpBuf[bufPos]), " %0*X",
                             int(cpuAddressMode ? 4 : 6),
                             (unsigned int) matchAddrs[i]);
    bufPos += n;
    if (((i & 3) == 3 && bufPos >= 28) || i == (matchCnt - 1)) {
      printMessage(&(tmpBuf[0]));
      bufPos = 0;
    }
  }
  if (startAddr != endAddr) {
    std::sprintf(&(tmpBuf[0]), "H %X %X",
                 (unsigned int) ((startAddr + 1U) & addressMask),
                 (unsigned int) endAddr);
    std::string newCmd(&(tmpBuf[0]));
    for (size_t i = 3; i < args.size(); i++) {
      newCmd += ' ';
      newCmd += args[i];
      if (args[i].length() > 0) {
        if (args[i][0] == '"')
          newCmd += '"';
      }
    }
    printMessage(newCmd.c_str());
    this->move_up();
  }
}

void Plus4EmuGUIMonitor::command_searchAndReplace(
    const std::vector<std::string>& args)
{
  size_t  searchArgOffs = 3;
  size_t  searchArgCnt = 0;
  for (size_t i = searchArgOffs; i < args.size(); i++) {
    if (args[i] == ",")
      break;
    searchArgCnt++;
  }
  size_t  replaceArgOffs = searchArgCnt + 4;
  size_t  replaceArgCnt = 0;
  for (size_t i = replaceArgOffs; i < args.size(); i++) {
    replaceArgCnt++;
  }
  if (searchArgCnt < 1 || replaceArgCnt < 1)
    throw Plus4Emu::Exception("insufficient arguments for search/replace");
  std::vector<uint8_t>  searchString_;
  std::vector<uint8_t>  searchMask_;
  parseSearchPattern(searchString_, searchMask_,
                     args, searchArgOffs, searchArgCnt);
  std::vector<uint8_t>  replaceString_;
  std::vector<uint8_t>  replaceMask_;
  parseSearchPattern(replaceString_, replaceMask_,
                     args, replaceArgOffs, replaceArgCnt);
  if (replaceString_.size() < 1)
    throw Plus4Emu::Exception("insufficient arguments for search/replace");
  uint32_t  startAddr = parseHexNumberEx(args[1].c_str());
  uint32_t  endAddr = parseHexNumberEx(args[2].c_str());
  if ((startAddr | endAddr) > addressMask)
    throw Plus4Emu::Exception("address is out of range");
  size_t    replaceCnt = 0;
  while (true) {
    int32_t   nextAddr = searchPattern(searchString_, searchMask_,
                                       startAddr, endAddr, cpuAddressMode);
    if (nextAddr < 0)
      break;
    replaceCnt++;
    startAddr = uint32_t(nextAddr) & addressMask;
    size_t  j = 0;
    do {
      uint8_t c = gui->vm.readMemory(startAddr, cpuAddressMode);
      c &= (replaceMask_[j] ^ uint8_t(0xFF));
      c |= (replaceString_[j] & replaceMask_[j]);
      gui->vm.writeMemory(startAddr, c, cpuAddressMode);
      if (startAddr == endAddr)
        break;
      startAddr = (startAddr + 1U) & addressMask;
    } while (++j < replaceString_.size());
    if (j < replaceString_.size())
      break;
  }
  char    tmpBuf[64];
  std::sprintf(&(tmpBuf[0]), "Replaced %lu matches",
               (unsigned long) replaceCnt);
  printMessage(&(tmpBuf[0]));
}

void Plus4EmuGUIMonitor::command_memoryCopy(const std::vector<std::string>&
                                                args)
{
  if (args.size() != 4)
    throw Plus4Emu::Exception("invalid number of memory copy arguments");
  uint32_t  srcStartAddr = parseHexNumberEx(args[1].c_str());
  uint32_t  srcEndAddr = parseHexNumberEx(args[2].c_str());
  uint32_t  dstStartAddr = parseHexNumberEx(args[3].c_str());
  if ((srcStartAddr | srcEndAddr | dstStartAddr) > addressMask)
    throw Plus4Emu::Exception("address is out of range");
  if (srcStartAddr >= dstStartAddr) {
    while (true) {
      uint8_t c = gui->vm.readMemory(srcStartAddr, cpuAddressMode);
      gui->vm.writeMemory(dstStartAddr, c, cpuAddressMode);
      if (srcStartAddr == srcEndAddr)
        break;
      srcStartAddr = (srcStartAddr + 1U) & addressMask;
      dstStartAddr = (dstStartAddr + 1U) & addressMask;
    }
  }
  else {
    uint32_t  dstEndAddr =
        (dstStartAddr + (srcEndAddr - srcStartAddr)) & addressMask;
    while (true) {
      uint8_t c = gui->vm.readMemory(srcEndAddr, cpuAddressMode);
      gui->vm.writeMemory(dstEndAddr, c, cpuAddressMode);
      if (srcStartAddr == srcEndAddr)
        break;
      srcEndAddr = (srcEndAddr - 1U) & addressMask;
      dstEndAddr = (dstEndAddr - 1U) & addressMask;
    }
  }
}

void Plus4EmuGUIMonitor::command_memoryFill(const std::vector<std::string>&
                                                args)
{
  if (args.size() < 4)
    throw Plus4Emu::Exception("insufficient arguments for memory fill");
  uint32_t  startAddr = parseHexNumberEx(args[1].c_str());
  uint32_t  endAddr = parseHexNumberEx(args[2].c_str());
  if ((startAddr | endAddr) > addressMask)
    throw Plus4Emu::Exception("address is out of range");
  std::vector<uint8_t>  pattern_;
  for (size_t i = 3; i < args.size(); i++) {
    uint32_t  c = parseHexNumberEx(args[i].c_str());
    if (c > 0xFFU)
      throw Plus4Emu::Exception("byte value is out of range");
    pattern_.push_back(uint8_t(c));
  }
  size_t  patternPos = 0;
  while (true) {
    gui->vm.writeMemory(startAddr, pattern_[patternPos], cpuAddressMode);
    if (startAddr == endAddr)
      break;
    startAddr = (startAddr + 1U) & addressMask;
    if (++patternPos >= pattern_.size())
      patternPos = 0;
  }
}

void Plus4EmuGUIMonitor::command_memoryCompare(const std::vector<std::string>&
                                                   args)
{
  if (args.size() != 4)
    throw Plus4Emu::Exception("invalid number of memory compare arguments");
  uint32_t  startAddr1 = parseHexNumberEx(args[1].c_str());
  uint32_t  endAddr1 = parseHexNumberEx(args[2].c_str());
  uint32_t  startAddr2 = parseHexNumberEx(args[3].c_str());
  if ((startAddr1 | endAddr1 | startAddr2) > addressMask)
    throw Plus4Emu::Exception("address is out of range");
  size_t    diffCnt = 0;
  uint32_t  diffAddrs[64];
  size_t    bytesRemaining = size_t((endAddr1 - startAddr1) & addressMask) + 1;
  do {
    uint8_t c1 = gui->vm.readMemory(startAddr1, cpuAddressMode);
    uint8_t c2 = gui->vm.readMemory(startAddr2, cpuAddressMode);
    if (c1 != c2)
      diffAddrs[diffCnt++] = startAddr1;
    startAddr1 = (startAddr1 + 1U) & addressMask;
    startAddr2 = (startAddr2 + 1U) & addressMask;
    bytesRemaining--;
  } while (diffCnt < 64 && bytesRemaining > 0);
  char    tmpBuf[64];
  int     bufPos = 0;
  for (size_t i = 0; i < diffCnt; i++) {
    int     n = std::sprintf(&(tmpBuf[bufPos]), " %0*X",
                             int(cpuAddressMode ? 4 : 6),
                             (unsigned int) diffAddrs[i]);
    bufPos += n;
    if (((i & 3) == 3 && bufPos >= 28) || i == (diffCnt - 1)) {
      printMessage(&(tmpBuf[0]));
      bufPos = 0;
    }
  }
  if (bytesRemaining > 0) {
    std::sprintf(&(tmpBuf[0]), "C %X %X %X",
                 (unsigned int) startAddr1, (unsigned int) endAddr1,
                 (unsigned int) startAddr2);
    printMessage(&(tmpBuf[0]));
    this->move_up();
  }
}

void Plus4EmuGUIMonitor::command_assemblerOffset(
    const std::vector<std::string>& args)
{
  if (args.size() <= 1) {
    assembleOffset = int32_t(0);
  }
  else {
    bool    negativeFlag = false;
    size_t  argOffs = 1;
    if (args[1] == "-") {
      negativeFlag = true;
      argOffs++;
    }
    else if (args[1] == "+") {
      argOffs++;
    }
    if (args.size() != (argOffs + 1))
      throw Plus4Emu::Exception("invalid number of assemble offset arguments");
    assembleOffset =
        int32_t(parseHexNumberEx(args[argOffs].c_str(), addressMask));
    if (assembleOffset > int32_t(addressMask >> 1))
      assembleOffset -= int32_t(addressMask + 1U);
    if (negativeFlag)
      assembleOffset = -assembleOffset;
  }
  disassembleOffset = -assembleOffset;
  char    tmpBuf[128];
  if (assembleOffset == 0) {
    std::sprintf(&(tmpBuf[0]), "Assemble offset set to 0\n"
                               "Disassemble offset set to 0");
  }
  else {
    int     n = (cpuAddressMode ? 4 : 6);
    if (assembleOffset > 0) {
      std::sprintf(&(tmpBuf[0]), "Assemble offset set to +%0*X\n"
                                 "Disassemble offset set to -%0*X",
                   n, (unsigned int) assembleOffset,
                   n, (unsigned int) assembleOffset);
    }
    else {
      std::sprintf(&(tmpBuf[0]), "Assemble offset set to -%0*X\n"
                                 "Disassemble offset set to +%0*X",
                   n, (unsigned int) disassembleOffset,
                   n, (unsigned int) disassembleOffset);
    }
  }
  printMessage(&(tmpBuf[0]));
}

void Plus4EmuGUIMonitor::command_printInfo(
    const std::vector<std::string>& args)
{
  if (args.size() > 1)
    throw Plus4Emu::Exception("too many arguments");
  switch (gui->vm.getDebugContext()) {
  case 0:
    printMessage("Debug context:       main CPU");
    break;
  case 1:
    printMessage("Debug context:       floppy / unit 8");
    break;
  case 2:
    printMessage("Debug context:       floppy / unit 9");
    break;
  case 3:
    printMessage("Debug context:       floppy / unit 10");
    break;
  case 4:
    printMessage("Debug context:       floppy / unit 11");
    break;
  case 5:
    printMessage("Debug context:       printer");
    break;
  }
  if (cpuAddressMode)
    printMessage("Address mode:        CPU (16 bit)");
  else
    printMessage("Address mode:        physical (22 bit)");
  char    tmpBuf[64];
  int     n = (cpuAddressMode ? 4 : 6);
  std::sprintf(&(tmpBuf[0]), "Assemble offset:    %c%0*X",
               int(assembleOffset == 0 ?
                   ' ' : (assembleOffset > 0 ? '+' : '-')),
               n,
               (unsigned int) (assembleOffset >= 0 ?
                               assembleOffset : (-assembleOffset)));
  printMessage(&(tmpBuf[0]));
  std::sprintf(&(tmpBuf[0]), "Disassemble offset: %c%0*X",
               int(disassembleOffset == 0 ?
                   ' ' : (disassembleOffset > 0 ? '+' : '-')),
               n,
               (unsigned int) (disassembleOffset >= 0 ?
                               disassembleOffset : (-disassembleOffset)));
  printMessage(&(tmpBuf[0]));
}

void Plus4EmuGUIMonitor::command_continue(const std::vector<std::string>& args)
{
  if (args.size() > 1)
    throw Plus4Emu::Exception("too many arguments");
  debugWindow->focusWidget = this;
  gui->vm.setSingleStepMode(0);
  debugWindow->deactivate();
}

void Plus4EmuGUIMonitor::command_step(const std::vector<std::string>& args)
{
  if (args.size() > 1)
    throw Plus4Emu::Exception("too many arguments");
  debugWindow->focusWidget = this;
  gui->vm.setSingleStepMode(1);
  debugWindow->deactivate();
}

void Plus4EmuGUIMonitor::command_stepOver(const std::vector<std::string>& args)
{
  if (args.size() > 1)
    throw Plus4Emu::Exception("too many arguments");
  debugWindow->focusWidget = this;
  gui->vm.setSingleStepMode(2);
  debugWindow->deactivate();
}

void Plus4EmuGUIMonitor::command_trace(const std::vector<std::string>& args)
{
  closeTraceFile();
  if (args.size() < 2 || args.size() > 4)
    throw Plus4Emu::Exception("invalid number of arguments");
  if (args[1].length() < 1 || args[1][0] != '"')
    throw Plus4Emu::Exception("file name is not a string");
  uint32_t  maxInsns = 0U;
  int32_t   startAddr = int32_t(-1);
  if (args.size() > 2) {
    maxInsns = parseHexNumberEx(args[2].c_str());
    if (maxInsns > 0x01000000U)
      throw Plus4Emu::Exception("invalid instruction count");
  }
  if (!maxInsns)
    maxInsns = 65536U;
  if (args.size() > 3) {
    uint32_t  n = parseHexNumberEx(args[3].c_str());
    if (n > 0xFFFFU)
      throw Plus4Emu::Exception("address is out of range");
    startAddr = int32_t(n);
  }
  std::string fileName(args[1].c_str() + 1);
  std::FILE *f = (std::FILE *) 0;
  int       err = gui->vm.openFileInWorkingDirectory(f, fileName, "w", false);
  if (err != 0) {
    if (err >= -6 && err <= -2)
      printMessage(fileOpenErrorMessages[(-err) - 1]);
    else
      printMessage(fileOpenErrorMessages[0]);
    return;
  }
  traceFile = f;
  traceInstructionsRemaining = size_t(maxInsns);
  if (startAddr >= 0) {
    Plus4::M7501Registers r;
    reinterpret_cast<Plus4::Plus4VM *>(&(gui->vm))->getCPURegisters(r);
    r.reg_PC = uint16_t(startAddr);
    reinterpret_cast<Plus4::Plus4VM *>(&(gui->vm))->setCPURegisters(r);
  }
  debugWindow->focusWidget = this;
  gui->vm.setSingleStepMode(3);
  debugWindow->deactivate();
}

void Plus4EmuGUIMonitor::command_setDebugContext(
    const std::vector<std::string>& args)
{
  if (args.size() > 2)
    throw Plus4Emu::Exception("too many arguments");
  int     n = 0;
  if (args.size() < 2) {
    n = gui->vm.getDebugContext();
    n = (n + 1) % 6;
  }
  else {
    uint32_t  tmp = parseHexNumberEx(args[1].c_str());
    switch (tmp) {
    case 0U:
      break;
    case 4U:
      n = 5;
      break;
    case 8U:
    case 9U:
      n = int(tmp - 7U);
      break;
    case 16U:
    case 17U:
      n = int(tmp - 13U);
      break;
    default:
      throw Plus4Emu::Exception("invalid debug context");
    }
  }
  debugWindow->debugContextValuator->value(n);
  debugWindow->debugContextValuator->do_callback();
  switch (gui->vm.getDebugContext()) {
  case 0:
    printMessage("Debug context set to main CPU");
    break;
  case 1:
    printMessage("Debug context set to floppy unit 8");
    break;
  case 2:
    printMessage("Debug context set to floppy unit 9");
    break;
  case 3:
    printMessage("Debug context set to floppy unit 10");
    break;
  case 4:
    printMessage("Debug context set to floppy unit 11");
    break;
  case 5:
    printMessage("Debug context set to printer");
    break;
  }
}

void Plus4EmuGUIMonitor::command_load(const std::vector<std::string>& args,
                                      bool verifyMode)
{
  if (args.size() < 2 || args.size() > 4)
    throw Plus4Emu::Exception("invalid number of arguments");
  if (args[1].length() < 1 || args[1][0] != '"')
    throw Plus4Emu::Exception("file name is not a string");
  std::string fileName(args[1].c_str() + 1);
  bool      haveStartAddr = false;
  bool      haveEndAddr = false;
  bool      cpuAddressMode_ = true;
  uint32_t  startAddr = 0U;
  uint32_t  endAddr = 0U;
  uint32_t  addressMask_ = 0xFFFFU;
  if (args.size() > 2) {
    startAddr = parseHexNumberEx(args[2].c_str());
    haveStartAddr = true;
    cpuAddressMode_ = cpuAddressMode;
    addressMask_ = addressMask;
  }
  if (args.size() > 3) {
    endAddr = parseHexNumberEx(args[3].c_str());
    haveEndAddr = true;
  }
  if ((startAddr | endAddr) > addressMask_)
    throw Plus4Emu::Exception("address is out of range");
  std::FILE *f = (std::FILE *) 0;
  int       err = gui->vm.openFileInWorkingDirectory(f, fileName, "rb");
  if (err == 0) {
    try {
      uint16_t  tmpAddr =
          Plus4::TED7360::readPRGFileHeader(f, fileName.c_str());
      if (!haveStartAddr)
        startAddr = tmpAddr;
    }
    catch (...) {
      err = -1;
    }
  }
  if (err != 0) {
    if (err >= -6 && err <= -2)
      printMessage(fileOpenErrorMessages[(-err) - 1]);
    else
      printMessage(fileOpenErrorMessages[0]);
    return;
  }
  try {
    if (!haveEndAddr)
      endAddr = (startAddr - 1U) & addressMask_;
    char      tmpBuf[64];
    uint32_t  addr = startAddr;
    int       c = 0;
    if (!verifyMode) {
      // load
      while (addr != endAddr) {
        c = std::fgetc(f);
        if (c == EOF)
          break;
        gui->vm.writeMemory(addr, uint8_t(c & 0xFF), cpuAddressMode_);
        addr = (addr + 1U) & addressMask_;
      }
      if (addr != startAddr) {
        int     n = (cpuAddressMode_ ? 4 : 6);
        std::sprintf(&(tmpBuf[0]), "Loaded PRG file to %0*X-%0*X",
                     n, (unsigned int) startAddr, n, (unsigned int) addr);
        printMessage(&(tmpBuf[0]));
      }
    }
    else {
      // verify
      size_t    diffCnt = 0;
      while (addr != endAddr) {
        c = std::fgetc(f);
        if (c == EOF)
          break;
        if (readMemoryForFileIO(addr, cpuAddressMode_) != uint8_t(c & 0xFF))
          diffCnt++;
        addr = (addr + 1U) & addressMask_;
      }
      std::sprintf(&(tmpBuf[0]), "%lu differences", (unsigned long) diffCnt);
      printMessage(&(tmpBuf[0]));
    }
  }
  catch (std::exception& e) {
    std::fclose(f);
    printMessage(e.what());
    return;
  }
  std::fclose(f);
}

void Plus4EmuGUIMonitor::command_save(const std::vector<std::string>& args)
{
  if (args.size() != 5 && (args.size() != 4 || !cpuAddressMode))
    throw Plus4Emu::Exception("invalid number of arguments");
  if (args[1].length() < 1 || args[1][0] != '"')
    throw Plus4Emu::Exception("file name is not a string");
  std::string fileName(args[1].c_str() + 1);
  uint32_t  startAddr = parseHexNumberEx(args[2].c_str());
  uint32_t  endAddr = parseHexNumberEx(args[3].c_str());
  uint32_t  loadAddr = startAddr;
  if (args.size() > 4)
    loadAddr = parseHexNumberEx(args[4].c_str());
  if ((startAddr | endAddr) > addressMask || loadAddr > 0xFFFFU)
    throw Plus4Emu::Exception("address is out of range");
  std::FILE *f = (std::FILE *) 0;
  int       err = gui->vm.openFileInWorkingDirectory(f, fileName, "wb", false);
  if (err != 0) {
    if (err >= -6 && err <= -2)
      printMessage(fileOpenErrorMessages[(-err) - 1]);
    else
      printMessage(fileOpenErrorMessages[0]);
    return;
  }
  try {
    if (std::fputc(int(loadAddr & 0xFFU), f) == EOF)
      throw Plus4Emu::Exception("Error writing file");
    if (std::fputc(int(loadAddr >> 8), f) == EOF)
      throw Plus4Emu::Exception("Error writing file");
    while (startAddr != endAddr) {
      uint8_t c = readMemoryForFileIO(startAddr, cpuAddressMode);
      if (std::fputc(int(c), f) == EOF)
        throw Plus4Emu::Exception("Error writing file");
      startAddr = (startAddr + 1U) & addressMask;
    }
    if (std::fflush(f) != 0)
      throw Plus4Emu::Exception("Error writing file");
  }
  catch (std::exception& e) {
    std::fclose(f);
    printMessage(e.what());
    return;
  }
  std::fclose(f);
}

void Plus4EmuGUIMonitor::command_toggleCPUAddressMode(
    const std::vector<std::string>& args)
{
  if (args.size() > 2)
    throw Plus4Emu::Exception("too many arguments for address mode");
  bool    oldCPUAddressMode = cpuAddressMode;
  if (args.size() > 1) {
    uint32_t  n = parseHexNumberEx(args[1].c_str());
    cpuAddressMode = (n != 0U);
  }
  else
    cpuAddressMode = !cpuAddressMode;
  addressMask = (cpuAddressMode ? 0x0000FFFFU : 0x003FFFFFU);
  if (cpuAddressMode != oldCPUAddressMode) {
    assembleOffset = int32_t(0);
    disassembleOffset = int32_t(0);
    if (cpuAddressMode)
      printMessage("CPU address mode");
    else
      printMessage("Physical address mode");
  }
}

void Plus4EmuGUIMonitor::command_help(const std::vector<std::string>& args)
{
  if (args.size() != 2) {
    printMessage(".       assemble");
    printMessage(";       set CPU registers");
    printMessage(">       modify memory");
    printMessage("?       print help");
    printMessage("A       assemble");
    printMessage("AM      set/toggle CPU/physical address mode");
    printMessage("AO      set assemble and disassemble offset");
    printMessage("C       compare memory");
    printMessage("D       disassemble");
    printMessage("F       fill memory with pattern");
    printMessage("G       continue or go to address");
    printMessage("H       search for pattern in memory");
    printMessage("I       print current settings");
    printMessage("L       load PRG file to memory");
    printMessage("M       dump memory");
    printMessage("R       print CPU registers");
    printMessage("S       save memory to PRG file");
    printMessage("SR      search and replace pattern in memory");
    printMessage("T       copy memory");
    printMessage("TR      trace (log instructions to file)");
    printMessage("V       verify (compare memory and PRG file)");
    printMessage("W       set debug context");
    printMessage("X       continue");
    printMessage("Y       step over");
    printMessage("Z       step");
  }
  else if (args[1] == "." || args[1] == "A") {
    printMessage("A <address> ...");
  }
  else if (args[1] == ";") {
    printMessage(";<pc> [sr [ac [xr [yr [sp]]]]]");
  }
  else if (args[1] == ">") {
    printMessage("><address> [value1 [value2 [...]]]");
  }
  else if (args[1] == "?") {
    printMessage("?       print short help for all commands");
    printMessage("? <cmd> print detailed help for a command");
  }
  else if (args[1] == "AM") {
    printMessage("AM      toggle CPU/physical address mode");
    printMessage("AM 0    set physical (22 bit) address mode");
    printMessage("AM 1    set CPU (16 bit) address mode");
    printMessage("Changing the address mode resets assemble");
    printMessage("and disassemble offset to zero");
  }
  else if (args[1] == "AO") {
    printMessage("AO      reset assemble/disassemble offset");
    printMessage("AO <n>  set assemble offset to +n bytes");
    printMessage("AO +<n> set assemble offset to +n bytes");
    printMessage("AO -<n> set assemble offset to -n bytes");
    printMessage("Disassemble offset is set to -(asm offset)");
  }
  else if (args[1] == "C") {
    printMessage("C <start1> <end1> <start2>");
  }
  else if (args[1] == "D") {
    printMessage("D [\"filename\"] [start [end [runtimeAddr]]]");
  }
  else if (args[1] == "F") {
    printMessage("F <start> <end> <value1> [value2 [...]]");
  }
  else if (args[1] == "G") {
    printMessage("G       continue emulation");
    printMessage("G<addr> go to address and continue emulation");
  }
  else if (args[1] == "H") {
    printMessage("H <start> <end> <pattern>");
    printMessage("Pattern can include any of the following:");
    printMessage("    NN  search for exact byte value NN");
    printMessage("  MMNN  search for byte NN with bit mask MM");
    printMessage("     *  matches any single byte");
    printMessage(" \"str\"  search for string (bit mask = 3F)");
  }
  else if (args[1] == "I") {
    printMessage("I       print current monitor settings");
  }
  else if (args[1] == "L") {
    printMessage("L <\"filename\"> [start [end+1]]");
    printMessage("Zeropage variables are not updated");
  }
  else if (args[1] == "M") {
    printMessage("M [start [end]]");
  }
  else if (args[1] == "R") {
    printMessage("R       print CPU registers");
  }
  else if (args[1] == "S") {
    printMessage("S <\"filename\"> <start> <end+1> <loadAddr>");
    printMessage("Zeropage variables are not updated");
    printMessage("Load address is optional in CPU address mode");
  }
  else if (args[1] == "SR") {
    printMessage("SR <start> <end> <searchPat>, <replacePat>");
    printMessage("Patterns can include any of the following:");
    printMessage("    NN  exact byte value NN");
    printMessage("  MMNN  byte NN with bit mask MM");
    printMessage("     *  match any single byte / no change");
    printMessage(" \"str\"  string (bit mask = 3F)");
  }
  else if (args[1] == "T") {
    printMessage("T <srcStart> <srcEnd> <dstStart>");
  }
  else if (args[1] == "TR") {
    printMessage("TR <\"filename\"> [maxInsns [addr]]");
    printMessage("maxInsns=0 (default) is interpreted as 65536");
  }
  else if (args[1] == "V") {
    printMessage("V <\"filename\"> [start [end+1]]");
    printMessage("Zeropage variables are not updated");
  }
  else if (args[1] == "W") {
    printMessage("W       cycle debug context");
    printMessage("W0      set debug context to main CPU");
    printMessage("W4      set debug context to printer");
    printMessage("W8      set debug context to floppy unit 8");
    printMessage("W9      set debug context to floppy unit 9");
    printMessage("W10     set debug context to floppy unit 10");
    printMessage("W11     set debug context to floppy unit 11");
  }
  else if (args[1] == "X") {
    printMessage("X       continue emulation");
  }
  else if (args[1] == "Y") {
    printMessage("Y       step one instruction with step over");
  }
  else if (args[1] == "Z") {
    printMessage("Z       step one CPU instruction");
  }
  else {
    printMessage("Unknown command name");
  }
}

int Plus4EmuGUIMonitor::enterKeyCallback(int c, Fl_Text_Editor *e_)
{
  (void) c;
  Plus4EmuGUIMonitor& e = *(reinterpret_cast<Plus4EmuGUIMonitor *>(e_));
  const char  *s = e.buf_->line_text(e.insert_position());
  e.moveDown();
  if (s) {
    if (s[0] != '\0' && s[0] != '\n') {
      try {
        e.parseCommand(s);
      }
      catch (...) {
        e.move_up();
        e.insert_position(e.buf_->line_end(e.insert_position()) - 1);
        e.overstrike("?");
        e.moveDown();
      }
    }
    std::free(const_cast<char *>(s));
  }
  return 1;
}

void Plus4EmuGUIMonitor::moveDown()
{
  try {
    insert_position(buf_->line_end(insert_position()));
    if (insert_position() >= buf_->length()) {
      int     n = buf_->count_lines(0, buf_->length());
      while (n >= MONITOR_MAX_LINES) {
        buf_->remove(0, buf_->line_end(0) + 1);
        n--;
      }
      insert_position(buf_->length());
      this->insert("\n");
    }
    else {
      move_down();
      insert_position(buf_->line_start(insert_position()));
    }
    show_insert_position();
  }
  catch (...) {
  }
}

void Plus4EmuGUIMonitor::parseCommand(const char *s)
{
  std::vector<std::string>  args;
  tokenizeString(args, s);
  if (args.size() == 0)
    return;
  if (args[0] == ";")
    command_setRegisters(args);
  else if (args[0] == ">")
    command_memoryModify(args);
  else if (args[0] == "?")
    command_help(args);
  else if (args[0] == "A" || args[0] == ".")
    command_assemble(args);
  else if (args[0] == "AM")
    command_toggleCPUAddressMode(args);
  else if (args[0] == "AO")
    command_assemblerOffset(args);
  else if (args[0] == "C")
    command_memoryCompare(args);
  else if (args[0] == "D")
    command_disassemble(args);
  else if (args[0] == "F")
    command_memoryFill(args);
  else if (args[0] == "G")
    command_go(args);
  else if (args[0] == "H")
    command_searchPattern(args);
  else if (args[0] == "I")
    command_printInfo(args);
  else if (args[0] == "L")
    command_load(args, false);
  else if (args[0] == "M")
    command_memoryDump(args);
  else if (args[0] == "R")
    command_printRegisters(args);
  else if (args[0] == "S")
    command_save(args);
  else if (args[0] == "SR")
    command_searchAndReplace(args);
  else if (args[0] == "T")
    command_memoryCopy(args);
  else if (args[0] == "TR")
    command_trace(args);
  else if (args[0] == "V")
    command_load(args, true);
  else if (args[0] == "W")
    command_setDebugContext(args);
  else if (args[0] == "X")
    command_continue(args);
  else if (args[0] == "Y")
    command_stepOver(args);
  else if (args[0] == "Z")
    command_step(args);
  else
    throw Plus4Emu::Exception("invalid monitor command");
}

void Plus4EmuGUIMonitor::printMessage(const char *s)
{
  if (!s)
    return;
  try {
    std::string tmpBuf;
    while (true) {
      tmpBuf = "";
      while (*s != '\0' && *s != '\n') {
        tmpBuf += (*s);
        s++;
      }
      insert_position(buf_->line_start(insert_position()));
      overstrike(tmpBuf.c_str());
      if (insert_position() < buf_->line_end(insert_position()))
        buf_->remove(insert_position(), buf_->line_end(insert_position()));
      moveDown();
      if (*s == '\0')
        break;
      s++;
    }
  }
  catch (...) {
  }
}

void Plus4EmuGUIMonitor::disassembleInstruction(bool assembleMode)
{
  disassembleAddress = disassembleAddress & addressMask;
  std::string tmpBuf;
  uint32_t  nextAddr = gui->vm.disassembleInstruction(tmpBuf,
                                                      disassembleAddress,
                                                      cpuAddressMode,
                                                      disassembleOffset);
  disassembleAddress = nextAddr & addressMask;
  if (!assembleMode)
    tmpBuf = std::string(". ") + tmpBuf;
  else
    tmpBuf = std::string("A ") + tmpBuf;
  printMessage(tmpBuf.c_str());
}

void Plus4EmuGUIMonitor::memoryDump()
{
  char      tmpBuf[64];
  uint8_t   dataBuf[8];
  uint32_t  startAddr = memoryDumpAddress & addressMask;
  memoryDumpAddress = startAddr;
  for (int i = 0; i < 8; i++) {
    dataBuf[i] = gui->vm.readMemory(memoryDumpAddress, cpuAddressMode) & 0xFF;
    memoryDumpAddress = (memoryDumpAddress + 1U) & addressMask;
  }
  char    *bufp = &(tmpBuf[0]);
  int     n = std::sprintf(bufp, ">%0*X",
                           int(cpuAddressMode ? 4 : 6),
                           (unsigned int) startAddr);
  bufp = bufp + n;
  n = std::sprintf(bufp, "  %02X %02X %02X %02X %02X %02X %02X %02X",
                   (unsigned int) dataBuf[0], (unsigned int) dataBuf[1],
                   (unsigned int) dataBuf[2], (unsigned int) dataBuf[3],
                   (unsigned int) dataBuf[4], (unsigned int) dataBuf[5],
                   (unsigned int) dataBuf[6], (unsigned int) dataBuf[7]);
  bufp = bufp + n;
  for (int i = 0; i < 8; i++) {
    dataBuf[i] &= uint8_t(0x7F);
    if (dataBuf[i] < uint8_t(' ') || dataBuf[i] == uint8_t(0x7F))
      dataBuf[i] = uint8_t('.');
  }
  std::sprintf(bufp, "  :%c%c%c%c%c%c%c%c",
               int(dataBuf[0]), int(dataBuf[1]), int(dataBuf[2]),
               int(dataBuf[3]), int(dataBuf[4]), int(dataBuf[5]),
               int(dataBuf[6]), int(dataBuf[7]));
  printMessage(&(tmpBuf[0]));
}

void Plus4EmuGUIMonitor::printCPURegisters()
{
  Plus4::M7501Registers r;
  reinterpret_cast<Plus4::Plus4VM *>(&(gui->vm))->getCPURegisters(r);
  char    tmpBuf[32];
  std::sprintf(&(tmpBuf[0]), ";%04X %02X %02X %02X %02X %02X",
               (unsigned int) r.reg_PC, (unsigned int) r.reg_SR,
               (unsigned int) r.reg_AC, (unsigned int) r.reg_XR,
               (unsigned int) r.reg_YR, (unsigned int) r.reg_SP);
  printMessage(&(tmpBuf[0]));
}

void Plus4EmuGUIMonitor::breakMessage(const char *s)
{
  try {
    if (s == (char *) 0 || s[0] == '\0')
      s = "BREAK";
    printMessage(s);
    printMessage("  PC  SR AC XR YR SP");
    printCPURegisters();
  }
  catch (...) {
  }
}

void Plus4EmuGUIMonitor::parseSearchPattern(
    std::vector<uint8_t>& searchString_, std::vector<uint8_t>& searchMask_,
    const std::vector<std::string>& args, size_t argOffs, size_t argCnt)
{
  for (size_t i = argOffs; i < args.size() && i < (argOffs + argCnt); i++) {
    if (args[i].length() >= 1) {
      if (args[i][0] == '"') {
        for (size_t j = 1; j < args[i].length(); j++) {
          uint8_t c = uint8_t(args[i][j] & 0x7F);
          if (c >= 0x60)
            c = c & 0x1F;
          else
            c = c & 0x3F;
          searchString_.push_back(c);
          searchMask_.push_back(uint8_t(0x3F));
        }
      }
      else if (args[i] == "*") {
        // match any byte
        searchString_.push_back(uint8_t(0x00));
        searchMask_.push_back(uint8_t(0x00));
      }
      else {
        uint32_t  n = parseHexNumberEx(args[i].c_str());
        if (n > 0xFFFFU)
          throw Plus4Emu::Exception("search value is out of range");
        uint8_t   m = 0xFF;
        // use upper 8 bits as AND mask
        if (n > 0xFFU)
          m = uint8_t(n >> 8);
        searchString_.push_back(uint8_t(n) & m);
        searchMask_.push_back(m);
      }
    }
  }
}

int32_t Plus4EmuGUIMonitor::searchPattern(
    const std::vector<uint8_t>& searchString_,
    const std::vector<uint8_t>& searchMask_,
    uint32_t startAddr, uint32_t endAddr, bool cpuAddressMode_)
{
  uint32_t  addrMask_ = (cpuAddressMode_ ? 0x0000FFFFU : 0x003FFFFFU);
  uint32_t  i = startAddr & addrMask_;
  if (searchString_.size() < 1)
    return int32_t(i);                  // empty string
  uint32_t  j = i;
  size_t    l = size_t((endAddr - startAddr) & addrMask_) + 1;
  if (l < searchString_.size())
    return int32_t(-1);
  l = l - searchString_.size();
  std::vector<uint8_t>  tmpBuf;
  tmpBuf.resize(searchString_.size());
  for (size_t k = 0; k < searchString_.size(); k++) {
    tmpBuf[k] = gui->vm.readMemory(j, cpuAddressMode_);
    j = (j + 1U) & addrMask_;
  }
  size_t  bufPos = 0;
  while (true) {
    size_t  p = bufPos;
    size_t  k = 0;
    while (true) {
      if ((tmpBuf[p] & searchMask_[k]) != searchString_[k])
        break;
      if (++k >= searchString_.size())
        return int32_t(i);              // found a match, return start address
      if (++p >= tmpBuf.size())
        p = 0;
    }
    if (!l)
      break;
    l--;
    tmpBuf[bufPos] = gui->vm.readMemory(j, cpuAddressMode_);
    i = (i + 1U) & addrMask_;
    j = (j + 1U) & addrMask_;
    if (++bufPos >= tmpBuf.size())
      bufPos = 0;
  }
  // not found
  return int32_t(-1);
}

int32_t Plus4EmuGUIMonitor::searchPattern(const std::vector<std::string>& args,
                                          size_t argOffs, size_t argCnt,
                                          uint32_t startAddr, uint32_t endAddr,
                                          bool cpuAddressMode_)
{
  std::vector<uint8_t>  searchString_;
  std::vector<uint8_t>  searchMask_;
  parseSearchPattern(searchString_, searchMask_, args, argOffs, argCnt);
  return searchPattern(searchString_, searchMask_,
                       startAddr, endAddr, cpuAddressMode_);
}

void Plus4EmuGUIMonitor::writeTraceFile(int debugContext_, uint16_t addr)
{
  if (!traceInstructionsRemaining) {
    closeTraceFile();
    return;
  }
  traceInstructionsRemaining--;
  char    tmpBuf[64];
  char    *bufp = &(tmpBuf[0]);
  if (debugContext_ == 0) {
    unsigned int  tedX, tedY;
    tedY = ((unsigned int) gui->vm.readMemory(0xFF1C, true) & 0x01U) << 8;
    tedY |= ((unsigned int) gui->vm.readMemory(0xFF1D, true) & 0xFFU);
    tedX = (unsigned int) gui->vm.readMemory(0xFF1E, true) & 0xFFU;
    int   n = std::sprintf(bufp, "[%04X:%02X] ", tedY, tedX);
    bufp = bufp + n;
  }
  else {
    int   n = std::sprintf(bufp, "[U%d] ",
                           int(((debugContext_ + 3) ^ 12) & 15));
    bufp = bufp + n;
  }
  int   n = std::sprintf(bufp, "%04X ", (unsigned int) (addr & 0xFFFF));
  bufp = bufp + n;
  try {
    std::string tmpBuf2;
    tmpBuf2.reserve(40);
    gui->vm.disassembleInstruction(tmpBuf2, addr, true);
    if (tmpBuf2.length() > 21 && tmpBuf2.length() <= 40) {
      n = std::sprintf(bufp, "%s", tmpBuf2.c_str() + 21);
      bufp = bufp + n;
    }
  }
  catch (...) {
  }
  if (std::fprintf(traceFile, "%s\n", &(tmpBuf[0]))
      != (int(bufp - &(tmpBuf[0])) + 1)) {
    closeTraceFile();           // error writing file, disk may be full
  }
  if (!traceInstructionsRemaining)
    closeTraceFile();
}

void Plus4EmuGUIMonitor::closeTraceFile()
{
  traceInstructionsRemaining = 0;
  std::FILE *f = traceFile;
  if (f) {
    traceFile = (std::FILE *) 0;
    std::fclose(f);
  }
}

uint8_t Plus4EmuGUIMonitor::readMemoryForFileIO(uint32_t addr,
                                                bool cpuAddressMode_) const
{
  if (cpuAddressMode_) {
    uint32_t  addr_ = addr & 0xFFFFU;
    if ((addr_ >= 0x8000U && addr_ <= 0xFCFFU) || addr_ >= 0xFF20U) {
      if (gui->vm.getDebugContext() == 0) {
        // always read RAM, regardless of FF3E/FF3F memory paging
        uint8_t segment_ = gui->vm.getMemoryPage(int((addr_ >> 14) & 3U));
        if (segment_ < 0x08) {
          segment_ = gui->vm.getMemoryPage(1);
          if (segment_ == 0xFF)
            segment_ = gui->vm.getMemoryPage(0);        // 16K or 32K
          else
            segment_++;
          if ((addr_ & 0x4000U) != 0U && segment_ != 0xFF)
            segment_++;
        }
        addr_ = (addr_ & 0x3FFFU) | (uint32_t(segment_) << 14);
        return gui->vm.readMemory(addr_, false);
      }
    }
  }
  return gui->vm.readMemory(addr, cpuAddressMode_);
}

