
// plus4emu -- portable Commodore Plus/4 emulator
// Copyright (C) 2003-2018 Istvan Varga <istvanv@users.sourceforge.net>
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

#ifndef PLUS4EMU_CHARCONV_HPP
#define PLUS4EMU_CHARCONV_HPP

#include "plus4emu.hpp"

namespace Plus4Emu {

  /*!
   * Convert a single UTF-8 encoded character from 'buf' to PETSCII.
   * Returns 0 if the end of the input buffer has been reached,
   * 0xFF on invalid character.
   */
  uint8_t utf8ToPETSCII(const char *buf, size_t& bufPos, bool lowerCaseMode);

  /*!
   * Convert PETSCII character 'c' to UTF-8, and write it to 'buf'.
   */
  void petsciiToUTF8(std::string& buf, uint8_t c, bool lowerCaseMode);

  /*!
   * Convert a character from screen memory to UTF-8, and write it to 'buf'.
   */
  void screenCharToUTF8(std::string& buf, uint8_t c, bool lowerCaseMode);

}       // namespace Plus4Emu

#endif  // PLUS4EMU_CHARCONV_HPP

