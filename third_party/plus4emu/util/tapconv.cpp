
// plus4emu -- portable Commodore Plus/4 emulator
// Copyright (C) 2003-2007 Istvan Varga <istvanv@users.sourceforge.net>
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

#include "plus4emu.hpp"
#include "tape.hpp"

int main(int argc, char **argv)
{
  std::FILE       *inFile = (std::FILE *) 0;
  Plus4Emu::Tape  *outFile = (Plus4Emu::Tape *) 0;
  try {
    if (argc != 3)
      throw Plus4Emu::Exception("Usage: tapconv <infile> <outfile>");
    inFile = std::fopen(argv[1], "rb");
    if (!inFile)
      throw Plus4Emu::Exception("Error opening input file");
    bool    usingOldTapFormat = false;
    {
      char    tmpBuf[20];
      if (std::fread(&(tmpBuf[0]), 1, 20, inFile) != 20)
        throw Plus4Emu::Exception("Error reading input file");
      if (tmpBuf[12] == 1)
        usingOldTapFormat = true;
      else if (tmpBuf[12] != 2)
        throw Plus4Emu::Exception("Unknown input file format");
      tmpBuf[12] = '\0';
      if (std::strcmp(&(tmpBuf[0]), "C16-TAPE-RAW") != 0)
        throw Plus4Emu::Exception("Unknown input file format");
    }
    int     outputSignal = 0;
    int     cnt = 0;    // downsample by a factor of 32
    int     inputSignal = (usingOldTapFormat ? -1 : 1);
    int     inputCnt = 0;
    int     savedInputCnt = 0;
    outFile = Plus4Emu::openTapeFile(argv[2], 3, 27710, 1);
    outFile->record();
    outFile->setIsMotorOn(true);
    while (true) {
      if (!inputCnt) {
        inputSignal = -inputSignal;
        if (!(usingOldTapFormat && inputSignal > 0)) {
          int   c = std::fgetc(inFile);
          if (c == EOF)
            break;
          if (!c) {
            savedInputCnt = 0;
            for (int i = 0; i < 3; i++) {
              c = std::fgetc(inFile);
              if (c == EOF)
                throw Plus4Emu::Exception("Unexpected end of input file");
              savedInputCnt = (savedInputCnt >> 8) & 0xFFFF;
              savedInputCnt = savedInputCnt | ((c & 0xFF) << 16);
            }
          }
          else
            savedInputCnt = c << 3;
          if (usingOldTapFormat)
            savedInputCnt = savedInputCnt >> 1;
        }
        inputCnt = savedInputCnt;
      }
      outputSignal += inputSignal;
      if (++cnt >= 32) {
        cnt = 0;
        outFile->setInputSignal(outputSignal > 0 ? 1 : 0);
        outFile->runOneSample();
        outputSignal = 0;
      }
      if (inputCnt)
        inputCnt--;
    }
  }
  catch (std::exception& e) {
    if (inFile)
      std::fclose(inFile);
    if (outFile)
      delete outFile;
    std::fprintf(stderr, "%s\n", e.what());
    return -1;
  }
  std::fclose(inFile);
  delete outFile;
  return 0;
}

