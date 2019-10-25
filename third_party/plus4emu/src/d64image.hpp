
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

#ifndef PLUS4EMU_D64IMAGE_HPP
#define PLUS4EMU_D64IMAGE_HPP

#include "plus4emu.hpp"

namespace Plus4 {

  class D64Image {
   protected:
    static const int      d64TrackOffsetTable[44];
    static const int      sectorsPerTrackTable[44];
    static const int      trackSizeTable[44];
    static const unsigned char  trackSpeedTable[44];
    static const uint8_t  gcrEncodeTable[16];
    static const uint8_t  gcrDecodeTable[32];
    uint8_t     trackBuffer_GCR[8192];
    uint8_t     trackBuffer_D64[5376];  // for 21 256-byte sectors
    uint8_t     badSectorTable[24];
    bool        trackDirtyFlag;
    int         currentTrack;           // 0 to 42
                                        // (1 to 'nTracks' are valid tracks)
    int         nTracks;                // number of tracks (35 to 42, or zero
                                        // if there is no disk image file)
    std::FILE   *imageFile;
    bool        writeProtectFlag;
    uint8_t     diskID;
    uint8_t     idCharacter1;
    uint8_t     idCharacter2;
    bool        haveBadSectorTable;
    // ----------------
    static void gcrEncodeFourBytes(uint8_t *outBuf, const uint8_t *inBuf);
    static bool gcrDecodeFourBytes(uint8_t *outBuf, const uint8_t *inBuf);
    void gcrEncodeTrack(int trackNum, int nSectors, int nBytes);
    int gcrDecodeTrack(int trackNum, int nSectors, int nBytes);
    bool readTrack(int trackNum = -1);
    bool flushTrack(int trackNum = -1);
    virtual bool setCurrentTrack(int trackNum);
    D64Image();
    virtual ~D64Image();
    void setImageFile(std::FILE *imageFile_, bool isReadOnly);
  };

}       // namespace Plus4

#endif  // PLUS4EMU_D64IMAGE_HPP

