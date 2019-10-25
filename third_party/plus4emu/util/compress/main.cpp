
// compressor utility for Commodore Plus/4 programs
// Copyright (C) 2007-2016 Istvan Varga <istvanv@users.sourceforge.net>
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
#include "compress.hpp"

#include <vector>

// compression type (0, 1, 2, 3 or 5, or -1 to use default or auto-detect)
static int    compressionType = -1;
// extract compressed file
static bool   extractMode = false;
// test compressed file(s)
static bool   testMode = false;
// compression level (1: fast, low compression ... 10: slow, high compression)
static int    compressionLevel = 5;
// use all RAM (up to $4000) on the C16
static bool   c16Mode = false;
// do not verify checksum in self-extracting module
static bool   noCRCCheck = false;
// do not read ahead one byte when decompressing (faster, but unsafe if the
// end address is $FD00)
static bool   noReadBuffer = false;
// border effect type (1 to 3) to be used by the decompressor, or 0 to disable
static int    borderEffectType = 2;
// do not clean up after decompression if this is set to true
static bool   noCleanup = false;
// do not enable interrupts after decompression if this is set to true
static bool   noCLI = false;
// do not enable ROM after decompression if this is set to true
static bool   noROM = false;
// do not blank display while decompressing (smaller but slower SFX module)
static bool   noBlankDisplay = false;
// do not update zeropage variables after decompression if this is set to true
static bool   noZPUpdate = false;
// do not include decompressor code in the PRG output if non-negative
static int    rawLoadAddr = -1;
// do not read or write PRG/P00 header
static bool   noPRGMode = false;
// write output file as p4fliconv raw compressed image
static bool   fliImageFormat = false;
// start address (0 - 0xFFFF), or -1 for run, -2 for basic, or -3 for monitor
static long   runAddr = -2L;

static long convertStringToInteger(const char *s, bool isRunAddr = false)
{
  if (s == (char *) 0 || s[0] == '\0')
    throw Plus4Emu::Exception("invalid integer argument format");
  if (isRunAddr) {
    if (std::strcmp(s, "RUN") == 0 || std::strcmp(s, "Run") == 0 ||
        std::strcmp(s, "run") == 0) {
      return -1L;
    }
    if (std::strcmp(s, "BASIC") == 0 || std::strcmp(s, "Basic") == 0 ||
        std::strcmp(s, "basic") == 0) {
      return -2L;
    }
    if (std::strcmp(s, "MONITOR") == 0 || std::strcmp(s, "Monitor") == 0 ||
        std::strcmp(s, "monitor") == 0) {
      return -3L;
    }
  }
  bool    negativeFlag = false;
  bool    hexMode = false;
  if (s[0] == '+') {
    s++;
  }
  else if (s[0] == '-') {
    negativeFlag = true;
    s++;
  }
  if (s[0] == '$' || s[0] == 'X' || s[0] == 'x') {
    hexMode = true;
    s++;
  }
  else if (s[0] == '0') {
    if (s[1] == 'X' || s[1] == 'x') {
      hexMode = true;
      s = s + 2;
    }
  }
  if (s[0] == '\0')
    throw Plus4Emu::Exception("invalid integer argument format");
  long    n = 0L;
  for (size_t i = 0; s[i] != '\0'; i++) {
    n = n * (hexMode ? 16L : 10L);
    if (s[i] >= '0' && s[i] <= '9')
      n = n + long(s[i] - '0');
    else if (hexMode && (s[i] >= 'A' && s[i] <= 'F'))
      n = n + long(s[i] - 'A') + 10L;
    else if (hexMode && (s[i] >= 'a' && s[i] <= 'f'))
      n = n + long(s[i] - 'a') + 10L;
    else
      throw Plus4Emu::Exception("invalid integer argument format");
    if (n > 0xFFFFL)
      throw Plus4Emu::Exception("integer argument is out of range");
  }
  if (negativeFlag)
    n = (-n);
  return n;
}

static void readInputFile(std::vector< unsigned char >& buf,
                          unsigned int& startAddress,
                          const char *fileName,
                          size_t skipBytes = 0, size_t lengthLimit = 65536)
{
  startAddress = 0U;
  buf.clear();
  if (fileName == (char *) 0 || fileName[0] == '\0')
    throw Plus4Emu::Exception("invalid input file name");
  std::FILE *f = (std::FILE *) 0;
  int       fileType = 0;       // 0: raw, 1: .prg, 2: .p00, 3: .r00/.s00/.u00
  try {
    f = std::fopen(fileName, "rb");
    if (!f)
      throw Plus4Emu::Exception("error opening input file");
    if (!noPRGMode) {
      uint8_t tmpBuf[28];
      size_t  bytesRead = std::fread(&(tmpBuf[0]), sizeof(uint8_t), 28, f);
      size_t  nameLen = std::strlen(fileName);
      fileType = 1;
      if (nameLen >= 4 && bytesRead >= 26) {
        if (fileName[nameLen - 4] == '.' &&
            ((fileName[nameLen - 3] | char(0x20)) == 'p' ||
             (fileName[nameLen - 3] | char(0x20)) == 'r' ||
             (fileName[nameLen - 3] | char(0x20)) == 's' ||
             (fileName[nameLen - 3] | char(0x20)) == 'u') &&
            fileName[nameLen - 2] >= '0' && fileName[nameLen - 2] <= '9' &&
            fileName[nameLen - 1] >= '0' && fileName[nameLen - 1] <= '9') {
          if (tmpBuf[0] == 0x43 && tmpBuf[1] == 0x36 && tmpBuf[2] == 0x34 &&
              tmpBuf[3] == 0x46 && tmpBuf[4] == 0x69 && tmpBuf[5] == 0x6C &&
              tmpBuf[6] == 0x65 && tmpBuf[7] == 0x00) {
            fileType = ((fileName[nameLen - 3] | char(0x20)) == 'p' ? 2 : 3);
          }
        }
      }
      if ((fileType == 1 && bytesRead < 2) || (fileType == 2 && bytesRead < 28))
        throw Plus4Emu::Exception("unexpected end of input file");
      if (fileType == 1) {                      // .prg
        startAddress =
            (unsigned int) tmpBuf[0] | ((unsigned int) tmpBuf[1] << 8);
        for (size_t i = 2; i < bytesRead; i++)
          buf.push_back((unsigned char) tmpBuf[i]);
      }
      else if (fileType == 2) {                 // .p00
        startAddress =
            (unsigned int) tmpBuf[26] | ((unsigned int) tmpBuf[27] << 8);
      }
      else {                                    // .r00/.s00/.u00
        for (size_t i = 26; i < bytesRead; i++)
          buf.push_back((unsigned char) tmpBuf[i]);
      }
    }
    while (true) {
      int     c = std::fgetc(f);
      if (c == EOF)
        break;
      buf.push_back((unsigned char) (c & 0xFF));
    }
    std::fclose(f);
    f = (std::FILE *) 0;
  }
  catch (...) {
    if (f)
      std::fclose(f);
    throw;
  }
  if (skipBytes > buf.size())
    skipBytes = buf.size();
  if (skipBytes > 0)
    buf.erase(buf.begin(), buf.begin() + skipBytes);
  if (buf.size() > lengthLimit)
    buf.resize(lengthLimit);
}

int main(int argc, char **argv)
{
  const char  *programName = argv[0];
  if (programName == (char *) 0)
    programName = "";
  for (size_t i = std::strlen(programName); i > 0; ) {
    i--;
    if (programName[i] == '/' || programName[i] == '\\' ||
        programName[i] == ':') {
      programName = programName + (i + 1);
      break;
    }
  }
  if (programName[0] == '\0')
    programName = "compress";
  std::vector< std::string >  fileNames;
  std::vector< int >  fileOffsets;
  std::vector< int >  fileLengths;
  std::vector< int >  loadAddresses;
  fileOffsets.push_back(0);
  fileLengths.push_back(65536);
  loadAddresses.push_back(-1);
  bool    printUsageFlag = false;
  bool    helpFlag = false;
  bool    endOfOptions = false;
  std::FILE *f = (std::FILE *) 0;
  Plus4Compress::Compressor *compress = (Plus4Compress::Compressor *) 0;
  try {
    for (int i = 1; i < argc; i++) {
      std::string tmp = argv[i];
      if (tmp.length() < 1)
        continue;
      if (endOfOptions || tmp[0] != '-') {
        fileNames.push_back(tmp);
        fileOffsets.push_back(0);
        fileLengths.push_back(65536);
        loadAddresses.push_back(-1);
        continue;
      }
      if (tmp == "--") {
        endOfOptions = true;
        continue;
      }
      if (tmp.length() >= 4) {
        // allow GNU-style long options
        if (tmp[1] == '-')
          tmp.erase(0, 1);
      }
      if (tmp == "-h" || tmp == "-help") {
        printUsageFlag = true;
        helpFlag = true;
        throw Plus4Emu::Exception("");
      }
      else if (tmp == "-x") {
        extractMode = true;
        testMode = false;
      }
      else if (tmp == "-t") {
        testMode = true;
        extractMode = false;
      }
      else if (tmp.length() == 3 &&
               (tmp[1] == 'm' && (tmp[2] >= '0' && tmp[2] <= '5'))) {
        compressionType = int(tmp[2] - '0');
      }
      else if (tmp == "-mz" || tmp == "-zlib") {
        compressionType = 5;
      }
      else if (tmp == "-m") {
        if (++i >= argc) {
          printUsageFlag = true;
          throw Plus4Emu::Exception("missing argument for '-m'");
        }
        int     n = int(convertStringToInteger(argv[i]));
        n = ((n >= 0 && n <= 5) ? n : -1);
        compressionType = n;
      }
      else if (tmp.length() == 2 && (tmp[1] >= '1' && tmp[1] <= '9')) {
        compressionLevel = int(tmp[1] - '0');
      }
      else if (tmp == "-X") {
        compressionLevel = 10;
      }
      else if (tmp == "-c16") {
        c16Mode = true;
      }
      else if (tmp == "-nocrc") {
        noCRCCheck = true;
      }
      else if (tmp == "-nobuf") {
        noReadBuffer = true;
      }
      else if (tmp == "-borderfx") {
        if (++i >= argc) {
          printUsageFlag = true;
          throw Plus4Emu::Exception("missing argument for '-borderfx'");
        }
        int     n = int(convertStringToInteger(argv[i]));
        n = ((n >= 0 && n <= 3) ? n : 0);
        borderEffectType = n;
      }
      else if (tmp == "-nocleanup") {
        noCleanup = true;
      }
      else if (tmp == "-nocli") {
        noCLI = true;
      }
      else if (tmp == "-norom") {
        noROM = true;
      }
      else if (tmp == "-noblank") {
        noBlankDisplay = true;
      }
      else if (tmp == "-fastsfx") {
        noCRCCheck = true;
        noReadBuffer = true;
        borderEffectType = 1;
      }
      else if (tmp == "-shortsfx") {
        noCRCCheck = true;
        noReadBuffer = true;
        borderEffectType = 0;
        noCleanup = true;
        noBlankDisplay = true;
        noZPUpdate = true;
      }
      else if (tmp == "-nozp") {
        noZPUpdate = true;
      }
      else if (tmp == "-zp") {
        noZPUpdate = false;
      }
      else if (tmp == "-raw") {
        if (++i >= argc) {
          printUsageFlag = true;
          throw Plus4Emu::Exception("missing argument for '-raw'");
        }
        int     n = int(convertStringToInteger(argv[i]));
        n = (n >= 0 ? (n & 0xFFFF) : -1);
        rawLoadAddr = n;
        noZPUpdate = true;
      }
      else if (tmp == "-noprg") {
        noPRGMode = true;
        noZPUpdate = true;
      }
      else if (tmp == "-fli") {
        fliImageFormat = true;
        rawLoadAddr = 0x17FE;
        noZPUpdate = true;
      }
      else if (tmp == "-skip") {
        if (++i >= argc) {
          printUsageFlag = true;
          throw Plus4Emu::Exception("missing argument for '-skip'");
        }
        int     n = int(convertStringToInteger(argv[i]));
        n = (n >= 0 ? (n & 0xFFFF) : 0);
        fileOffsets[fileNames.size()] = n;
      }
      else if (tmp == "-length") {
        if (++i >= argc) {
          printUsageFlag = true;
          throw Plus4Emu::Exception("missing argument for '-length'");
        }
        int     n = int(convertStringToInteger(argv[i]));
        n = (n >= 0 ? (n & 0xFFFF) : 65536);
        fileLengths[fileNames.size()] = n;
      }
      else if (tmp == "-loadaddr") {
        if (++i >= argc) {
          printUsageFlag = true;
          throw Plus4Emu::Exception("missing argument for '-loadaddr'");
        }
        int     n = int(convertStringToInteger(argv[i]));
        n = (n >= 0 ? (n & 0xFFFF) : -1);
        loadAddresses[fileNames.size()] = n;
      }
      else if (tmp == "-start") {
        if (++i >= argc) {
          printUsageFlag = true;
          throw Plus4Emu::Exception("missing argument for '-start'");
        }
        runAddr = convertStringToInteger(argv[i], true);
      }
      else {
        printUsageFlag = true;
        throw Plus4Emu::Exception("invalid command line option");
      }
    }
    if (fileNames.size() < (testMode ? 1 : 2)) {
      printUsageFlag = true;
      throw Plus4Emu::Exception("missing file name");
    }
    if (compressionType >= 3 && !(noPRGMode && noZPUpdate)) {
      if (compressionType == 3)
        throw Plus4Emu::Exception("-m3 compression requires -noprg");
      else
        throw Plus4Emu::Exception("ZLib compression requires -noprg");
    }
    // if offset, length, or load address is specified after the last file name,
    // apply it to the last file
    if (fileOffsets[fileNames.size()] > 0)
      fileOffsets[fileNames.size() - 1] = fileOffsets[fileNames.size()];
    if (fileLengths[fileNames.size()] < 65536)
      fileLengths[fileNames.size() - 1] = fileLengths[fileNames.size()];
    if (loadAddresses[fileNames.size()] >= 0)
      loadAddresses[fileNames.size() - 1] = loadAddresses[fileNames.size()];
    if (!(extractMode || testMode)) {
      // in compress mode, these parameters can only be applied to the input
      // files
      if (fileOffsets[fileNames.size() - 1] > 0)
        fileOffsets[fileNames.size() - 2] = fileOffsets[fileNames.size()];
      if (fileLengths[fileNames.size() - 1] < 65536)
        fileLengths[fileNames.size() - 2] = fileLengths[fileNames.size()];
      if (loadAddresses[fileNames.size() - 1] >= 0)
        loadAddresses[fileNames.size() - 2] = loadAddresses[fileNames.size()];
    }
    if (testMode) {
      // test compressed file(s)
      noPRGMode = true;
      std::vector< unsigned char >  inBuf;
      bool    errorFlag = false;
      for (size_t i = 0; i < fileNames.size(); i++) {
        std::printf("%s: ", fileNames[i].c_str());
        inBuf.clear();
        unsigned int  startAddr = 0U;
        readInputFile(inBuf, startAddr, fileNames[i].c_str(),
                      size_t(fileOffsets[i]), size_t(fileLengths[i]));
        if (inBuf.size() < 1) {
          errorFlag = true;
          std::printf("FAILED (empty file)\n");
        }
        else {
          std::vector< std::vector< unsigned char > > tmpBuf;
          try {
            Plus4Compress::decompressData(tmpBuf, inBuf, compressionType);
            std::printf("OK\n");
          }
          catch (Plus4Emu::Exception) {
            errorFlag = true;
            std::printf("FAILED\n");
          }
        }
      }
      return (errorFlag ? -1 : 0);
    }
    if (extractMode) {
      // extract compressed file
      bool    savedNoPRGMode = noPRGMode;
      noPRGMode = true;
      std::vector< unsigned char >  inBuf;
      unsigned int  startAddr = 0U;
      readInputFile(inBuf, startAddr, fileNames[0].c_str(),
                    size_t(fileOffsets[0]), size_t(fileLengths[0]));
      if (inBuf.size() < 1)
        throw Plus4Emu::Exception("empty input file");
      noPRGMode = savedNoPRGMode;
      std::vector< std::vector< unsigned char > > outBufs;
      compressionType =
          Plus4Compress::decompressData(outBufs, inBuf, compressionType);
      size_t  n = 1;
      for (size_t i = 0; i < outBufs.size(); i++) {
        if (outBufs[i].size() < 2)
          continue;
        startAddr =
            (unsigned int) outBufs[i][0] | ((unsigned int) outBufs[i][1] << 8);
        if ((compressionType == 0 &&
             (startAddr == 0x096DU || startAddr == 0x09DDU ||
              startAddr == 0x03E7U)) ||
            (compressionType > 0 &&
             (startAddr == 0x002DU || startAddr == 0x009DU))) {
          // skip any decompressor code or $2D/$9D update blocks
          continue;
        }
        if (loadAddresses[n] >= 0)
          startAddr = (unsigned int) loadAddresses[n];
        if (n >= fileNames.size())
          throw Plus4Emu::Exception("too few output file names");
        try {
          f = std::fopen(fileNames[n].c_str(), "wb");
          if (!f)
            throw Plus4Emu::Exception("cannot open output file");
          if (!noPRGMode) {
            if (std::fputc(int(startAddr & 0xFFU), f) == EOF)
              throw Plus4Emu::Exception("error writing output file");
            if (std::fputc(int((startAddr >> 8) & 0xFFU), f) == EOF)
              throw Plus4Emu::Exception("error writing output file");
          }
          for (size_t j = 2; j < outBufs[i].size(); j++) {
            if (std::fputc(int(outBufs[i][j]), f) == EOF)
              throw Plus4Emu::Exception("error writing output file");
          }
          if (std::fflush(f) != 0)
            throw Plus4Emu::Exception("error writing output file");
          std::fclose(f);
          f = (std::FILE *) 0;
        }
        catch (...) {
          if (f) {
            std::fclose(f);
            f = (std::FILE *) 0;
            std::remove(fileNames[n].c_str());
          }
          throw;
        }
        n++;
      }
      if (n < fileNames.size()) {
        if (n == 1)
          throw Plus4Emu::Exception("empty input file");
        throw Plus4Emu::Exception("too many output file names");
      }
      return 0;
    }
    // compress file(s)
    if (compressionType < 0)
      compressionType = 0;              // default to method 0 (LZ77 + Huffman)
    if (noPRGMode) {
      if (fliImageFormat) {
        throw Plus4Emu::Exception("cannot use -noprg and -fli "
                                  "at the same time");
      }
      rawLoadAddr = 0;                  // not used, but must be >= 0
    }
    std::vector< unsigned char >  outBuf;
    std::vector< unsigned char >  inBuf;
    std::vector< bool >           bytesUsed;
    std::vector< unsigned char >  tmpBuf;
    compress = Plus4Compress::createCompressor(compressionType, outBuf);
    inBuf.resize(65536);
    bytesUsed.resize(65536);
    for (size_t i = 0; i < 65536; i++) {
      inBuf[i] = 0x00;
      bytesUsed[i] = false;
    }
    // read all input files
    for (int i = 0; i < int(fileNames.size() - 1); i++) {
      tmpBuf.resize(0);
      unsigned int  startAddr = 0U;
      readInputFile(tmpBuf, startAddr, fileNames[i].c_str(),
                    size_t(fileOffsets[i]), size_t(fileLengths[i]));
      if (loadAddresses[i] >= 0)
        startAddr = (unsigned int) loadAddresses[i];
      unsigned int  endAddr = startAddr + (unsigned int) tmpBuf.size();
      if (fliImageFormat &&
          (endAddr < 0x6000U || endAddr > 0xE500U ||
           !(startAddr == 0x1800U ||
             (startAddr == 0x17FEU &&
              tmpBuf[0] == 0x00 && tmpBuf[1] == 0x00)))) {
        throw Plus4Emu::Exception("input file is not a p4fliconv image");
      }
      for (size_t j = 0; j < tmpBuf.size(); j++) {
        unsigned int  addr = (startAddr + (unsigned int) j) & 0xFFFFU;
        inBuf[addr] = tmpBuf[j];
        bytesUsed[addr] = true;
      }
    }
    unsigned int  firstAddr = 0xFD00U;
    while (firstAddr != 0U && bytesUsed[firstAddr])
      firstAddr = (firstAddr + 1U) & 0xFFFFU;
    // find last address used
    unsigned int  endAddr = (firstAddr - 1U) & 0xFFFFU;
    while (!bytesUsed[endAddr]) {
      if (endAddr == firstAddr)
        throw Plus4Emu::Exception("no input data to compress");
      endAddr = (endAddr - 1U) & 0xFFFFU;
    }
    endAddr = (endAddr + 1U) & 0xFFFFU;
    if (!noZPUpdate) {
      compress->setCompressionLevel(1);
      compress->addZeroPageUpdate(endAddr, false);
    }
    // find and compress all continuous data blocks
    unsigned int  blockNum = 0U;
    unsigned int  startAddr = firstAddr;
    unsigned int  curPos = firstAddr;
    do {
      if (bytesUsed[curPos]) {
        curPos = (curPos + 1U) & 0xFFFFU;
        if (curPos != 0U && curPos != firstAddr)
          continue;
      }
      else if (curPos == startAddr) {
        curPos = (curPos + 1U) & 0xFFFFU;
        startAddr = curPos;
        continue;
      }
      blockNum++;
      std::fprintf(stderr,
                   (compressionType != 3 ?
                    "Block #%u ($%04X-$%04X): " : "Block #%u ($%04X-$%04X)\n"),
                   blockNum, startAddr, curPos);
      tmpBuf.resize(0);
      unsigned int  i = startAddr;
      do {
        tmpBuf.push_back(inBuf[i]);
        i = (i + 1U) & 0xFFFFU;
      } while (i != curPos);
      compress->setCompressionLevel(compressionLevel);
      compress->compressData(tmpBuf, startAddr, (curPos == endAddr), true);
      if (curPos != 0U && curPos != firstAddr)
        curPos = (curPos + 1U) & 0xFFFFU;
      startAddr = curPos;
    } while (curPos != firstAddr);
    // write output file
    f = std::fopen(fileNames[fileNames.size() - 1].c_str(), "wb");
    if (!f)
      throw Plus4Emu::Exception("cannot open output file");
    if (rawLoadAddr < 0) {
      std::vector< unsigned char >  sfxBuf;
      Plus4Compress::getSFXModule(
          sfxBuf, compressionType, int(runAddr), c16Mode,
          noCRCCheck, noReadBuffer, borderEffectType, noBlankDisplay,
          noCleanup, noROM, noCLI);
      for (size_t i = 0; i < sfxBuf.size(); i++) {
        if (std::fputc(int(sfxBuf[i]), f) == EOF)
          throw Plus4Emu::Exception("error writing output file");
      }
    }
    else if (!noPRGMode) {
      if (std::fputc(rawLoadAddr & 0xFF, f) == EOF)
        throw Plus4Emu::Exception("error writing output file");
      if (std::fputc((rawLoadAddr >> 8) & 0xFF, f) == EOF)
        throw Plus4Emu::Exception("error writing output file");
    }
    if (fliImageFormat) {
      size_t  nBytes = outBuf.size();
      if (std::fputc(int((nBytes >> 8) & 0xFF), f) == EOF)
        throw Plus4Emu::Exception("error writing output file");
      if (std::fputc(int(nBytes & 0xFF), f) == EOF)
        throw Plus4Emu::Exception("error writing output file");
    }
    for (size_t i = 0; i < outBuf.size(); i++) {
      if (std::fputc(int(outBuf[i]), f) == EOF)
        throw Plus4Emu::Exception("error writing output file");
    }
    if (std::fflush(f) != 0)
      throw Plus4Emu::Exception("error writing output file");
    std::fclose(f);
    f = (std::FILE *) 0;
  }
  catch (std::exception& e) {
    if (f) {
      std::fclose(f);
      std::remove(fileNames[fileNames.size() - 1].c_str());
    }
    if (compress) {
      delete compress;
      compress = (Plus4Compress::Compressor *) 0;
    }
    if (printUsageFlag || helpFlag) {
      std::fprintf(stderr, "Usage:\n");
      std::fprintf(stderr, "    %s [OPTIONS...] <infile...> [OPTIONS...] "
                           "<outfile>\n", programName);
      std::fprintf(stderr, "        compress file(s)\n");
      std::fprintf(stderr, "    %s -x [OPTIONS...] <infile> [OPTIONS...] "
                           "<outfile...>\n", programName);
      std::fprintf(stderr, "        extract compressed file (experimental)\n");
      std::fprintf(stderr, "    %s -t [OPTIONS...] <infile...>\n", programName);
      std::fprintf(stderr, "        test compressed file(s)\n");
    }
    if (printUsageFlag && !helpFlag) {
      std::fprintf(stderr, "    %s --help\n", programName);
      std::fprintf(stderr, "        print detailed usage information\n");
    }
    if (helpFlag) {
      std::fprintf(stderr, "General options:\n");
      std::fprintf(stderr, "    --\n");
      std::fprintf(stderr, "        interpret all remaining arguments as file "
                           "names\n");
      std::fprintf(stderr, "    -m0 ... -m5 | -mz | -zlib\n");
      std::fprintf(stderr, "        select compression method (default: 0)\n");
      std::fprintf(stderr, "    -1 ... -9\n");
      std::fprintf(stderr, "        set compression level vs. speed (default: "
                           "5)\n");
      std::fprintf(stderr, "    -X\n");
      std::fprintf(stderr, "        set maximum compression level (very slow "
                           "and may or may not make\n");
      std::fprintf(stderr, "        the output file smaller)\n");
      std::fprintf(stderr, "    -noprg\n");
      std::fprintf(stderr, "        read and write raw files without PRG or "
                           "P00 header (implies\n");
      std::fprintf(stderr, "        -raw and -nozp)\n");
      std::fprintf(stderr, "    -loadaddr <ADDR>\n");
      std::fprintf(stderr, "        override the load address of the next "
                           "input (when compressing)\n"
                           "        or output (when extracting) file\n");
      std::fprintf(stderr, "Input file options:\n");
      std::fprintf(stderr, "    -skip <N>\n");
      std::fprintf(stderr, "        skip N bytes at the beginning of the next "
                           "input file\n");
      std::fprintf(stderr, "    -length <N>\n");
      std::fprintf(stderr, "        read at most N bytes (default: 65536) from "
                           "the next input file\n");
      std::fprintf(stderr, "Output format options:\n");
      std::fprintf(stderr, "    -raw <LOADADDR>\n");
      std::fprintf(stderr, "        write the compressed data only as a PRG "
                           "file (implies -nozp)\n");
      std::fprintf(stderr, "    -fli\n");
      std::fprintf(stderr, "        compress p4fliconv raw FLI image "
                           "(implies -raw and -nozp)\n");
      std::fprintf(stderr, "    -nozp\n");
      std::fprintf(stderr, "        do not update zeropage variables at "
                           "$2D-$32 and $9D-$9E\n");
      std::fprintf(stderr, "    -zp\n");
      std::fprintf(stderr, "        update zeropage variables at $2D-$32 and "
                           "$9D-$9E\n");
      std::fprintf(stderr, "Self-extracting decompressor options:\n");
      std::fprintf(stderr, "    -c16\n");
      std::fprintf(stderr, "        generate decompression code for the C16\n");
      std::fprintf(stderr, "    -nocrc\n");
      std::fprintf(stderr, "        do not verify checksum before "
                           "decompression\n");
      std::fprintf(stderr, "    -nobuf\n");
      std::fprintf(stderr, "        disable read buffer (unsafe if end address "
                           "is $FD00; can only be\n"
                           "        used with -nocrc if -m0 is selected)\n");
      std::fprintf(stderr, "    -borderfx <N>       (-m1 and -m2 only)\n");
      std::fprintf(stderr, "        border effect type (1 to 3), or zero to "
                           "disable effects\n");
      std::fprintf(stderr, "    -nocleanup\n");
      std::fprintf(stderr, "        do not clean up after decompression\n");
      std::fprintf(stderr, "    -noblank            (-m1 and -m2 only)\n");
      std::fprintf(stderr, "        do not blank display while decompressing "
                           "(smaller but slower)\n");
      std::fprintf(stderr, "    -nocli\n");
      std::fprintf(stderr, "        do not enable interrupts after "
                           "decompression\n");
      std::fprintf(stderr, "    -norom\n");
      std::fprintf(stderr, "        do not enable ROM after decompression\n");
      std::fprintf(stderr, "    -fastsfx\n");
      std::fprintf(stderr, "        same as '-nocrc -nobuf -borderfx 1'\n");
      std::fprintf(stderr, "    -shortsfx\n");
      std::fprintf(stderr, "        same as '-nocrc -nobuf -borderfx 0 "
                           "-nocleanup -noblank -nozp'\n");
      std::fprintf(stderr, "    -start <ADDR>\n");
      std::fprintf(stderr, "        start program at address ADDR, or RUN if "
                           "ADDR is -1 or 'run',\n"
                           "        return to basic if -2 or 'basic' "
                           "(default), or monitor if ADDR\n"
                           "        is -3 or 'monitor'\n");
      std::fprintf(stderr, "Addresses can be specified in decimal format, or "
                           "hexadecimal with '$',\n"
                           "'x', or '0x' prefix.\n");
      return 0;
    }
    std::fprintf(stderr, " *** %s: %s\n", programName, e.what());
    return -1;
  }
  if (compress)
    delete compress;
  return 0;
}

